#include "Message.hpp"

#include <cassert>

#include "Exceptions.hpp"
#include "BinaryMessage.hpp"
#include "ResponseMessage.hpp"
#include "QueryMessage.hpp"
#include "SetupMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "ShotMessage.hpp"
#include "TargetControlMessage.hpp"
#include "ExitMessage.hpp"
#include "TestMessage.hpp"

using namespace Exceptions;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString idKey("id");
#endif

} // end anonymous namespace

#ifdef WITH_JSON
const StaticString Message::typeKey("type");
#endif

const std::unordered_map<Message::Type, std::string> Message::nameLookup = {
	{Message::Type::EMPTY, "empty"},
	{Message::Type::RESPONSE, "response"},
	{Message::Type::QUERY, "query"},
	{Message::Type::SETUP, "setup"},
	{Message::Type::START, "start"},
	{Message::Type::STOP, "stop"},
	{Message::Type::STATUS, "status"},
	{Message::Type::STATUS_RESPONSE, "status response"},
	{Message::Type::RESULTS, "results"},
	{Message::Type::RESULTS_RESPONSE, "results response"},
	{Message::Type::SHOT, "shot"},
	{Message::Type::MOVEMENT, "movement"},
	{Message::Type::TARGET_CONTROL, "target control"},
	{Message::Type::EXIT, "exit"},
	{Message::Type::TEST, "test"}
	// {Message::Type::UNKNOWN, "unknown"}
};

const std::unordered_map<std::string, Message::Type> Message::typeLookup = {
	{"empty", Message::Type::EMPTY},
	{"response", Message::Type::RESPONSE},
	{"query", Message::Type::QUERY},
	{"setup", Message::Type::SETUP},
	{"start", Message::Type::START},
	{"stop", Message::Type::STOP},
	{"status", Message::Type::STATUS},
	{"status response", Message::Type::STATUS_RESPONSE},
	{"results", Message::Type::RESULTS},
	{"results response", Message::Type::RESULTS_RESPONSE},
	{"shot", Message::Type::SHOT},
	{"movement", Message::Type::MOVEMENT},
	{"target control", Message::Type::TARGET_CONTROL},
	{"exit", Message::Type::EXIT},
	{"test", Message::Type::TEST}
	// {"unknown", Message::Type::UNKNOWN}
};



Message::Message(message_id_t idNum) :
	id(idNum)
{
}

#ifdef WITH_JSON
std::unique_ptr<Message> Message::fromJSON(const Json::Value& object)
{
	ENFORCE(IOException, object.isMember(idKey), "The message contains no ID");

	const Value& idValue = object[idKey];

	ENFORCE(IOException, idValue.isInt(), "The message's ID field is not an integer.");
	const int rawID = idValue.asInt();
	ENFORCE(IOException, rawID >= 0, "The message's ID must be positive");
	return std::unique_ptr<Message>(new Message((message_id_t)rawID));
}

Json::Value Message::toJSON() const
{
	Value ret(objectValue);
	ret[idKey] = id;
	ret[typeKey] = nameLookup.at(getType());
	return ret;
}
#endif

std::unique_ptr<Message> Message::fromBinary(uint8_t* buf, size_t len)
{
	using namespace BinaryMessage;
	ENFORCE(IOException, isValidMessage(buf, len), "Message is not valid");
	return std::unique_ptr<Message>(new Message(getID(buf)));
}

std::vector<uint8_t> Message::toBinary() const
{
	// Send a binary message with whatever payload is needed
	auto payload = getBinaryPayload();
	return BinaryMessage::makeMessage(getType(), id, begin(payload), end(payload));
}

std::vector<uint8_t> Message::getBinaryPayload() const
{
	// An empty message has no payload.
	return vector<uint8_t>();
}

bool Message::operator==(const Message& o) const
{
	return id == o.id;
}

#ifdef WITH_JSON
std::unique_ptr<Message> JSONToMessage(const Json::Value& object)
{
	using Type = Message::Type;

	ENFORCE(IOException, object.isMember(Message::typeKey), "JSON object has no type field");

	const Value& typeVal = object[Message::typeKey];

	ENFORCE(IOException, typeVal.isString(), "The JSON object's type field is not a string");

	auto it = Message::typeLookup.find(typeVal.asString());

	ENFORCE(IOException, it != end(Message::typeLookup), "The JSON object's type field is unknown");

	using Type = Message::Type;

	switch (it->second) {

		case Type::EMPTY:
			return Message::fromJSON(object);

		case Type::RESPONSE:
			return ResponseMessage::fromJSON(object);

		case Type::QUERY:
			return QueryMessage::fromJSON(object);

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
#endif

std::unique_ptr<Message> binaryToMessage(uint8_t* buf, size_t len)
{
	using Type = Message::Type;

	// May be a duplicate, but hell, unless it's a performance concern, we'll take it.
	// If we want to eliminate the duplicate, take a look at the fromBinary functions
	ENFORCE(IOException, BinaryMessage::isValidMessage(buf, len), "Message is not valid");

	auto type = BinaryMessage::getType(buf);

	using Type = Message::Type;

	switch (type) {

		case Type::EMPTY:
			return Message::fromBinary(buf, len);

		case Type::RESPONSE:
			return ResponseMessage::fromBinary(buf, len);

		case Type::QUERY:
			return QueryMessage::fromBinary(buf, len);

		case Type::START:
			return StartMessage::fromBinary(buf, len);

		case Type::STOP:
			return StopMessage::fromBinary(buf, len);

		case Type::SHOT:
			return ShotMessage::fromBinary(buf, len);

		case Type::TARGET_CONTROL:
			return TargetControlMessage::fromBinary(buf, len);

		// We don't have binary versions of these
		case Type::STATUS:
		case Type::STATUS_RESPONSE:
		case Type::RESULTS:
		case Type::RESULTS_RESPONSE:
		case Type::SETUP:
		case Type::TEST:
		case Type::EXIT:
		default:
			assert(false);
	}
}
