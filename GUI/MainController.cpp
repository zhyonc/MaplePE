#include "pch.h"
#include "MainView.h"
#include "MainController.h"
#include <fstream>
#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

MainController::MainController(MainView* view) {
	m_mainView = view;
	m_exeDir = GetExecutableDir(NULL);
	bool loadOK = LoadSetting(m_exeDir, m_setting);
	if (!loadOK) {
		bool saveOK = SaveSetting(m_exeDir, m_setting);
		if (!saveOK) {
			m_mainView->MBError(L"Failed to save setting on first load");
		}
	}
	m_GUIServer = new GUIServer(this);
	m_GUIServer->Run();
}

MainController::~MainController()
{
	delete(m_GUIServer);
	m_GUIServer = nullptr;
}

const Setting& MainController::GetSetting()
{
	return m_setting;
}

bool MainController::SetSetting(const Setting& s)
{
	bool ok = SaveSetting(m_exeDir, s);
	if (!ok) {
		return false;
	}
	if (s.CInPacketFilterOpcodes != m_setting.CInPacketFilterOpcodes || s.COutPacketFilterOpcodes != m_setting.COutPacketFilterOpcodes) {
		m_GUIServer->BroadcastFilterOpcodes(s.CInPacketFilterOpcodes, s.COutPacketFilterOpcodes);
	}
	m_setting = s;
	return true;
}

std::wstring MainController::GetExeDir()
{
	return m_exeDir;
}

void MainController::OnPacketLogModel(PacketLogModel& log)
{
	log.SetIsTypeHeader1Byte(m_setting.IsTypeHeader1Byte);
	m_packetLogModels.push_back(log);
	m_mainView->InsertPacketLogItem(m_packetLogModels.size() - 1, log);
}

PacketLogModel MainController::GetPacketLogModel(int logID)
{
	if (logID < 0 || static_cast<size_t>(logID) >= m_packetLogModels.size()) {
		std::wstring err = L"Failed to get packet log model with logID " + std::to_wstring(logID);
		this->m_mainView->MBError(err);
		return PacketLogModel{};
	}
	return m_packetLogModels[logID];
}

const std::vector<PacketLogModel>& MainController::GetPacketLogModels()
{
	return m_packetLogModels;
}

bool MainController::JumpLogItem(int logID)
{
	if (logID < 0 || static_cast<size_t>(logID) >= m_packetLogModels.size()) {
		return false;
	}
	m_mainView->JumpLogItem(logID);
	return true;
}

std::wstring MainController::SendData(const int pid, const bool isInPacket, const std::wstring& data)
{
	PacketLogModel log(pid, isInPacket, data);
	if (pid == 0) {
		return m_GUIServer->BroadcastPacketInfo(log);
	}
	return m_GUIServer->SendPacketInfo(log);
}

std::wstring MainController::SendFormatData(int logID, const std::wstring& data)
{
	PacketLogModel log = this->GetPacketLogModel(logID);
	log.SetData(data);
	return m_GUIServer->SendPacketInfo(log);
}

void MainController::ClearPacketLogModel()
{
	m_packetLogModels.clear();
}

void MainController::LoadPacketLogFromJSON(const std::wstring& path)
{
	std::ifstream in(path);
	if (!in.is_open()) {
		return;
	}

	ordered_json j;
	in >> j;

	for (const auto& item : j) {
		std::vector<PacketAction> actions;
		for (const auto& actionJson : item.at("actions")) {
			PacketAction action{};
			action.Type = PacketActionType(actionJson.at("type").get<int>());
			action.Size = actionJson.at("size").get<int>();
			action.RetAddr = PacketScript::HexToInt(actionJson.at("retAddr").get<std::string>());
			actions.push_back(action);
		}
		std::string data = item.at("data").get<std::string>();
		std::wstring dataW = PacketScript::MultiByte2WideChar(data.c_str(), data.size());
		PacketLogModel log(
			item.at("pid").get<int>(),
			item.at("index").get<int>(),
			item.at("isInPacket").get<bool>(),
			item.at("isTypeHeader1Byte").get<bool>(),
			item.at("length").get<int>(),
			item.at("opcode").get<int>(),
			dataW,
			actions
		);
		m_packetLogModels.push_back(log);
		m_mainView->InsertPacketLogItem(m_packetLogModels.size() - 1, log);
	}
}

void MainController::SavePacketLogToJSON(const std::wstring& path, const std::vector<int>& selectedIndices)
{
	ordered_json j = ordered_json::array();
	auto exportLog = [&](const auto& log) {
		ordered_json actionsJSON = ordered_json::array();
		for (const auto& action : log.GetActions()) {
			actionsJSON.push_back({
				{"type", action.Type},
				{"size", action.Size},
				{"retAddr", PacketScript::Int2Hex(action.RetAddr)}
				});
		}
		j.push_back({
			{"pid", log.GetPID()},
			{"index", log.GetIndex()},
			{"isInPacket", log.IsInPacket()},
			{"isTypeHeader1Byte", log.IsTypeHeader1Byte()},
			{"length", log.GetLength()},
			{"opcode", log.GetOpcode()},
			{"data", PacketScript::WideChar2MultiByte(log.GetData())},
			{"actions", actionsJSON},
			});
		};
	if (selectedIndices.empty()) {
		for (const auto& log : m_packetLogModels) {
			exportLog(log);
		}
	}
	else {
		for (int idx : selectedIndices) {
			PacketLogModel log = GetPacketLogModel(idx);
			if (log.IsEmpty()) {
				continue;
			}
			exportLog(log);
		}
	}
	std::ofstream out(path);
	out << j.dump(4);
}