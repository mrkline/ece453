#include "ResponsePayload.hpp"

#include <unordered_map>

#include "Exceptions.hpp"

using namespace Json;
using namespace Exceptions;
using namespace std;

namespace std
{
	// Hasher for ResponsePayload::Code
	template<>
	struct hash<ResponsePayload::Code> {
		size_t operator()(ResponsePayload::Code t) const { return std::hash<int>()((int)t); }
	};
}

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString respondingToKey("responding to");
const StaticString codeKey("code");
const StaticString messageKey("message");

const unordered_map<string, ResponsePayload::Code> nameToCode = {
	{"ok", ResponsePayload::Code::OK},
	{"internal error", ResponsePayload::Code::INTERNAL_ERROR},
	{"unknown request", ResponsePayload::Code::UNKNOWN_REQUEST},
	{"invalid request", ResponsePayload::Code::INVALID_REQUEST},
	{"unsupported request", ResponsePayload::Code::UNSUPPORTED_REQUEST}
};

const unordered_map<ResponsePayload::Code, StaticString> codeToName = {
	{ResponsePayload::Code::OK, StaticString("ok")},
	{ResponsePayload::Code::INTERNAL_ERROR, StaticString("internal error")},
	{ResponsePayload::Code::UNKNOWN_REQUEST, StaticString("unknown request")},
	{ResponsePayload::Code::INVALID_REQUEST, StaticString("invalid request")},
	{ResponsePayload::Code::UNSUPPORTED_REQUEST, StaticString("unsupported request")}
};

} // End anonymous namespace

ResponsePayload::ResponsePayload(int respTo, Code c, const std::string& msg) :
	respondingTo(respTo),
	code(c),
	message(msg)
{
	enforce<ArgumentException>(respondingTo > 0, "Responding to a negative ID does not make sense.", __FUNCTION__);
}

std::unique_ptr<ResponsePayload> ResponsePayload::fromJSON(const Json::Value& object)
{
	enforce<IOException>(object.isMember(respondingToKey), "Response payload is missing the response ID",
	                                     __FUNCTION__);

	enforce<IOException>(object.isMember(codeKey), "Response payload is missing the code", __FUNCTION__);
	enforce<IOException>(object.isMember(messageKey), "Response payload is missing the message", __FUNCTION__);

	const Value& respondingToValue = object[respondingToKey];
	const Value& codeValue = object[codeKey];
	const Value& messageValue = object[messageKey];

	enforce<IOException>(respondingToValue.isInt(), "The response ID is not an integer.", __FUNCTION__);
	enforce<IOException>(codeValue.isString(), "The response code is not a string.", __FUNCTION__);
	enforce<IOException>(messageValue.isString(), "The response message is not a string.", __FUNCTION__);

	const auto codeIt = nameToCode.find(codeValue.asString());
	enforce<IOException>(codeIt != end(nameToCode), "The response code is invalid.", __FUNCTION__);

	return std::unique_ptr<ResponsePayload>(
		new ResponsePayload(respondingToValue.asInt(), codeIt->second, messageValue.asString()));
}

Json::Value ResponsePayload::toJSON() const
{
	Value ret(objectValue);

	ret[respondingToKey] = respondingTo;
	ret[codeKey] = codeToName.at(code);
	ret[messageKey] = message;

	return ret;
}

bool ResponsePayload::operator==(const ResponsePayload& o) const
{
	return respondingTo == o.respondingTo
		&& code == o.code
		&& message == o.message;
}
