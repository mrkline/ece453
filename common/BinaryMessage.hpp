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
	ret.emplace_back((uint8_t)((id & 0xff00) >> 8));
	ret.emplace_back((uint8_t)((id & 0x00ff) >> 0));

	// Append payload
	ret.insert(end(ret), payloadStart, payloadEnd);

	// Append CRC
	uint16_t crc = getCRC(ret);
	ret.emplace_back((uint8_t)((crc & 0xff00) >> 8));
	ret.emplace_back((uint8_t)((crc & 0x00ff) >> 0));

	return ret;
}
