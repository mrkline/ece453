#pragma once

#include <memory>
#include <string>

#include "Message.hpp"

/// A response payload for a message
class ResponseMessage : public Message {

public:

	enum class Code {
		OK, ///< Everything went as expected.
		INTERNAL_ERROR, ///< Something went wrong inside the system.
		UNKNOWN_REQUEST, ///< The system couldn't make sense of the request.
		INVALID_REQUEST, ///< The request was valid but was given at an inappropriate time.
		UNSUPPORTED_REQUEST ///< The request was valid but the system could not fulfill it.
	};

	ResponseMessage(message_id_t idNum, message_id_t respTo, Code c, const std::string& msg);

#ifdef WITH_JSON
	static std::unique_ptr<ResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	const message_id_t respondingTo;

	const Code code;

	const std::string message;

	virtual Type getType() const override { return Type::RESPONSE; }

	bool operator==(const Message& o) const override;
};
