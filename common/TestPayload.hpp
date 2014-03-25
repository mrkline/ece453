#pragma once

#include "Exceptions.hpp"
#include "Payload.hpp"
#include "GameTypes.hpp"

/// A test payload that just contains a JSON object. It could contain anything
class TestPayload : public Payload {

public:

	TestPayload(const Json::Value& object = Json::Value(Json::objectValue)) : val(object) { }

	static std::unique_ptr<TestPayload> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<TestPayload>(new TestPayload(object));
	}

	Json::Value toJSON() const override
	{
		ENFORCE(Exceptions::Exception, val.isObject(), "The value must be an object");
		return val;
	}

	Type getType() const override { return Type::TEST; }

	bool operator==(const TestPayload& o) const { return val == o.val; }

	Json::Value val;
};
