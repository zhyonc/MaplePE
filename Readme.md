# MaplePE
A MapleStory Packet Editor that can parse packet structures and send custom packets to the client

## Features
- Extract InPacket and OutPacket information from the target process
- Log and format packet structures as list items
- Search or jump by ID to quickly locate a packet item
- Edit packet data and send it to the client
- Generate simple code from package structures
- Filter specific opcodes to avoid logging certain packets (e.g. 1,2,3,4)
- Import and export packet logs in JSON format

## How to use
### Ready
- Some version addresses can be found [here](https://docs.google.com/spreadsheets/d/1-LqA-OWgOuT-Xy3i73Jy5F_uVmogaGvmYpH4mLDSPhg)
- Find the following addresses that need to be hooked:
	- `CInPacket::Decode1`
	- `CInPacket::Decode2`
	- `CInPacket::Decode4`
	- `CInPacket::Decode8` (optional)
	- `CInPacket::Skip8` (optional)
	- `CInPacket::DecodeStr`
	- `CInPacket::DecodeBuffer`
	- `COutPacket::Encode1`
	- `COutPacket::Encode2`
	- `COutPacket::Encode4`
	- `COutPacket::Encode8` (optional)
	- `COutPacket::EncodeStr`
	- `COutPacket::EncodeBuffer`
	- `COutPacket::MakeBufferList`
	- `CClientSocket::ProcessPacket`
	- `CClientSocket::SendPacket` (optional)
	- `SendPacketEH` (optional)
- Run `MaplePE.exe` and click the icon to open the `SettingView`
- Fill in the function addresses and save setting
- Use your launcher to start the game client

### Inject
- Each exsits process is injected only once
- Click the `Patch` button will locate the target process ID
- Click the `InjectDLL` button will inject `Packet.dll` into the target process
- There are several possible reasons why the injection might fail
	- Permission denied: Please run `MaplePE.exe` as administrator
	- Missing `Packet.dll`: Some security software may delete unknown DLL files
	- Game Anti-Cheat: Please bypass Security Client if it blocks `WriteProcessMemory` 
	- Injection protection: Some third-party launchers implement memory protection to prevent DLL injection

### Edit
- Select a packet item will display the full bytes in the bottom
- Edit bytes and send them to the client
- Double-click a packet item will open the `FormatView`
- Edit values or sequences and send them to the client

### Search
- Press `Crtl+F` key or select search menu to open the `SearchView`
- Input search conditions to find a packet item 
- Double-click a packet item will locate it in the `MainView`
- Press `Crtl+G` key or select jump menu to open the `JumpView`
- Input item id will locate it in the `MainView`

## How this tool works
- `MaplePE.exe` runs a UDP server to receive packet information
- Inject `Packet.dll` into target process using `CreateRemoteThread` with `LoadLibraryW`
- `Packet.dll` hooks functions related to writing and reading packets to detect packet structures
- `Packet.dll` uses a UDP client to send packet information that includes packet structures and the return address
- `MaplePE.exe` logs and formats packet structures as list items
- `MaplePE.exe` allows users to edit packet data and send it to the client
- Before sending a packet via `MaplePE.exe`, the server must respond with at least one packet to ensure that the CClientSocket instance can be correctly specified
## Hook Detection
Since some versions after BB add `hook-detection` in `CClientSocket::SendPacket`  
Hook `MakeBufferList` instead of `SendPacket` to avoid client crash when sending OutPacket after DLL injection  
But sending `OutPacket` via `MaplePE.exe` still causes crash because the `SendPacket` retAddr is not from `MapleStory`  
Please use CE fill `90` to disable the following crash conditions if you want to send `OutPacket` via `MaplePE.exe`  
```c
void __fastcall CClientSocket::SendPacket(CClientSocket *this, int a2, COutPacket *oPacket){
  	if ( (int)retaddr < 4198400 || (int)retaddr > (int)&loc_7FFFFE + 4198402 )
	{
		JUMPOUT(0); // 1st crash
	}
	if ( (unsigned int)retaddr <= (unsigned int)IWzProperty::IWzProperty || (ZFatalSection *)retaddr >= _sync.m_pLock )
	{
		MEMORY[0](this, a2); // 2nd crash
	}
	if ( m_hSocket && m_hSocket != -1 && !this->m_ctxConnect.lAddr._m_uCount )
	{
		if ( (unsigned int)retaddr <= (unsigned int)IWzProperty::IWzProperty || (ZFatalSection *)retaddr >= m_pLock )
		{
			MEMORY[0](); // 3rd crash
		}
		COutPacket::MakeBufferList(oPacket, &this->m_lpSendBuff, 0x5Fu, &this->m_uSeqSnd, 1, this->m_uSeqSnd);
		if ( (unsigned int)retaddr <= (unsigned int)IWzProperty::IWzProperty || (ZFatalSection *)retaddr >= m_pLock )
		{
			MEMORY[0](); // 4th crash
		}
		this->m_uSeqSnd = CIGCipher::innoHash((unsigned __int8 *)&this->m_uSeqSnd, 4, 0);
		if ( (unsigned int)retaddr <= (unsigned int)IWzProperty::IWzProperty || (ZFatalSection *)retaddr >= m_pLock )
		{
			MEMORY[0](); // 5th crash
		}
		CClientSocket::Flush(this);
		if ( (unsigned int)retaddr <= (unsigned int)IWzProperty::IWzProperty || (ZFatalSection *)retaddr >= m_pLock )
		{
			MEMORY[0](); // 6th crash
    }
	}
}
```

## Development Environment
- IDE: Visual Studio 2022
	- C++ desktop application development
	- C++ MFC for the latest v143 build tools (x86 & x64)
	- `MFC-GridCtrl` linked as a static library (.lib)
- Networking library: WinSock2 (Windows Sockets API)
- Hooking library: [MemorySDK](https://github.com/zhyonc/MemorySDK) linked as a static library (.lib)
- To maintain control ID order
	- If a new control is added via Resource View, please manually copy its ID to `resource_ids.h`
	- Remove all auto-generated control IDs from `resource.h` and include `resource_ids.h` in `resource.h`

## Credits
[RirePE](https://github.com/Riremito/RirePE) offers similar features to this tool  
[KaedeEditor](https://github.com/Riremito/KaedeEditor) provides function addresses for some versions through AOB scanning  
[MFC-GridCtrl](https://github.com/ChrisMaunder/MFC-GridCtrl) is a grid control for displaying packet structures in a tabular format