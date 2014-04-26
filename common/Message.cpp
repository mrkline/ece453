#include "Message.hpp"

#include <cassert>
#include <unordered_map>

#include "Exceptions.hpp"
#include "BinaryMessage.hpp"
#include "ResponseMessage.hpp"
#include "SetupMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "ShotMessage.hpp"
#include "MovementMessage.hpp"
#include "TargetControlMessage.hpp"
#include "ExitMessage.hpp"
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
	ENFORCE(ArgumentException, id >= 0, "The ID cannot be negative.");
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
	using Type = Message::Type;

	static const std::unordered_map<Message::Type, StaticString> nameLookup = {
		{Type::EMPTY, StaticString("empty")},
		{Type::RESPONSE, StaticString("response")},
		{Type::SETUP, StaticString("setup")},
		{Type::START, StaticString("start")},
		{Type::STOP, StaticString("stop")},
		{Type::STATUS, StaticString("status")},
		{Type::STATUS_RESPONSE, StaticString("status response")},
		{Type::RESULTS, StaticString("results")},
		{Type::RESULTS_RESPONSE, StaticString("results response")},
		{Type::SHOT, StaticString("shot")},
		{Type::MOVEMENT, StaticString("movement")},
		{Type::TARGET_CONTROL, StaticString("target control")},
		{Type::EXIT, StaticString("exit")},
		{Type::TEST, StaticString("test")}
		// {Message::Type::UNKNOWN, "unknown"}
	};

	Value ret(objectValue);
	ret[idKey] = id;
	ret[typeKey] = nameLookup.at(getType());
	return ret;
}

std::vector<uint8_t> Message::toBinary() const
{
	// Send a binary message with no body
	array<uint8_t, 0> nothing;
	return makeBinaryMessage(getType(), (uint16_t)id, begin(nothing), end(nothing));
}

bool Message::operator==(const Message& o) const
{
	return id == o.id;
}

std::unique_ptr<Message> JSONToMessage(const Json::Value& object)
{
	using Type = Message::Type;

	static const std::unordered_map<std::string, Message::Type> typeLookup = {
		{"empty", Type::EMPTY},
		{"response", Type::RESPONSE},
		{"setup", Type::SETUP},
		{"start", Type::START},
		{"stop", Type::STOP},
		{"status", Type::STATUS},
		{"status response", Type::STATUS_RESPONSE},
		{"results", Type::RESULTS},
		{"results response", Type::RESULTS_RESPONSE},
		{"shot", Type::SHOT},
		{"movement", Type::MOVEMENT},
		{"target control", Type::TARGET_CONTROL},
		{"exit", Type::EXIT},
		{"test", Type::TEST}
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

		case Type::SHOT:
			return ShotMessage::fromJSON(object);

		case Type::MOVEMENT:
			return MovementMessage::fromJSON(object);

		case Type::TARGET_CONTROL:
			return TargetControlMessage::fromJSON(object);

		case Type::EXIT:
			return ExitMessage::fromJSON(object);

		case Type::TEST:
			return TestMessage::fromJSON(object);

		default:
			assert(false);
	}
}
