#pragma once

#include <memory>
#include <string>

#include "Payload.hpp"

/// A response payload for a message
class ResponsePayload : public Payload {

public:

	enum class Code {
		OK,
		INTERNAL_ERROR,
		UNKNOWN_REQUEST,
		INVALID_REQUEST,
		UNSUPPORTED_REQUEST
	};

	ResponsePayload(int respTo, Code c, const std::string& msg);

	static std::unique_ptr<ResponsePayload> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	const int respondingTo;

	const Code code;

	const std::string message;

	virtual Type getType() const override { return Type::RESPONSE; }

	bool operator==(const ResponsePayload& o) const;
};
