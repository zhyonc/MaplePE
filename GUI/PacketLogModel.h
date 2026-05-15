#pragma once

class PacketLogModel {
public:
	PacketLogModel() = default;
	PacketLogModel(PacketInfo& info);
	PacketLogModel(int pid, bool isInPacket, const std::wstring& data);
	PacketLogModel(int pid, int index, bool isInPacket, bool isTypeHeader1Byte, int length, int opcode, std::wstring& data, std::vector<PacketAction>& actions);
	const bool IsEmpty() const;
	const int GetPID() const;
	const std::wstring GetPIDStr() const;
	const int GetIndex() const;
	const std::wstring GetIndexStr() const;
	const std::wstring GetType() const;
	const bool IsInPacket() const;
	const bool IsTypeHeader1Byte() const;
	const int GetLength() const;
	const std::wstring GetLengthStr() const;
	const int GetOpcode() const;
	const std::wstring GetOpcodeStr() const;
	const std::wstring& GetData() const;
	const std::vector<PacketAction>& GetActions() const;
	void SetIsTypeHeader1Byte(bool isTypeHeader1Byte);
	void SetData(const std::wstring& data);
private:
	uint32_t m_pid = 0;
	uint32_t m_index = 0;
	bool m_isInPacket = false;
	bool m_isTypeHeader1Byte = false;
	size_t m_length = 0;
	uint16_t m_opcode = 0;
	std::wstring m_data;
	std::vector<PacketAction> m_actions;
};