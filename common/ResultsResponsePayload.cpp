#include "ResultsResponsePayload.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString playerStatsKey("player stats");
const StaticString scoreKey("score");
const StaticString hitsKey("hits");
const StaticString shotsKey("shots");
const StaticString hitKey("hit");
const StaticString timeKey("time");
const StaticString movementKey("movement");

// For laziness
typedef ResultsResponsePayload::Shot Shot;
typedef ResultsResponsePayload::PlayerStats PlayerStats;

std::vector<Vector3> parseMovement(const Value& moves)
{
	std::vector<Vector3> ret;

	for (const Value& move : moves) {

		enforce<IOException>(move.isArray() && move.size() == 3, "A movement is not a three-element array",
		                     __FUNCTION__);

		// Apparently the compiler can't figure out if 0, 1, and 2 are chars or ints. Help it along.
		const Value& xVal = move[0U];
		const Value& yVal = move[1U];
		const Value& zVal = move[2U];

		enforce<IOException>(xVal.isDouble(), "A movement value is not a decimal type.", __FUNCTION__);
		enforce<IOException>(yVal.isDouble(), "A movement value is not a decimal type.", __FUNCTION__);
		enforce<IOException>(zVal.isDouble(), "A movement value is not a decimal type.", __FUNCTION__);

		ret.emplace_back(xVal.asDouble(), yVal.asDouble(), zVal.asDouble());
	}

	return ret;
}

std::vector<Shot> parseShots(const Value& shots)
{
	std::vector<Shot> ret;

	for (const Value& shot : shots) {

		enforce<IOException>(shot.isObject(), "An item in the shots array of a player's results is not an object.",
		                     __FUNCTION__);

		enforce<IOException>(shot.isMember(hitKey), "A shot is missing its hit indicator.", __FUNCTION__);
		enforce<IOException>(shot.isMember(timeKey), "A shot is missing its timestamp.", __FUNCTION__);
		enforce<IOException>(shot.isMember(movementKey), "A shot is missing its movement history.", __FUNCTION__);

		const Value& hitValue = shot[hitKey];
		const Value& timeValue = shot[timeKey];
		const Value& movementValue = shot[movementKey];

		enforce<IOException>(hitValue.isBool(), "A shot's hit indicator is not a boolean.", __FUNCTION__);
		enforce<IOException>(timeValue.isInt(), "A shot's timestamp is not an integer.", __FUNCTION__);
		enforce<IOException>(movementValue.isArray(), "A shot's movement history is not an array.", __FUNCTION__);

		ret.emplace_back(hitValue.asBool(), timeValue.asInt(), parseMovement(movementValue));
	}

	return ret;
}

PlayerStats parseStats(const Value& stat)
{
	enforce<IOException>(stat.isMember(scoreKey), "A player results set is missing its score.", __FUNCTION__);
	enforce<IOException>(stat.isMember(hitsKey), "A player results set is missing its hit count.", __FUNCTION__);
	enforce<IOException>(stat.isMember(shotsKey), "A player results set is missing its shots list.", __FUNCTION__);

	const Value& scoreValue = stat[scoreKey];
	const Value& hitsValue = stat[hitsKey];
	const Value& shotsValue = stat[shotsKey];

	enforce<IOException>(scoreValue.isInt(), "A player's score is not an integer.", __FUNCTION__);
	enforce<IOException>(hitsValue.isInt(), "A player's hit count is not an integer.", __FUNCTION__);
	enforce<IOException>(shotsValue.isArray(), "A player's shot list is not an array.", __FUNCTION__);

	return PlayerStats(scoreValue.asInt(), hitsValue.asInt(), parseShots(shotsValue));
}

} // end anonymous namespace

ResultsResponsePayload::ResultsResponsePayload(int respTo, const std::string& message, StatsList&& playerStats) :
	ResponsePayload(respTo, RC_OK, message), // If we're sending a results response payload back, the request was ok.
	stats(move(playerStats))
{
	for (const auto& stat : stats) {
		enforce<ArgumentException>(stat.hits >= 0, "A player cannot have negative hits.", __FUNCTION__);

		for (const auto& shot : stat.shots)
			enforce<ArgumentException>(shot.time >= 0, "A shot cannot happen before the game starts.", __FUNCTION__);
	}
}

std::unique_ptr<ResultsResponsePayload> ResultsResponsePayload::fromJSON(const Json::Value& object)
{
	const auto responseInfo = ResponsePayload::fromJSON(object); // Get the basic response info

	enforce<IOException>(responseInfo->code == RC_OK, "Full results response payloads must have an OK response code.",
	                     __FUNCTION__);

	enforce<IOException>(object.isMember(playerStatsKey), "Results response payload is missing player stats",
	                                     __FUNCTION__);

	const Value& playerStatsValue = object[playerStatsKey];

	enforce<IOException>(playerStatsValue.isArray(), "The player stats value is not an array.", __FUNCTION__);

	StatsList playerStats;

	for (const Value& stat : playerStatsValue) {
		playerStats.emplace_back(parseStats(stat));
	}

	return std::unique_ptr<ResultsResponsePayload>(
		new ResultsResponsePayload(responseInfo->respondingTo, responseInfo->message, move(playerStats)));
}
