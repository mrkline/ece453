#include "TestMessage.hpp"

#include "Exceptions.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;

namespace {

const StaticString payloadKey("test payload");

} // end anonymous namespace

TestMessage::TestMessage(int id, const Json::Value& object) :
	Message(id),
	val(object)
{
}

std::unique_ptr<TestMessage> TestMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(payloadKey), "Test message is missing its payload");

	const Value& payloadValue = object[payloadKey];

	ENFORCE(IOException, payloadValue.isObject(), "Test message payload is not a JSON object");

	return std::unique_ptr<TestMessage>(new TestMessage(msg->id, object));
}

Json::Value TestMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[payloadKey] = val;

	return ret;
}

bool TestMessage::operator==(const Message& o) const
{
	if (!Message::operator==(o))
		return false;

	auto tm = dynamic_cast<const TestMessage*>(&o);

	if (tm == nullptr)
		return false;

	return val == tm->val;
}
