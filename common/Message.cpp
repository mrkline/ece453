#include "Message.hpp"

#include <cassert>

#include "Exceptions.hpp"
#include "ResponsePayload.hpp"
#include "SetupPayload.hpp"
#include "StatusResponsePayload.hpp"
#include "ResultsResponsePayload.hpp"
#include "TestPayload.hpp"

using namespace Exceptions;
using namespace Json;

namespace {

const int currentVersion = 1;

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString typeKey("type");
const StaticString versionKey("version");
const StaticString idKey("id");
const StaticString payloadKey("payload");

} // end anonymous namespace

Message::Message(Payload::Type t, int ver, int idNum, std::unique_ptr<Payload>&& load) :
	type(t),
	version(ver),
	id(idNum),
	payload(std::move(load))
{
	enforce<ArgumentException>(version > 0, "The version number cannot be negative.", __FUNCTION__);
	enforce<ArgumentException>(id > 0, "The ID cannot be negative.", __FUNCTION__);
	enforce<ArgumentException>(load == nullptr || type == payload->getType(), "The type must match the payload's type.",
	                           __FUNCTION__);

	switch(type) {
		case Payload::Type::RESPONSE:
		case Payload::Type::SETUP:
		case Payload::Type::STATUS_RESPONSE:
		case Payload::Type::RESULTS_RESPONSE:
		case Payload::Type::TEST:
			enforce<IOException>(payload != nullptr, "For this message type, a payload is required.",
			                     __FUNCTION__);
			break;

		case Payload::Type::START:
		case Payload::Type::STOP:
		case Payload::Type::STATUS:
		case Payload::Type::RESULTS:
			enforce<IOException>(payload == nullptr, "For this message type, the payload should be null",
			                     __FUNCTION__);
			break;

		default:
			throw Exception("Error in program logic: we forgot to handle some payload", __FUNCTION__);
	}
}

std::unique_ptr<Message> Message::fromJSON(const Json::Value& object)
{
	enforce<IOException>(object.isMember(typeKey), "The message contains no type", __FUNCTION__);
	enforce<IOException>(object.isMember(versionKey), "The message contains no version", __FUNCTION__);
	enforce<IOException>(object.isMember(idKey), "The message contains no ID", __FUNCTION__);
	enforce<IOException>(object.isMember(payloadKey), "The message contains no payload", __FUNCTION__);

	const Value& typeValue = object[typeKey];
	const Value& versionValue = object[versionKey];
	const Value& idValue = object[idKey];
	const Value& payloadValue = object[payloadKey];

	enforce<IOException>(typeValue.isString(), "The message's type field is not a string.", __FUNCTION__);
	enforce<IOException>(versionValue.isInt(), "The message's version field is not an integer.", __FUNCTION__);
	enforce<IOException>(idValue.isInt(), "The message's ID field is not an integer.", __FUNCTION__);
	enforce<IOException>(payloadValue.isNull() || payloadValue.isObject(), "The message's payload is invalid.",
	                     __FUNCTION__);

	std::unique_ptr<Payload> load;
	Payload::Type type = Payload::nameToType(typeValue.asString());

	switch(type) {
		case Payload::Type::RESPONSE:
		case Payload::Type::SETUP:
		case Payload::Type::STATUS_RESPONSE:
		case Payload::Type::RESULTS_RESPONSE:
		case Payload::Type::TEST:
			enforce<IOException>(payloadValue.isObject(), "For this message type, a payload is required.",
			                     __FUNCTION__);
			break;

		case Payload::Type::START:
		case Payload::Type::STOP:
		case Payload::Type::STATUS:
		case Payload::Type::RESULTS:
			enforce<IOException>(payloadValue.isNull(), "For this message type, the payload should be null",
			                     __FUNCTION__);
			break;

		default:
			throw IOException("The message's payload type is unknown.", __FUNCTION__);
	}

	switch(type) {
		case Payload::Type::RESPONSE:
			load = ResponsePayload::fromJSON(payloadValue);
			break;

		case Payload::Type::SETUP:
			load = SetupPayload::fromJSON(payloadValue);
			break;

		case Payload::Type::STATUS_RESPONSE:
			load = StatusResponsePayload::fromJSON(payloadValue);
			break;

		case Payload::Type::RESULTS_RESPONSE:
			load = ResultsResponsePayload::fromJSON(payloadValue);
			break;

		case Payload::Type::TEST:
			load = TestPayload::fromJSON(payloadValue);

		case Payload::Type::START:
		case Payload::Type::STOP:
		case Payload::Type::STATUS:
		case Payload::Type::RESULTS:
			break; // Load stays null

		default:
			throw Exception("Error in program logic: we forgot to parse some payload", __FUNCTION__);
	}

	return std::unique_ptr<Message>(new Message(type, versionValue.asInt(), idValue.asInt(), move(load)));
}

Json::Value Message::toJSON() const
{
	Value ret(objectValue);
	ret[typeKey] = Payload::typeToName(type);
	ret[versionKey] = version;
	ret[idKey] = id;
	if (payload != nullptr)
		ret[payloadKey] = payload->toJSON();
	else
		ret[payloadKey] = Value(); // A null value
	return ret;
}

bool Message::operator==(const Message& o) const
{
	const bool basics = type == o.type
		&& version == o.version
		&& id == o.id;
	
	if (basics) {
		switch(type) {
			case Payload::Type::RESPONSE:
				return *static_cast<ResponsePayload*>(payload.get())
					== *static_cast<ResponsePayload*>(o.payload.get());
				break;

			case Payload::Type::SETUP:
				return *static_cast<SetupPayload*>(payload.get())
					== *static_cast<SetupPayload*>(o.payload.get());
				break;

			case Payload::Type::STATUS_RESPONSE:
				return *static_cast<StatusResponsePayload*>(payload.get())
					== *static_cast<StatusResponsePayload*>(o.payload.get());
				break;

			case Payload::Type::RESULTS_RESPONSE:
				return *static_cast<ResultsResponsePayload*>(payload.get())
					== *static_cast<ResultsResponsePayload*>(o.payload.get());
				break;

			case Payload::Type::TEST:
				return *static_cast<TestPayload*>(payload.get())
					== *static_cast<TestPayload*>(o.payload.get());
				break;

			case Payload::Type::START:
			case Payload::Type::STOP:
			case Payload::Type::STATUS:
			case Payload::Type::RESULTS:
				return true;
				break;

			default:
				throw Exception("Error in program logic: we forgot to handle some payload", __FUNCTION__);
		}
	}
	else {
		return false;
	}
}
