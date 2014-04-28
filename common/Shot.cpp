#include "Shot.hpp"

#include <limits>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
const StaticString playerKey("player");
const StaticString targetKey("target ID");
const StaticString timeKey("time");
const StaticString movementKey("movement");
#endif

} // end anonymous namespace

Shot::Shot(board_id_t p, board_id_t tar, timestamp_t time) :
	player(p),
	target(tar),
	time(time)
{
	ENFORCE(ArgumentException, time >= 0, "A shot cannot take place before the game starts");
}

#ifdef WITH_JSON
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

	return Shot((board_id_t)p, (board_id_t)tar, time);
}
#endif

bool Shot::operator==(const Shot& o) const
{
	return player == o.player
		&& target == o.target
		&& time == o.time;
}
