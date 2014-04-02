#include "ResponseMessage.hpp"

#include <unordered_map>

#include "Exceptions.hpp"

using namespace Json;
using namespace Exceptions;
using namespace std;

namespace std
{
	// Hasher for ResponseMessage::Code
	template<>
	struct hash<ResponseMessage::Code> {
		size_t operator()(ResponseMessage::Code t) const { return std::hash<int>()((int)t); }
	};
}

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString respondingToKey("responding to");
const StaticString codeKey("code");
const StaticString messageKey("message");

const unordered_map<string, ResponseMessage::Code> nameToCode = {
	{"ok", ResponseMessage::Code::OK},
	{"internal error", ResponseMessage::Code::INTERNAL_ERROR},
	{"unknown request", ResponseMessage::Code::UNKNOWN_REQUEST},
	{"invalid request", ResponseMessage::Code::INVALID_REQUEST},
	{"unsupported request", ResponseMessage::Code::UNSUPPORTED_REQUEST}
};

const unordered_map<ResponseMessage::Code, StaticString> codeToName = {
	{ResponseMessage::Code::OK, StaticString("ok")},
	{ResponseMessage::Code::INTERNAL_ERROR, StaticString("internal error")},
	{ResponseMessage::Code::UNKNOWN_REQUEST, StaticString("unknown request")},
	{ResponseMessage::Code::INVALID_REQUEST, StaticString("invalid request")},
	{ResponseMessage::Code::UNSUPPORTED_REQUEST, StaticString("unsupported request")}
};

} // End anonymous namespace

ResponseMessage::ResponseMessage(int idNum, int respTo, Code c, const std::string& msg) :
	Message(idNum),
	respondingTo(respTo),
	code(c),
	message(msg)
{
	ENFORCE(ArgumentException, respondingTo > 0, "Responding to a negative ID does not make sense.");
}

std::unique_ptr<ResponseMessage> ResponseMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(respondingToKey), "Response payload is missing the response ID");

	ENFORCE(IOException, object.isMember(codeKey), "Response payload is missing the code");
	ENFORCE(IOException, object.isMember(messageKey), "Response payload is missing the message");

	const Value& respondingToValue = object[respondingToKey];
	const Value& codeValue = object[codeKey];
	const Value& messageValue = object[messageKey];

	ENFORCE(IOException, respondingToValue.isInt(), "The response ID is not an integer.");
	ENFORCE(IOException, codeValue.isString(), "The response code is not a string.");
	ENFORCE(IOException, messageValue.isString(), "The response message is not a string.");

	const auto codeIt = nameToCode.find(codeValue.asString());
	ENFORCE(IOException, codeIt != end(nameToCode), "The response code is invalid.");

	return std::unique_ptr<ResponseMessage>(
		new ResponseMessage(msg->id,
		                    respondingToValue.asInt(), codeIt->second, messageValue.asString()));
}

Json::Value ResponseMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[respondingToKey] = respondingTo;
	ret[codeKey] = codeToName.at(code);
	ret[messageKey] = message;

	return ret;
}

bool ResponseMessage::operator==(const Message& o) const
{
	if (!Message::operator==(o))
		return false;

	auto rm = dynamic_cast<const ResponseMessage*>(&o);

	if (rm == nullptr)
		return false;

	return respondingTo == rm->respondingTo
		&& code == rm->code
		&& message == rm->message;
}
