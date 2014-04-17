#include "ShotMessage.hpp"

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

const StaticString shotKey("shot");

} // end anonymous namespace

ShotMessage::ShotMessage(int id, const Shot& s) :
	Message(id),
	shot(s)
{
}

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

bool ShotMessage::operator==(const Message& o) const
{
	if(!Message::operator==(o))
		return false;

	auto sm = dynamic_cast<const ShotMessage*>(&o);

	if (sm == nullptr)
		return false;

	return shot == sm->shot;
}
