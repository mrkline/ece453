#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "Exceptions.hpp"
#include "Message.hpp"

using namespace std;
using namespace Exceptions;

namespace BinaryMessage {

const std::array<uint8_t, 2>& getMagicBytes();

uint16_t getCRC(const void* data, size_t len);

inline uint16_t getCRC(const std::vector<uint8_t> data) { return getCRC(data.data(), data.size()); }

// Don't let us accidentally call this for bytes, which would get promoted to a larger type

inline void appendInt(std::vector<uint8_t>&, uint8_t) { std::terminate(); }

inline void appendInt(std::vector<uint8_t>&, int8_t) { std::terminate(); }

void appendInt(std::vector<uint8_t>& buf, uint16_t i);

inline void appendInt(std::vector<uint8_t>& buf, int16_t i) { appendInt(buf, (uint16_t)i); }

void appendInt(std::vector<uint8_t>& buf, uint32_t i);

inline void appendInt(std::vector<uint8_t>& buf, int32_t i) { appendInt(buf, (uint32_t)i); }

uint16_t extractUInt16(const uint8_t* buf);

inline int16_t extractInt16(const uint8_t* buf) { return (int16_t)extractUInt16(buf); }

uint32_t extractUInt32(const uint8_t* buf);

inline int32_t extractInt32(const uint8_t* buf) { return (int32_t)extractUInt32(buf); }

template <typename InputIt>
std::vector<uint8_t> makeMessage(Message::Type type, message_id_t id,
                                 InputIt payloadStart, InputIt payloadEnd)
{
	if (payloadStart > payloadEnd)
		THROW(ArgumentOutOfRangeException, "The start iterator is after the end iterator");

	std::vector<uint8_t> ret;
	// The message will be at least 9 bytes long
	// (2 magic bytes, type, 2 for ID, 2 for length, 2 for checksum)
	ret.reserve(9);

	// Append magic bytes
	auto magic = getMagicBytes();
	ret.emplace_back(magic[0]);
	ret.emplace_back(magic[1]);

	// Append type
	ret.emplace_back((int8_t)type);

	// Append ID
	appendInt(ret, id);

	//Append length
	appendInt(ret, (uint16_t)std::distance(payloadStart, payloadEnd));

	// Append payload
	ret.insert(end(ret), payloadStart, payloadEnd);

	// Append CRC
	appendInt(ret, getCRC(ret));

	return ret;
}

/**
 * \brief Returns true if a given sequence of bytes is a valid binary message
 * \param buf The buffer in which the bytes are held
 * \param len The length of the buffer
 * \param why If not null, assigned the reason the message was invalid
 *            if the function returns false
 * \returns true if the bytes pointed to by buf is a valid binary message
 */
bool isValidMessage(const uint8_t* buf, size_t len, std::string* why = nullptr);

/**
 * \brief Gets the message type from a binary message
 * \param buf A pointer to the start of the message
 * \returns The message type
 * \warning This assumes you have validated the message already.
 */
inline Message::Type getType(const uint8_t* buf)
{
	return (Message::Type)buf[2];
}

/**
 * \brief Gets the message ID from a binary message
 * \param buf A pointer to the start of the message
 * \returns The message ID
 * \warning This assumes you have validated the message already.
 */
inline message_id_t getID(const uint8_t* buf)
{
	// Yell at us so we remember to change this function if we change the message ID length
	static_assert(sizeof(message_id_t) == 2, "Someone changed the message ID length");
	return extractUInt16(&buf[3]);
}

/**
 * \brief Given a buffer, get a pointer to the payload and also get the size
 * \param buf A pointer to the start of the message
 * \param len A reference to a variable that will receive the payload's size
 * \post len contains the length of the payload
 * \returns A pointer to the payload
 * \warning This assumes you have validated the message already.
 */
inline std::pair<const uint8_t*, size_t> getPayload(const uint8_t* buf)
{
	buf += 5;
	size_t len = extractUInt16(buf);
	buf += 2;
	return std::pair<const uint8_t*, size_t>(buf, len);
}

} // end namespace BinaryMessage
