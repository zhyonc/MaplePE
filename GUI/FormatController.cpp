#include "pch.h"
#include "FormatView.h"
#include "FormatController.h"

namespace {

	const std::wstring kCommentSymbol = L" // ";

	const std::map<PacketActionType, std::wstring> kActionText = {
		{ PacketActionType::Decode1, L"Decode1" },
		{ PacketActionType::Decode2, L"Decode2" },
		{ PacketActionType::Decode4, L"Decode4" },
		{ PacketActionType::Decode8, L"Decode8" },
		{ PacketActionType::Skip8, L"Skip8" },
		{ PacketActionType::DecodeStr, L"DecodeStr" },
		{ PacketActionType::DecodeBuffer, L"DecodeBuffer" },
		{ PacketActionType::Encode1, L"Encode1" },
		{ PacketActionType::Encode2, L"Encode2" },
		{ PacketActionType::Encode4, L"Encode4" },
		{ PacketActionType::Encode8, L"Encode8" },
		{ PacketActionType::EncodeStr, L"EncodeStr" },
		{ PacketActionType::EncodeBuffer, L"EncodeBuffer" },
	};

}

FormatController::FormatController(int nSelectedID, IMainController* mainControllerImpl, FormatView* view)
{
	m_logID = nSelectedID;
	m_mainControllerImpl = mainControllerImpl;
	m_formatView = view;
	initPacketFormatModels();
}

FormatController::~FormatController()
{
}


int FormatController::GetLogID()
{
	return m_logID;
}

void FormatController::initPacketFormatModels()
{
	PacketLogModel log = m_mainControllerImpl->GetPacketLogModel(m_logID);
	if (log.IsEmpty()) {
		return;
	}
	// Recover data to buffer
	std::vector<uint8_t> buffer;
	const std::wstring data = log.GetData();
	PacketScript::Data2Buffer(data, buffer);
	// Get actions
	const std::vector<PacketAction> actions = log.GetActions();
	// Set item
	size_t valuePos = 0, segmentPos = 0, outLen = 0;
	for (size_t i = 0; i < actions.size(); i++)
	{
		PacketAction action = actions[i];
		std::wstring value = L"";
		std::wstring segment = L"";
		switch (action.Type) {
		case PacketActionType::Decode1:
		case PacketActionType::Encode1:
			value = std::to_wstring((int8_t)PacketScript::Decode1(buffer, valuePos));
			segment = PacketScript::GetHexSegment(data, segmentPos, 1);
			break;
		case PacketActionType::Decode2:
		case PacketActionType::Encode2:
			value = std::to_wstring((int16_t)PacketScript::Decode2(buffer, valuePos));
			segment = PacketScript::GetHexSegment(data, segmentPos, 2);
			break;
		case PacketActionType::Decode4:
		case PacketActionType::Encode4:
			value = std::to_wstring((int32_t)PacketScript::Decode4(buffer, valuePos));
			segment = PacketScript::GetHexSegment(data, segmentPos, 4);
			break;
		case PacketActionType::Decode8:
		case PacketActionType::Skip8:
		case PacketActionType::Encode8:
			value = std::to_wstring(PacketScript::Decode8(buffer, valuePos));
			segment = PacketScript::GetHexSegment(data, segmentPos, 8);
			break;
		case PacketActionType::DecodeStr:
		case PacketActionType::EncodeStr:
			value = PacketScript::DecodeStr(buffer, valuePos, outLen);
			segment = PacketScript::GetHexSegment(data, segmentPos, 2 + outLen);
			action.Size = static_cast<uint32_t>(outLen);
			break;
		case PacketActionType::DecodeBuffer:
		case PacketActionType::EncodeBuffer:
			value = PacketScript::DecodeBuffer(buffer, valuePos, action.Size);
			segment = PacketScript::GetHexSegment(data, segmentPos, action.Size);
			break;
		}
		std::wstring retAddr = PacketScript::Int2HexW(action.RetAddr);
		uint8_t actionType = static_cast<uint8_t>(action.Type);
		std::wstring actionText = kActionText.at(action.Type);
		PacketFormatModel format(i, retAddr, actionType, actionText, action.Size, value, segment);
		m_packetFormatModels.push_back(format);
	}
}

const std::vector<PacketFormatModel>& FormatController::GetPacketFormatModels()
{
	return m_packetFormatModels;
}

PacketFormatModel* FormatController::getPacketFormatModel(int index)
{
	if (index <0 || static_cast<size_t>(index) > m_packetFormatModels.size()) {
		return nullptr;
	}
	return &m_packetFormatModels[index];
}

