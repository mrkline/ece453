#include "Shot.hpp"

#include <limits>

#include "BinaryMessage.hpp"
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

std::vector<uint8_t> Shot::toBinary() const
{
	using namespace BinaryMessage;

	vector<uint8_t> ret;

	ret.emplace_back(player);
	ret.emplace_back(target);
	appendInt(ret, time);

	return ret;
}

Shot Shot::fromBinary(const uint8_t* buf, size_t len)
{
	// Quick checks that I didn't forget to update this function if sizes change
	static_assert(sizeof(board_id_t) == sizeof(int8_t), "Someone changed the board ID length");
	static_assert(sizeof(timestamp_t) == sizeof(int32_t), "Someone changed the timestamp length");

	ENFORCE(IOException, len >= sizeof(player) + sizeof(target) + sizeof(time),
	        "This buffer does not have room for a shot.");

	board_id_t player = (board_id_t)buf[0];
	board_id_t target = (board_id_t)buf[1];
	timestamp_t time = BinaryMessage::extractInt32(&buf[2]);

	return Shot(player, target, time);
}

bool Shot::operator==(const Shot& o) const
{
	return player == o.player
		&& target == o.target
		&& time == o.time;
}
