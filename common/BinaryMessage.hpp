#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "Exceptions.hpp"
#include "Message.hpp"

using namespace std;
using namespace Exceptions;

const std::array<uint8_t, 2>& getMagicBytes();

uint16_t getCRC(const std::vector<uint8_t> data);

void appendInt(std::vector<uint8_t>& buf, uint16_t i);

inline void appendInt(std::vector<uint8_t>& buf, int16_t i) { appendInt(buf, (uint16_t)i); }

void appendInt(std::vector<uint8_t>& buf, uint32_t i);

inline void appendInt(std::vector<uint8_t>& buf, int32_t i) { appendInt(buf, (uint32_t)i); }

template <typename InputIt>
std::vector<uint8_t> makeBinaryMessage(Message::Type type, message_id_t id,
                                       InputIt payloadStart, InputIt payloadEnd)
{
	if (payloadStart > payloadEnd)
		THROW(ArgumentOutOfRangeException, "The start iterator is after the end iterator");

	std::vector<uint8_t> ret;
	// The message will be at least 9 bytes long (2 magic bytes, type, 2 for ID, 4 for checksum)
	ret.reserve(9);

	// Append magic bytes
	auto magic = getMagicBytes();
	ret.emplace_back(magic[0]);
	ret.emplace_back(magic[1]);

	// Append type
	ret.emplace_back((uint8_t)type);

	// Append ID
	appendInt(ret, id);

	// Append payload
	ret.insert(end(ret), payloadStart, payloadEnd);

	// Append CRC
	appendInt(ret, getCRC(ret));

	return ret;
}
