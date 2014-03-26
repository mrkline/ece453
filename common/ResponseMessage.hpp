#pragma once

#include <memory>
#include <string>

#include "Message.hpp"

/// A response payload for a message
class ResponseMessage : public Message {

public:

	enum class Code {
		OK,
		INTERNAL_ERROR,
		UNKNOWN_REQUEST,
		INVALID_REQUEST,
		UNSUPPORTED_REQUEST
	};

	ResponseMessage(int idNum, int respTo, Code c, const std::string& msg);

	static std::unique_ptr<ResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	const int respondingTo;

	const Code code;

	const std::string message;

	virtual Type getType() const override { return Type::RESPONSE; }

	bool operator==(const Message& o) const override;
};
