#pragma once

#include <memory>
#include <string>

#include "Message.hpp"

/// A response acknowledging a previously-sent message
class ResponseMessage : public Message {

public:

	enum class Code : int8_t {
		OK, ///< Everything went as expected.
		INTERNAL_ERROR, ///< Something went wrong inside the system.
		UNKNOWN_REQUEST, ///< The system couldn't make sense of the request.
		INVALID_REQUEST, ///< The request was valid but was given at an inappropriate time.
		UNSUPPORTED_REQUEST ///< The request was valid but the system could not fulfill it.
	};

	/**
	 * \brief Constructs a response message
	 * \param idNum The ID of this message
	 * \param respTo The ID of the message we are acknowledging
	 * \param c The response code
	 * \param msg An optional string to go along with the message.
	 *            Note that binary response messages do not contain this string
	 */
	ResponseMessage(message_id_t idNum, message_id_t respTo, Code c, const std::string& msg = "");

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<ResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<ResponseMessage> fromBinary(uint8_t* buf, size_t len);

	/**
	 * \brief Gets the response's binary payload
	 *
	 * The payload consists of:
	 * - A 16-bit usnsigned integer representing the ID of the message being acknowledged
	 *   (i.e. respondingTo)
	 * - An unsigned byte representing the response code (i.e. code)
	 *
	 * Note that the string is omitted.
	 */
	std::vector<uint8_t> getBinaryPayload() const override;

	const message_id_t respondingTo;

	const Code code;

	const std::string message;

	virtual Type getType() const override { return Type::RESPONSE; }

	bool operator==(const Message& o) const override;
};
