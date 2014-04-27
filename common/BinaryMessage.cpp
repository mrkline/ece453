#include "BinaryMessage.hpp"

#include <boost/crc.hpp>

namespace {
	/// These bytes were chosen completely randomly
	const std::array<uint8_t, 2> magicBytes = { 'f', 'u' };
}

namespace BinaryMessage {

const std::array<uint8_t, 2>& getMagicBytes()
{
	return magicBytes;
}

uint16_t getCRC(const void* data, size_t len)
{
	boost::crc_ccitt_type gen; // Get a 16-bit
	gen.process_bytes(data, len);
	return gen.checksum();
}

void appendInt(std::vector<uint8_t>& buf, uint16_t i)
{
	buf.emplace_back((uint8_t)((i & 0xff00) >> 8));
	buf.emplace_back((uint8_t)((i & 0x00ff) >> 0));
}

void appendInt(std::vector<uint8_t>& buf, uint32_t i)
{
	buf.emplace_back((uint8_t)((i & 0xff000000) >> 24));
	buf.emplace_back((uint8_t)((i & 0x00ff0000) >> 16));
	buf.emplace_back((uint8_t)((i & 0x0000ff00) >> 8));
	buf.emplace_back((uint8_t)((i & 0x000000ff) >> 0));
}

uint16_t extractUInt16(const uint8_t* buf)
{
	return (uint16_t)(
		((uint16_t)buf[0] << 8) +
		((uint16_t)buf[1] << 0));
}

uint32_t extractUInt32(const uint8_t* buf)
{
	return ((uint32_t)buf[0] << 24)
	     + ((uint32_t)buf[1] << 16)
	     + ((uint32_t)buf[2] << 8)
	     + ((uint32_t)buf[3] << 0);
}

bool isValidMessage(const uint8_t* buf, size_t len, std::string* why)
{
	// We should have at least room for 9 bytes
	// (2 magic bytes, type, 2 for ID, 2 for length, 2 for checksum)
	if (len < 9) {
		if (why != nullptr)
			*why = "There is not enough room for even an empty message.";

		return false;
	}

	const uint8_t* cursor = buf;

	// Try to get the magic bytes
	const auto& magic = getMagicBytes();
	if (cursor[0] != magic[0] || cursor[1] != magic[1]) {
		if (why != nullptr)
			*why = "The first two bytes do not match our magic bytes";

		return false;
	}
	cursor += 2;

	// Get the type
	if((int8_t)*cursor < 0 || *cursor > (int8_t)Message::Type::UNKNOWN) {
		if (why != nullptr)
			*why = "The message type is an invalid value";
		return false;
	}
	++cursor;

	// Skip the ID
	cursor += sizeof(message_id_t);

	// Get the length
	uint16_t payloadLength = extractUInt16(cursor);
	if (len < 9u + payloadLength) {
		if (why != nullptr)
			*why = "There is not enough room for the message's reported payload";
		return false;
	}
	cursor += 2;

	// Get the CRC of everything up to the CRC
	uint16_t crc = getCRC(buf, 7 + payloadLength);

	// Compare it to our actual CRC
	cursor += payloadLength;
	if (extractUInt16(cursor) != crc) {
		if (why != nullptr)
			*why = "The CRC does not match the expected one";
		return false;
	}

	return true;
}

} // end namespace BinaryMessage
