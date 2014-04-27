#include "BinaryMessage.hpp"

#include <boost/crc.hpp>

namespace {
	/// These bytes were chosen completely randomly
	const std::array<uint8_t, 2> magicBytes = { 'f', 'u' };
}

const std::array<uint8_t, 2>& getMagicBytes()
{
	return magicBytes;
}

uint16_t getCRC(const std::vector<uint8_t> data)
{
	boost::crc_ccitt_type gen; // Get a 16-bit
	gen.process_bytes(data.data(), data.size());
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
