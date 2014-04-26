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
