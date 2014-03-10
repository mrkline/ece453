#include "ResponsePayload.hpp"

#include "Exceptions.hpp"

using namespace Json;
using namespace Exceptions;

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString respondingToKey("responding to");
const StaticString codeKey("code");
const StaticString messageKey("message");

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
	enforce<IOException>(codeValue.isInt(), "The response code is not an integer.", __FUNCTION__);
	enforce<IOException>(messageValue.isString(), "The response message is not a string.", __FUNCTION__);

	/* FIXME
	const int rawCode = codeValue.asInt();

	enforce<IOException>(rawCode >= ResponsePayload::Code::OK && rawCode <= ResponsePayload::Code::UNSUPPORTED_REQUEST, "The response code is invalid.",
	                     __FUNCTION__);

	return std::unique_ptr<ResponsePayload>(
		new ResponsePayload(respondingToValue.asInt(), (Code)rawCode, messageValue.asString()));
	*/
}

Json::Value ResponsePayload::toJSON() const
{
	Value ret(objectValue);

	ret[respondingToKey] = respondingTo;
	ret[codeKey] = (int)code;
	ret[messageKey] = message;

	return ret;
}

bool ResponsePayload::operator==(const ResponsePayload& o) const
{
	return respondingTo == o.respondingTo
		&& code == o.code
		&& message == o.message;
}
