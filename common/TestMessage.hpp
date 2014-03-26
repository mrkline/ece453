#pragma once

#include "Message.hpp"

/// A test payload that just contains a JSON object. It could contain anything
class TestMessage : public Message {

public:

	TestMessage(int id, const Json::Value& object = Json::Value());

	static std::unique_ptr<TestMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return Type::TEST; }

	bool operator==(const Message& o) const override;

	Json::Value val;
};
