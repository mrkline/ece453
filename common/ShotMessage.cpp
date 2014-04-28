#include "ShotMessage.hpp"

#include <cassert>

#include "BinaryMessage.hpp"
#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
const StaticString shotKey("shot");
#endif

} // end anonymous namespace

ShotMessage::ShotMessage(message_id_t id, const Shot& s) :
	Message(id),
	shot(s)
{
}

#ifdef WITH_JSON
std::unique_ptr<ShotMessage> ShotMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(shotKey), "Shot message is missing its shot");

	const Value& shotValue = object[shotKey];

	ENFORCE(IOException, shotValue.isObject(), "The shot is not an object.");

	Shot s = Shot::fromJSON(shotValue);

	return std::unique_ptr<ShotMessage>(
		new ShotMessage(msg->id, s));
}

Json::Value ShotMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[shotKey] = shot.toJSON();

	return ret;
}
#endif

std::unique_ptr<ShotMessage> ShotMessage::fromBinary(uint8_t* buf, size_t len)
{
	auto msg = Message::fromBinary(buf, len);

	auto load = BinaryMessage::getPayload(buf);
	return unique_ptr<ShotMessage>(
		new ShotMessage(msg->id, Shot::fromBinary(load.first, load.second)));
}

std::vector<uint8_t> ShotMessage::getBinaryPayload() const
{
	assert(Message::getBinaryPayload().size() == 0);
	return shot.toBinary();
}

bool ShotMessage::operator==(const Message& o) const
{
	if(!Message::operator==(o))
		return false;

	auto sm = dynamic_cast<const ShotMessage*>(&o);

	if (sm == nullptr)
		return false;

	return shot == sm->shot;
}
