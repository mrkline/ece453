#ifdef WITH_JSON

#pragma once

#include "Exceptions.hpp"
#include "Message.hpp"

/// A test payload that just contains a JSON object. It could contain anything
class TestMessage : public Message {

public:

	TestMessage(message_id_t id, const Json::Value& object = Json::Value());

	static std::unique_ptr<TestMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	Type getType() const override { return Type::TEST; }

	bool operator==(const Message& o) const override;

	Json::Value val;
};

#endif
