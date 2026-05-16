#pragma once
#include <sstream>
#include <iomanip>
#include "PacketScript.h"

namespace PacketScript {

	template<typename Stream>
	void InitHexStream(Stream& ss) {
		using CharT = typename Stream::char_type;
		ss << std::uppercase << std::hex << std::setfill(CharT('0'));
	}

	std::string Int2Hex(ULONG_PTR v)
	{
		std::ostringstream ss;
		InitHexStream(ss);
		ss << "0x" << std::setw(8) << v;
		return ss.str();
	}

	std::wstring Int2HexW(ULONG_PTR v) {
		std::wstringstream ss;
		InitHexStream(ss);
		ss << L"0x" << std::setw(8) << v;
		return ss.str();
	}

	ULONG_PTR HexToInt(const std::string& str)
	{
		if (str.empty()) {
			return 0;
		}
		return static_cast<ULONG_PTR>(std::stoull(str, nullptr, 0));
	}

	ULONG_PTR HexToIntW(const std::wstring& wstr)
	{
		if (wstr.empty()) {
			return 0;
		}
		return static_cast<ULONG_PTR>(std::stoull(wstr, nullptr, 0));
	}

	void Buffer2Data(std::vector<uint8_t>& buffer, std::wstring& data) {
		std::wstringstream ss;
		InitHexStream(ss);

		for (size_t i = 0; i < buffer.size(); ++i) {
			if (i > 0) {
				ss << L' ';
			}
			ss << std::setw(2) << static_cast<int>(buffer[i]);
		}
		data = ss.str();
	}

	bool Data2Buffer(const std::wstring& data, std::vector<uint8_t>& buffer) {
		buffer.clear();
		std::wstringstream ss(data);
		std::wstring byteStr;

		while (ss >> byteStr) {
			try {
				int byteVal = std::stoul(byteStr, nullptr, 16);
				if (byteVal <= 0xFF) {
					buffer.push_back(static_cast<uint8_t>(byteVal));
				}
			}
			catch (const std::exception&) {
				return false;
			}
		}
		return true;
	}

	std::wstring FormatSystemTime(const SYSTEMTIME& st) {
		std::wstringstream ss;
		ss << std::setfill(L'0') << std::setw(4) << st.wYear << L"-"
			<< std::setw(2) << st.wMonth << L"-"
			<< std::setw(2) << st.wDay << L" "
			<< std::setw(2) << st.wHour << L":"
			<< std::setw(2) << st.wMinute << L":"
			<< std::setw(2) << st.wSecond;
		return ss.str();
	}

	SYSTEMTIME ParseSystemTime(const std::wstring& timeStr) {
		SYSTEMTIME st{};
		swscanf_s(timeStr.c_str(), L"%4hu-%2hu-%2hu %2hu:%2hu:%2hu",
			&st.wYear, &st.wMonth, &st.wDay,
			&st.wHour, &st.wMinute, &st.wSecond);
		return st;
	}

	std::wstring MultiByte2WideChar(const char* pStr, size_t uSize) {
		int strLen = static_cast<int>(uSize);
		int wcharLen = MultiByteToWideChar(kSystemAnsiCodePage, 0, pStr, strLen, nullptr, 0);
		if (wcharLen <= 0) {
			return std::wstring();
		}
		std::wstring result(wcharLen, 0);
		MultiByteToWideChar(kSystemAnsiCodePage, 0, pStr, strLen, &result[0], wcharLen);
		return result;
	}

	std::string WideChar2MultiByte(const std::wstring& wstr) {
		int wstrLen = static_cast<int>(wstr.size());
		int byteLen = WideCharToMultiByte(kSystemAnsiCodePage, 0, wstr.c_str(), wstrLen, nullptr, 0, nullptr, nullptr);
		if (byteLen <= 0) {
			return std::string();
		}
		std::string result(byteLen, 0);
		WideCharToMultiByte(kSystemAnsiCodePage, 0, wstr.c_str(), wstrLen, &result[0], byteLen, nullptr, nullptr);
		return result;
	}

