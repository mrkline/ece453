#include "MovementMessage.hpp"

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
const StaticString movementKey("shot with movement");
#endif

} // end anonymous namespace

MovementMessage::MovementMessage(uint16_t id, ShotWithMovement&& m) :
	Message(id),
	movement(move(m))
{
}

#ifdef WITH_JSON
std::unique_ptr<MovementMessage> MovementMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(movementKey), "Movement message is missing its shot with movement");

	const Value& movementValue = object[movementKey];

	ENFORCE(IOException, movementValue.isObject(), "The shot with movement is not an object.");

	ShotWithMovement swm = ShotWithMovement::fromJSON(movementValue);

	return std::unique_ptr<MovementMessage>(
		new MovementMessage(msg->id, move(swm)));
}

Json::Value MovementMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[movementKey] = movement.toJSON();

	return ret;
}
#endif

bool MovementMessage::operator==(const Message& o) const
{
	if(!Message::operator==(o))
		return false;

	auto mm = dynamic_cast<const MovementMessage*>(&o);

	if (mm == nullptr)
		return false;

	return movement == mm->movement;
}