bool FormatController::EncodeValue(int row, const std::wstring& text, std::wstring& segment, std::wstring& strLen)
{
	PacketFormatModel* pFormat = this->getPacketFormatModel(row - 1);
	if (pFormat == nullptr) {
		return false;
	}
	std::vector<uint8_t> buffer;
	switch (pFormat->GetAction()) {
	case PacketActionType::Decode1:
	case PacketActionType::Encode1:
	{
		uint8_t value = static_cast<uint8_t>(std::stoi(text));
		PacketScript::Encode1(buffer, value);
		break;
	}
	case PacketActionType::Decode2:
	case PacketActionType::Encode2: {
		uint16_t value = static_cast<uint16_t>(std::stoi(text));
		PacketScript::Encode2(buffer, value);
		break;
	}
	case PacketActionType::Decode4:
	case PacketActionType::Encode4: {
		uint32_t value = static_cast<uint32_t>(std::stoi(text));
		PacketScript::Encode4(buffer, value);
		break;
	}
	case PacketActionType::Decode8:
	case PacketActionType::Skip8:
	case PacketActionType::Encode8: {
		uint64_t value = static_cast<uint64_t>(std::stoi(text));
		PacketScript::Encode8(buffer, value);
		break;
	}
	case PacketActionType::DecodeStr:
	case PacketActionType::EncodeStr: {
		size_t uSize = 0;
		PacketScript::EncodeStr(buffer, text, uSize);
		pFormat->SetSize(uSize);
		strLen = std::to_wstring(uSize);
		break;
	}
	case PacketActionType::DecodeBuffer:
	case PacketActionType::EncodeBuffer: {
		size_t uSize = pFormat->GetSize();
		PacketScript::EncodeBuffer(buffer, text, uSize);
		break;
	}
	default:
		m_formatView->MBError(L"Unknown action type");
		return false;
	}
	PacketScript::Buffer2Data(buffer, segment);
	segment += L" "; // Avoid merging bytes between separate lines
	pFormat->SetValue(text);
	pFormat->SetSegment(segment);
	return true;
}

bool FormatController::DecodeSegment(int row, const std::wstring& text, std::wstring& value, std::wstring& strLen)
{
	PacketFormatModel* pFormat = this->getPacketFormatModel(row - 1);
	if (pFormat == nullptr) {
		return false;
	}
	std::vector<uint8_t> buffer;
	bool ok = PacketScript::Data2Buffer(text, buffer);
	size_t pos = 0;
	switch (pFormat->GetAction()) {
	case PacketActionType::Decode1:
	case PacketActionType::Encode1:
	{
		value = std::to_wstring((int8_t)PacketScript::Decode1(buffer, pos));
		break;
	}
	case PacketActionType::Decode2:
	case PacketActionType::Encode2: {
		value = std::to_wstring((int16_t)PacketScript::Decode2(buffer, pos));
		break;
	}
	case PacketActionType::Decode4:
	case PacketActionType::Encode4: {
		value = std::to_wstring((int32_t)PacketScript::Decode4(buffer, pos));
		break;
	}
	case PacketActionType::Decode8:
	case PacketActionType::Skip8:
	case PacketActionType::Encode8: {
		value = std::to_wstring((int64_t)PacketScript::Decode8(buffer, pos));
		break;
	}
	case PacketActionType::DecodeStr:
	case PacketActionType::EncodeStr: {
		size_t uSize = 0;
		value = PacketScript::DecodeStr(buffer, pos, uSize);
		pFormat->SetSize(uSize);
		strLen = std::to_wstring(uSize);
		break;
	}
	case PacketActionType::DecodeBuffer:
	case PacketActionType::EncodeBuffer: {
		size_t uSize = pFormat->GetSize();
		value = PacketScript::DecodeBuffer(buffer, pos, uSize);
		break;
	}
	default:
		m_formatView->MBError(L"Unknown action type");
		return false;
	}
	pFormat->SetValue(value);
	pFormat->SetSegment(text);
	return true;
}

bool FormatController::UpdateCommnet(int row, const std::wstring& text)
{
	PacketFormatModel* pFormat = this->getPacketFormatModel(row - 1);
	if (pFormat == nullptr) {
		return false;
	}
	pFormat->SetComment(text);
	return true;
}

void FormatController::GenCodes(std::vector<std::wstring>& codes)
{
	const Setting s = m_mainControllerImpl->GetSetting();
	for (const auto& format : m_packetFormatModels) {
		PacketActionType actionType = static_cast<PacketActionType>(format.GetAction());
		size_t index = format.GetIndex();
		std::wstring comment = format.GetComment();
		std::wstring line;
		switch (actionType) {
		case Encode1: {
			wchar_t code[256];
			swprintf_s(code, s.CInPacketDecode1GenCode.c_str(), index);
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Encode2: {
			wchar_t code[256];
			swprintf_s(code, s.CInPacketDecode2GenCode.c_str(), index);
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Encode4: {
			wchar_t code[256];
			swprintf_s(code, s.CInPacketDecode4GenCode.c_str(), index);
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Encode8: {
			wchar_t code[256];
			swprintf_s(code, s.CInPacketDecode8GenCode.c_str(), index);
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case EncodeStr: {
			wchar_t code[256];
			swprintf_s(code, s.CInPacketDecodeStrGenCode.c_str(), index);
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case EncodeBuffer: {
			wchar_t code[256];
			swprintf_s(code, s.CInPacketDecodeBufferGenCode.c_str(), index, format.GetSize());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Decode1: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncode1GenCode.c_str(), format.GetValue().c_str());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Decode2: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncode2GenCode.c_str(), format.GetValue().c_str());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Decode4: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncode4GenCode.c_str(), format.GetValue().c_str());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Decode8: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncode8GenCode.c_str(), format.GetValue().c_str());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case Skip8: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncode8GenCode.c_str(), format.GetValue().c_str());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case DecodeStr: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncodeStrGenCode.c_str(), format.GetValue().c_str());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		case DecodeBuffer: {
			wchar_t code[256];
			swprintf_s(code, s.COutPacketEncodeBufferGenCode.c_str(), format.GetSize());
			line = code;
			if (!comment.empty()) {
				line += kCommentSymbol + comment;
			}
			break;
		}
		default:
			line = L"Unknown Action Type";
		}
		codes.push_back(line);
	}
}

std::wstring FormatController::SendFormatData()
{
	std::wstring data;
	for (const auto& format : m_packetFormatModels) {
		data += format.GetSegment();
	}
	return m_mainControllerImpl->SendFormatData(m_logID, data);
}
