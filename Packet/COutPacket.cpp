#include "pch.h"
#include "COutPacket.h"
#include "Router.h"

namespace {

	static std::set<uint16_t> gFilterOpcodeSet;

	static std::map<void*, std::vector<PacketAction>> gActionsMap;

	static std::map<void*, uint16_t> gOpcodeMap;

	static bool IsPayload(void* ecx) {
		OutPacket* oPacket = static_cast<OutPacket*>(ecx);
		return oPacket->m_uOffset > 0;
	}

	static ULONG_PTR GetCaller(int depth) {
		void* stack[3] = {};
		USHORT frames = RtlCaptureStackBackTrace(0, 3, stack, nullptr);
		if (depth >= 0 && depth < 3 && depth < frames) {
			return (ULONG_PTR)stack[depth];
		}
		return 0;
	}
}

namespace COutPacket {

	void UpdateFilterOpcodeSet(std::wstring& opcodes)
	{
		std::set<uint16_t> tempSet;
		std::wstringstream ss(opcodes);
		std::wstring token;

		while (std::getline(ss, token, L',')) {
			try {
				uint16_t opcode = static_cast<uint16_t>(std::stoul(token, nullptr, 0));
				tempSet.insert(opcode);
			}
			catch (const std::exception&) {
				DEBUG(L"Unknown filter opcodes")
			}
		}

		if (tempSet.empty()) {
			DEBUG(L"Failed to update COutPacketFilterOpcodeSet: no valid opcodes found");
			return;
		}

		gFilterOpcodeSet = std::move(tempSet);
	}

	bool IsFilterOpcode(uint16_t opcode)
	{
		return gFilterOpcodeSet.find(opcode) != gFilterOpcodeSet.end();
	}

	uint16_t GetOpcode(void* key)
	{
		auto it = gOpcodeMap.find(key);
		if (it == gOpcodeMap.end()) {
			return 0;
		}
		return it->second;
	}

	std::vector<PacketAction>* GetActions(void* key)
	{
		auto it = gActionsMap.find(key);
		if (it == gActionsMap.end()) {
			return nullptr;
		}
		return &it->second;
	}

	void DeleteActions(void* key)
	{
		gActionsMap.erase(key);
	}

	void SetActions(void* key, const std::vector<PacketAction>& actions)
	{
		gActionsMap[key] = actions;
	}


	void(__thiscall* Encode1)(void* ecx, uint8_t n) = nullptr;
	void __fastcall Encode1_Hook(void* ecx, FASTCALL_EDX_PADDING uint8_t n) {
		auto action = PacketAction{ PacketActionType::Encode1,1,(ULONG_PTR)_ReturnAddress() };
		auto actions = GetActions(ecx);
		if (actions == nullptr) {
			if (IsFilterOpcode(n)) {
				gActionsMap[ecx] = std::vector<PacketAction>{};
			}
			else {
				// COutPacket::COutPacket(opcodeIs1Byte)
				ULONG_PTR callerAddr = GetCaller(2);
				if (callerAddr > 0) {
					action.RetAddr = callerAddr;
				}
				gActionsMap[ecx] = std::vector<PacketAction>{ action };
				gOpcodeMap[ecx] = n;
			}
		}
		else if (!actions->empty()) {
			actions->push_back(action);
		}
		return Encode1(ecx, n);
	}

	void(__thiscall* Encode2)(void* ecx, uint16_t n) = nullptr;
	void __fastcall Encode2_Hook(void* ecx, FASTCALL_EDX_PADDING uint16_t n) {
		auto action = PacketAction{ PacketActionType::Encode2,2,(ULONG_PTR)_ReturnAddress() };
		auto actions = GetActions(ecx);
		if (actions == nullptr) {
			if (IsFilterOpcode(n)) {
				gActionsMap[ecx] = std::vector<PacketAction>{};
			}
			else {
				// COutPacket::COutPacket(opcode)
				ULONG_PTR callerAddr = GetCaller(2);
				if (callerAddr > 0) {
					action.RetAddr = callerAddr;
				}
				gActionsMap[ecx] = std::vector<PacketAction>{ action };
				gOpcodeMap[ecx] = n;
			}
		}
		else if (!actions->empty()) {
			actions->push_back(action);
		}
		return Encode2(ecx, n);
	}

	void(__thiscall* Encode4)(void* ecx, uint32_t n) = nullptr;
	void __fastcall Encode4_Hook(void* ecx, FASTCALL_EDX_PADDING uint32_t n) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::Encode4,4,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return Encode4(ecx, n);
	}

	void(__thiscall* Encode8)(void* ecx, uint64_t n) = nullptr;
	void __fastcall Encode8_Hook(void* ecx, FASTCALL_EDX_PADDING uint64_t n) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::Encode8,8,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return Encode8(ecx, n);
	}

	void(__thiscall* EncodeStr)(void* ecx, char* s) = nullptr;
	void __fastcall EncodeStr_Hook(void* ecx, FASTCALL_EDX_PADDING char* s) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::EncodeStr,0,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return EncodeStr(ecx, s);
	}

	void(__thiscall* EncodeBuffer)(void* ecx, uint8_t* p, uint32_t uSize) = nullptr;
	void __fastcall EncodeBuffer_Hook(void* ecx, FASTCALL_EDX_PADDING uint8_t* p, uint32_t uSize) {
		if (IsPayload(ecx)) {
			auto actions = GetActions(ecx);
			if (actions != nullptr && !actions->empty()) {
				actions->push_back(PacketAction{ PacketActionType::EncodeBuffer,uSize,(ULONG_PTR)_ReturnAddress() });
			}
		}
		return EncodeBuffer(ecx, p, uSize);
	}

	void(__thiscall* MakeBufferList)(void* ecx, void* l, uint16_t uSeqBase, uint32_t* puSeqKey, int bEnc, uint32_t dwKey) = nullptr;
	void __fastcall MakeBufferList_Hook(void* ecx, FASTCALL_EDX_PADDING  void* l, uint16_t uSeqBase, uint32_t* puSeqKey, int bEnc, uint32_t dwKey) {
		auto actions = GetActions(ecx);
		if (actions != nullptr && !actions->empty()) {
			OutPacket* oPacket = static_cast<OutPacket*>(ecx);
			PacketInfo info{};
			info.PID = Router::kPID;
			info.Index = Router::gPacketIndex++;
			info.IsInPacket = false;
			info.Opcode = GetOpcode(ecx);
			info.Payload = std::vector<uint8_t>(oPacket->m_aSendBuff, oPacket->m_aSendBuff + oPacket->m_uOffset);
			info.Actions = *actions;
			Router::SendPacketInfo(info);
		}
		DeleteActions(ecx);
		return MakeBufferList(ecx, l, uSeqBase, puSeqKey, bEnc, dwKey);
	}
}
