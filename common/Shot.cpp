#include "Shot.hpp"

#include <limits>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

const StaticString playerKey("player");
const StaticString targetKey("target ID");
const StaticString timeKey("time");
const StaticString movementKey("movement");

} // end anonymous namespace

Shot::Shot(int8_t p, int8_t tar, int time) :
	player(p),
	target(tar),
	time(time)
{
	ENFORCE(ArgumentException, time >= 0, "A shot cannot take place before the game starts");
}

Json::Value Shot::toJSON() const
{
	Value shotValue(objectValue);

	shotValue[playerKey] = player;
	shotValue[targetKey] = target;
	shotValue[timeKey] = time;

	return shotValue;
}

Shot Shot::fromJSON(const Json::Value& value)
{
	ENFORCE(IOException, value.isObject(), "A shot is not an object.");

	ENFORCE(IOException, value.isMember(playerKey), "A shot is missing its player ID.");
	ENFORCE(IOException, value.isMember(targetKey), "A shot is missing its target ID.");
	ENFORCE(IOException, value.isMember(timeKey), "A shot is missing its timestamp.");

	const Value& playerValue = value[playerKey];
	const Value& targetValue = value[targetKey];
	const Value& timeValue = value[timeKey];

	ENFORCE(IOException, playerValue.isInt(), "A shot's player ID is not an integer.");
	ENFORCE(IOException, targetValue.isInt(), "A shot's target ID is not an integer.");
	ENFORCE(IOException, timeValue.isInt(), "A shot's timestamp is not an integer.");

	const int p = playerValue.asInt();
	const int tar = targetValue.asInt();
	const int time = timeValue.asInt();

	ENFORCE(IOException, p >= -128 && p < 128, "A shot's player ID is not representable by a byte.");
	ENFORCE(IOException, tar >= -128 && tar < 128, "A shot's target ID is not representable by a byte.");

	return Shot((char)p, (char)tar, time);
}

bool Shot::operator==(const Shot& o) const
{
	return player == o.player
		&& target == o.target
		&& time == o.time;
}

ShotWithMovement::ShotWithMovement(char p, char tar, int time, std::vector<Vector3>&& m) :
	Shot(p, tar, time),
	movement(move(m))
{ }

ShotWithMovement::ShotWithMovement(const Shot& s, Movement&& m) :
	Shot(s),
	movement(move(m))
{ }

ShotWithMovement::ShotWithMovement(ShotWithMovement&& o) :
	Shot(o),
	movement(move(o.movement))
{ }

Json::Value ShotWithMovement::toJSON() const
{
	Value shotValue = Shot::toJSON();
	shotValue[movementKey] = movementToJSON(movement);
	return shotValue;
}

ShotWithMovement ShotWithMovement::fromJSON(const Json::Value& value)
{
	Shot plain = Shot::fromJSON(value);

	ENFORCE(IOException, value.isMember(movementKey), "A shot is missing its movement history.");
	const Value& movementValue = value[movementKey];
	ENFORCE(IOException, movementValue.isArray(), "A shot's movement history is not an array.");

	return ShotWithMovement(plain.player, plain.target, plain.time, movementFromJSON(movementValue));
}

bool ShotWithMovement::operator==(const Shot& o) const
{
	if (!Shot::operator==(o))
		return false;

	const auto swm = dynamic_cast<const ShotWithMovement*>(&o);
	if (swm == nullptr)
		return false;

	return movement == swm->movement;
}

Movement movementFromJSON(const Json::Value& moves)
{
	Movement ret;

	for (const Value& move : moves) {

		ENFORCE(IOException, move.isArray() && move.size() == 3, "A movement is not a three-element array");

		// Apparently the compiler can't figure out if 0, 1, and 2 are chars or ints. Help it along.
		const Value& xVal = move[0U];
		const Value& yVal = move[1U];
		const Value& zVal = move[2U];

		ENFORCE(IOException, xVal.isDouble(), "A movement value is not a decimal type.");
		ENFORCE(IOException, yVal.isDouble(), "A movement value is not a decimal type.");
		ENFORCE(IOException, zVal.isDouble(), "A movement value is not a decimal type.");

		ret.emplace_back(xVal.asDouble(), yVal.asDouble(), zVal.asDouble());
	}

	return ret;
}

Json::Value movementToJSON(const Movement moves)
{
	Value movementValue(arrayValue);

	for (const auto& vec : moves) {
		Value vecArray(arrayValue);
		vecArray.append(vec.x);
		vecArray.append(vec.y);
		vecArray.append(vec.z);

		movementValue.append(move(vecArray));
	}

	return movementValue;
}