	bool IsTimeValid(const SYSTEMTIME& st) {
		// ST_START -> ST_END
		if (st.wYear >= 1900 && st.wYear <= 2079) {
			return st.wMonth >= 1 && st.wMonth <= 12 && st.wDay >= 1 && st.wDay <= 31;
		}
		// ST_EMPTY
		if (st.wYear == 0 && st.wMonth == 0 && st.wDay == 0) {
			return true;
		}
		// ST_ZERO
		if (st.wYear == 1 && st.wMonth == 1 && st.wDay == 1) {
			return true;
		}
		// ST_ZERO_OVERFLOW
		if (st.wYear == 1754 && st.wMonth == 8 && st.wDay == 30 && st.wHour == 22 && st.wMinute == 43 && st.wSecond == 41) {
			return true;
		}
		// ST_SQL_MIN
		if (st.wYear == 1753 && st.wMonth == 1 && st.wDay == 1) {
			return true;
		}
		// ST_SQL_MIN_DATE
		if (st.wYear == 1000 && st.wMonth == 1 && st.wDay == 1) {
			return true;
		}
		return false;
	}

	std::wstring GetHexSegment(const std::wstring& data, size_t& segmentPos, size_t uSize) {
		size_t segmentLen = uSize * 3;
		if (segmentPos >= data.length()) {
			return std::wstring();
		}
		std::wstring segment;
		size_t remainLen = data.length() - segmentPos;
		size_t extractLen = (segmentLen < remainLen) ? segmentLen : remainLen;
		segment = data.substr(segmentPos, extractLen);
		segmentPos += extractLen;
		return segment;
	}

	Opcode DecodeOp(const std::vector<uint8_t>& buffer, size_t& pos)
	{
		uint16_t op = Decode2(buffer, pos);
		return static_cast<Opcode>(op);
	}

	uint8_t Decode1(const std::vector<uint8_t>& buffer, size_t& pos) {
		if (pos + 1 > buffer.size()) {
			return 0;
		}
		uint8_t value = buffer[pos];
		pos++;
		return value;
	}

	uint16_t Decode2(const std::vector<uint8_t>& buffer, size_t& pos) {
		if (pos + 2 > buffer.size()) {
			return 0;
		}
		uint16_t value = static_cast<uint16_t>(buffer[pos] | (buffer[pos + 1] << 8));
		pos += 2;
		return value;
	}

	uint32_t Decode4(const std::vector<uint8_t>& buffer, size_t& pos) {
		if (pos + 4 > buffer.size()) {
			return 0;
		}
		uint32_t value = 0;
		for (size_t i = 0; i < 4; i++)
		{
			value |= static_cast<uint32_t>(buffer[pos + i]) << (i * 8);
		}
		pos += 4;
		return value;
	}

	uint64_t Decode8(const std::vector<uint8_t>& buffer, size_t& pos) {
		if (pos + 8 > buffer.size()) {
			return 0;
		}
		uint64_t value = 0;
		for (size_t i = 0; i < 8; i++)
		{
			value |= static_cast<uint64_t>(buffer[pos + i]) << (i * 8);
		}
		pos += 8;
		return value;
	}

	SYSTEMTIME* DecodeFT(const std::vector<uint8_t>& buffer, size_t& pos, int64_t& value) {
		value = static_cast<int64_t>(Decode8(buffer, pos));
		if (value == 0 || value == -1) {
			return nullptr;
		}
		if (value == FT_EMPTY) {
			SYSTEMTIME st{};
			return &st;
		}
		if (value == FT_ZERO) {
			SYSTEMTIME st{};
			st.wYear = 1;
			st.wMonth = 1;
			st.wDay = 1;
			return &st;
		}
		if (value == FT_SQL_MIN_DATE) {
			SYSTEMTIME st{};
			st.wYear = 1000;
			st.wMonth = 1;
			st.wDay = 1;
			return &st;
		}
		FILETIME ft{};
		SYSTEMTIME st{};
		ft.dwLowDateTime = static_cast<uint32_t>(value & 0xFFFFFFFF);
		ft.dwHighDateTime = static_cast<uint32_t>(value >> 32);
		if (FileTimeToSystemTime(&ft, &st)) {
			return &st;
		}
		return nullptr;
	}

	std::wstring DecodeStr(const std::vector<uint8_t>& buffer, size_t& pos, size_t& uSize) {
		uSize = Decode2(buffer, pos);
		if (uSize == 0 || pos + uSize > buffer.size()) {
			return std::wstring();
		}
		const char* pStr = reinterpret_cast<const char*>(&buffer[pos]);
		std::wstring wstr = MultiByte2WideChar(pStr, uSize);
		pos += uSize;
		return wstr;
	}

	std::wstring DecodeStr(const std::vector<uint8_t>& buffer, size_t& pos)
	{
		size_t uSize = 0;
		return DecodeStr(buffer, pos, uSize);
	}

