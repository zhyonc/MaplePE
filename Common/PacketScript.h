#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include "Opcode.h"

#define FT_EMPTY -505255104000000000 // ST 0000-00-00 00:00:00
#define FT_ZERO -504911232000000000 // ST 0001-01-01 00:00:00
#define FT_ZERO_OVERFLOW 48491090210000000 // ST 1754-08-30 22:43:41
#define FT_SQL_MIN 47966688000000000 // ST 1753-01-01 00:00:00
#define FT_SQL_MIN_DATE -189657504000000000 // ST 1000-01-01 00:00:00
#define FT_START 94354848000000000 // ST 1900-01-01 00:00:00
#define FT_END 150842304000000000 // ST 2079-01-01 00:00:00

namespace {
	const UINT kSystemAnsiCodePage = GetACP();
}

namespace PacketScript {

	const size_t kCharacterNameLength = 13;
	const size_t kBuffer1 = 1;
	const size_t kBuffer2 = 2;
	const size_t kBuffer4 = 4;
	const size_t kBuffer8 = 8;

	template<typename Stream>
	void InitHexStream(Stream& ss);

	std::string Int2Hex(ULONG_PTR v);

	std::wstring Int2HexW(ULONG_PTR v);

	ULONG_PTR HexToInt(const std::string& str);

	ULONG_PTR HexToIntW(const std::wstring& wstr);

	void Buffer2Data(std::vector<uint8_t>& buffer, std::wstring& data);

	bool Data2Buffer(const std::wstring& data, std::vector<uint8_t>& buffer);

	std::wstring FormatSystemTime(const SYSTEMTIME& st);

	SYSTEMTIME ParseSystemTime(const std::wstring& timeStr);

	std::wstring MultiByte2WideChar(const char* pStr, size_t uSize);

	std::string WideChar2MultiByte(const std::wstring& wstr);

	bool IsTimeValid(const SYSTEMTIME& st);

	std::wstring GetHexSegment(const std::wstring& data, size_t& segmentPos, size_t uSize);

	Opcode DecodeOp(const std::vector<uint8_t>& buffer, size_t& pos);

	uint8_t Decode1(const std::vector<uint8_t>& buffer, size_t& pos);

	uint16_t Decode2(const std::vector<uint8_t>& buffer, size_t& pos);

	uint32_t Decode4(const std::vector<uint8_t>& buffer, size_t& pos);

	uint64_t Decode8(const std::vector<uint8_t>& buffer, size_t& pos);

	SYSTEMTIME* DecodeFT(const std::vector<uint8_t>& buffer, size_t& pos, int64_t& value);

	std::wstring DecodeStr(const std::vector<uint8_t>& buffer, size_t& pos, size_t& uSize);

	std::wstring DecodeStr(const std::vector<uint8_t>& buffer, size_t& pos);

	std::wstring DecodeBuffer(const std::vector<uint8_t>& buffer, size_t& pos, size_t uSize);

	void EncodeOp(std::vector<uint8_t>& buffer, Opcode value);

	void Encode1(std::vector<uint8_t>& buffer, uint8_t value);

	void Encode2(std::vector<uint8_t>& buffer, uint16_t value);

	void Encode4(std::vector<uint8_t>& buffer, uint32_t value);

	void Encode8(std::vector<uint8_t>& buffer, uint64_t value);

	void EncodeFT(std::vector<uint8_t>& buffer, const std::wstring& data);

	void EncodeStr(std::vector<uint8_t>& buffer, const std::wstring& wstr, size_t& uSize);

	void EncodeStr(std::vector<uint8_t>& buffer, const std::wstring& wstr);

	void EncodeBuffer(std::vector<uint8_t>& buffer, const std::wstring& data, size_t uSize);
}
