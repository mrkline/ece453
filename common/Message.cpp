#include "Message.hpp"

#include <unordered_map>

#include "Exceptions.hpp"
#include "ResponseMessage.hpp"
#include "SetupMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "TestMessage.hpp"

using namespace Exceptions;
using namespace Json;

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString idKey("id");

} // end anonymous namespace

const StaticString Message::typeKey("type");

Message::Message(int idNum) :
	id(idNum)
{
	ENFORCE(ArgumentException, id > 0, "The ID cannot be negative.");
}

std::unique_ptr<Message> Message::fromJSON(const Json::Value& object)
{
	ENFORCE(IOException, object.isMember(idKey), "The message contains no ID");

	const Value& idValue = object[idKey];

	ENFORCE(IOException, idValue.isInt(), "The message's ID field is not an integer.");
	return std::unique_ptr<Message>(new Message(idValue.asInt()));
}

Json::Value Message::toJSON() const
{
	static const std::unordered_map<Message::Type, StaticString> nameLookup = {
		{Message::Type::EMPTY, StaticString("empty")},
		{Message::Type::RESPONSE, StaticString("response")},
		{Message::Type::SETUP, StaticString("setup")},
		{Message::Type::START, StaticString("start")},
		{Message::Type::STOP, StaticString("stop")},
		{Message::Type::STATUS, StaticString("status")},
		{Message::Type::STATUS_RESPONSE, StaticString("status response")},
		{Message::Type::RESULTS, StaticString("results")},
		{Message::Type::RESULTS_RESPONSE, StaticString("results response")},
		{Message::Type::TEST, StaticString("test")}
		// {Message::Type::UNKNOWN, "unknown"}
	};

	Value ret(objectValue);
	ret[idKey] = id;
	ret[typeKey] = nameLookup.at(getType());
	return ret;
}

bool Message::operator==(const Message& o) const
{
	return id == o.id;
}

std::unique_ptr<Message> JSONToMessage(const Json::Value& object)
{
	static const std::unordered_map<std::string, Message::Type> typeLookup = {
		{"empty", Message::Type::EMPTY},
		{"response", Message::Type::RESPONSE},
		{"setup", Message::Type::SETUP},
		{"start", Message::Type::START},
		{"stop", Message::Type::STOP},
		{"status", Message::Type::STATUS},
		{"status response", Message::Type::STATUS_RESPONSE},
		{"results", Message::Type::RESULTS},
		{"results response", Message::Type::RESULTS_RESPONSE},
		{"test", Message::Type::TEST}
		// {"unknown", Message::Type::UNKNOWN}
	};

	ENFORCE(IOException, object.isMember(Message::typeKey), "JSON object has no type field");

	const Value& typeVal = object[Message::typeKey];

	ENFORCE(IOException, typeVal.isString(), "The JSON object's type field is not a string");

	auto it = typeLookup.find(typeVal.asString());
	
	ENFORCE(IOException, it != end(typeLookup), "The JSON object's type field is unknown");

	using Type = Message::Type;

	switch (it->second) {

		case Type::EMPTY:
			return Message::fromJSON(object);

		case Type::RESPONSE:
			return ResponseMessage::fromJSON(object);

		case Type::SETUP:
			return SetupMessage::fromJSON(object);

		case Type::START:
			return StartMessage::fromJSON(object);

		case Type::STOP:
			return StopMessage::fromJSON(object);

		case Type::STATUS:
			return StatusMessage::fromJSON(object);

		case Type::STATUS_RESPONSE:
			return StatusResponseMessage::fromJSON(object);

		case Type::RESULTS:
			return ResultsMessage::fromJSON(object);

		case Type::RESULTS_RESPONSE:
			return ResultsResponseMessage::fromJSON(object);

		case Type::TEST:
			return TestMessage::fromJSON(object);

		default:
			THROW(Exception, "Code error: Forgot to parse a message type");
	}
}
