#include "ResponseMessage.hpp"

#include <unordered_map>

#include "BinaryMessage.hpp"
#include "Exceptions.hpp"

using namespace Exceptions;
using namespace std;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace std
{
	// Hasher for ResponseMessage::Code
	template<>
	struct hash<ResponseMessage::Code> {
		size_t operator()(ResponseMessage::Code t) const { return std::hash<int>()((int)t); }
	};
}

namespace {

#ifdef WITH_JSON
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
#endif

} // End anonymous namespace

ResponseMessage::ResponseMessage(message_id_t idNum, message_id_t respTo, Code c, const std::string& msg) :
	Message(idNum),
	respondingTo(respTo),
	code(c),
	message(msg)
{
}

#ifdef WITH_JSON
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

	const int respondingToRaw = respondingToValue.asInt();

	ENFORCE(IOException, respondingToRaw >= 0, "The response ID must be positive");

	return std::unique_ptr<ResponseMessage>(
		new ResponseMessage(msg->id, (message_id_t)respondingToRaw, codeIt->second, messageValue.asString()));
}

Json::Value ResponseMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[respondingToKey] = respondingTo;
	ret[codeKey] = codeToName.at(code);
	ret[messageKey] = message;

	return ret;
}
#endif

std::unique_ptr<ResponseMessage> ResponseMessage::fromBinary(uint8_t* buf, size_t len)
{
	auto msg = Message::fromBinary(buf, len);

	using namespace BinaryMessage;

	size_t pl;
	const uint8_t* bl = getPayload(buf, pl);
	// Make sure the payload contains for our expected data (respondingTo ID and code)
	ENFORCE(IOException, pl >= sizeof(message_id_t) + 1, "The provided message is too small.");

	message_id_t resp = extractUInt16(bl);
	bl += 2;
	Code c = (Code)*bl;

	// Binary response messages contain no strings. Not worth the trouble or bandwidth.
	return std::unique_ptr<ResponseMessage>(
		new ResponseMessage(msg->id, resp, c, ""));
}

std::vector<uint8_t> ResponseMessage::getBinaryPayload() const
{
	vector<uint8_t> ret;
	BinaryMessage::appendInt(ret, respondingTo);
	ret.emplace_back((uint8_t)code);
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
