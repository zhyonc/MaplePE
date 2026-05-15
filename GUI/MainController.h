#pragma once
#include "IMainController.h"
#include "GUIServer.h"

class MainView;
class GUIServer;

class MainController final : public IMainController {
public:
	MainController(MainView* view);
	~MainController();
	const Setting& GetSetting() override;
	bool SetSetting(const Setting& s) override;
	std::wstring GetExeDir() override;
	void OnPacketLogModel(PacketLogModel& log) override;
	PacketLogModel GetPacketLogModel(int logID) override;
	const std::vector<PacketLogModel>& GetPacketLogModels() override;
	bool JumpLogItem(int logID) override;
	std::wstring SendData(const int pid, const bool isInPacket, const std::wstring& data);
	std::wstring SendFormatData(int logID, const std::wstring& data) override;
	void ClearPacketLogModel();
	void LoadPacketLogFromJSON(const std::wstring& path);
	void SavePacketLogToJSON(const std::wstring& path, const std::vector<int>& selectedIndices);
private:
	Setting m_setting;
	std::wstring m_exeDir = L"";
	MainView* m_mainView = nullptr;
	GUIServer* m_GUIServer = nullptr;
	std::vector<PacketLogModel> m_packetLogModels;
};