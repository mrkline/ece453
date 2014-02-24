#pragma once

#include <memory>
#include <string>

#include <jsoncpp/json/json.h>

#include "Payload.hpp"

/// A response payload for a message
class ResponsePayload : public Payload {

public:

	enum Code {
		RC_OK,
		RC_INTERNAL_ERROR,
		RC_UNKNOWN_REQUEST,
		RC_INVALID_REQUEST,
		RC_UNSUPPORTED_REQUEST
	};

	ResponsePayload(int respTo, Code c, const std::string& msg);

	static std::unique_ptr<ResponsePayload> fromJSON(const Json::Value& object);

	const int respondingTo;

	const Code code;

	const std::string message;

	virtual Type getType() const override { return PT_RESPONSE; }
};