	std::wstring DecodeBuffer(const std::vector<uint8_t>& buffer, size_t& pos, size_t uSize) {
		if (pos + uSize > buffer.size()) {
			return std::wstring();
		}
		switch (uSize) {
		case kCharacterNameLength: {
			const char* pStr = reinterpret_cast<const char*>(&buffer[pos]);
			std::wstring wStr = MultiByte2WideChar(pStr, kCharacterNameLength);
			pos += kCharacterNameLength;
			return wStr;
		}
		case kBuffer1:
			return std::to_wstring(static_cast<int8_t>(Decode1(buffer, pos)));
		case kBuffer2:
			return std::to_wstring(static_cast<int16_t>(Decode2(buffer, pos)));
		case kBuffer4:
			return std::to_wstring(static_cast<int32_t>(Decode4(buffer, pos)));
		case kBuffer8: {
			// Decode FT or number
			int64_t value = 0;
			SYSTEMTIME* st = DecodeFT(buffer, pos, value);
			if (st == nullptr || !IsTimeValid(*st)) {
				return std::to_wstring(value);
			}
			return FormatSystemTime(*st);
		}
		default:
			pos += uSize;
			return std::wstring();
		}
	}

	void EncodeOp(std::vector<uint8_t>& buffer, Opcode value)
	{
		uint16_t op = static_cast<uint16_t>(value);
		Encode2(buffer, op);
	}

	void Encode1(std::vector<uint8_t>& buffer, uint8_t value) {
		buffer.push_back(value);
	}

	void Encode2(std::vector<uint8_t>& buffer, uint16_t value) {
		const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
		buffer.insert(buffer.end(), p, p + 2);
	}

	void Encode4(std::vector<uint8_t>& buffer, uint32_t value) {
		const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
		buffer.insert(buffer.end(), p, p + 4);
	}

	void Encode8(std::vector<uint8_t>& buffer, uint64_t value) {
		const uint8_t* p = reinterpret_cast<const uint8_t*>(&value);
		buffer.insert(buffer.end(), p, p + 8);
	}

	void EncodeFT(std::vector<uint8_t>& buffer, const std::wstring& data) {
		FILETIME ft{};
		SYSTEMTIME st = ParseSystemTime(data);
		if (IsTimeValid(st) && SystemTimeToFileTime(&st, &ft)) {
			uint64_t rawFT = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
			Encode8(buffer, rawFT);
		}
		else {
			std::vector<uint8_t> temp;
			bool ok = Data2Buffer(data, temp);
			if (ok) {
				buffer.insert(buffer.end(), temp.begin(), temp.begin() + 8);
			}
			else {
				Encode8(buffer, 0);
			}
		}
	}

	void EncodeStr(std::vector<uint8_t>& buffer, const std::wstring& wstr, size_t& uSize) {
		std::string str = WideChar2MultiByte(wstr);
		uSize = str.size();
		Encode2(buffer, static_cast<uint16_t>(uSize));
		if (uSize == 0) {
			return;
		}
		const char* pStr = str.c_str();
		const uint8_t* p = reinterpret_cast<const uint8_t*>(pStr);
		buffer.insert(buffer.end(), p, p + uSize);
	}

	void EncodeStr(std::vector<uint8_t>& buffer, const std::wstring& wstr)
	{
		size_t uSize = 0;
		EncodeStr(buffer, wstr, uSize);
	}

	void EncodeBuffer(std::vector<uint8_t>& buffer, const std::wstring& data, size_t uSize) {
		switch (uSize) {
		case kCharacterNameLength: {
			std::string str = WideChar2MultiByte(data);
			size_t strLen = str.size();
			if (strLen > kCharacterNameLength) {
				str = str.substr(0, kCharacterNameLength);
			}
			else {
				str.append(kCharacterNameLength - strLen, '\0');
			}
			const char* pStr = str.c_str();
			const uint8_t* p = reinterpret_cast<const uint8_t*>(pStr);
			buffer.insert(buffer.end(), p, p + kCharacterNameLength);
			break;
		}
		case kBuffer8: {
			EncodeFT(buffer, data);
			break;
		}
		default:
			std::vector<uint8_t> tempBuffer;
			bool ok = Data2Buffer(data, tempBuffer);
			if (ok) {
				buffer.insert(buffer.end(), tempBuffer.begin(), tempBuffer.end());
			}
			else {
				std::vector<uint8_t> emptyBuffer(uSize);
				buffer.insert(buffer.end(), emptyBuffer.begin(), emptyBuffer.end());
			}
		}
	}

}

