#include "MovementMessage.hpp"

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

const StaticString movementKey("movement");

} // end anonymous namespace

MovementMessage::MovementMessage(int id, Movement&& m) :
	Message(id),
	movement(move(m))
{
}

std::unique_ptr<MovementMessage> MovementMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);
	
	ENFORCE(IOException, object.isMember(movementKey), "Movement message is missing its movement");

	const Value& movementValue = object[movementKey];

	ENFORCE(IOException, movementValue.isArray(), "The movement is not an array.");

	Movement m = movementFromJSON(movementValue);

	return std::unique_ptr<MovementMessage>(
		new MovementMessage(msg->id, move(m)));
}

Json::Value MovementMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[movementKey] = movementToJSON(movement);

	return ret;
}

bool MovementMessage::operator==(const Message& o) const
{
	if(!Message::operator==(o))
		return false;

	auto mm = dynamic_cast<const MovementMessage*>(&o);

	if (mm == nullptr)
		return false;

	return movement == mm->movement;
}
