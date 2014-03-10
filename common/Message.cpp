#include "Message.hpp"

#include <cassert>

#include "Exceptions.hpp"
#include "ResponsePayload.hpp"
#include "SetupPayload.hpp"
#include "StatusResponsePayload.hpp"
#include "ResultsResponsePayload.hpp"

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
	enforce<ArgumentException>(load == nullptr || type == load->getType(), "The type must match the payload's type.",
	                           __FUNCTION__);
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
		case Payload::PT_RESPONSE:
		case Payload::PT_SETUP:
		case Payload::PT_STATUS_RESPONSE:
		case Payload::PT_RESULTS_RESPONSE:
			enforce<IOException>(payloadValue.isObject(), "For this message type, a payload is required.",
			                     __FUNCTION__);
			break;

		case Payload::PT_START:
		case Payload::PT_STOP:
		case Payload::PT_STATUS:
		case Payload::PT_RESULTS:
			enforce<IOException>(payloadValue.isNull(), "For this message type, the payload should be null",
			                     __FUNCTION__);
			break;

		default:
			throw IOException("The message's payload type is unknown.", __FUNCTION__);
	}

	switch(type) {
		case Payload::PT_RESPONSE:
			load = ResponsePayload::fromJSON(payloadValue);
			break;

		case Payload::PT_SETUP:
			load = SetupPayload::fromJSON(payloadValue);
			break;

		case Payload::PT_STATUS_RESPONSE:
			load = StatusResponsePayload::fromJSON(payloadValue);
			break;

		case Payload::PT_RESULTS_RESPONSE:
			load = ResultsResponsePayload::fromJSON(payloadValue);
			break;

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
	ret[payloadKey] = payload->toJSON();
	return ret;
}
