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

std::vector<Shot> parseShots(const Value& shots)
{
	std::vector<Shot> ret;

	for (const Value& shot : shots) {

		ENFORCE(IOException, shot.isObject(), "An item in the shots array of a player's results is not an object.");

		ENFORCE(IOException, shot.isMember(hitKey), "A shot is missing its hit indicator.");
		ENFORCE(IOException, shot.isMember(timeKey), "A shot is missing its timestamp.");
		ENFORCE(IOException, shot.isMember(movementKey), "A shot is missing its movement history.");

		const Value& hitValue = shot[hitKey];
		const Value& timeValue = shot[timeKey];
		const Value& movementValue = shot[movementKey];

		ENFORCE(IOException, hitValue.isBool(), "A shot's hit indicator is not a boolean.");
		ENFORCE(IOException, timeValue.isInt(), "A shot's timestamp is not an integer.");
		ENFORCE(IOException, movementValue.isArray(), "A shot's movement history is not an array.");

		ret.emplace_back(hitValue.asBool(), timeValue.asInt(), parseMovement(movementValue));
	}

	return ret;
}

PlayerStats parseStats(const Value& stat)
{
	ENFORCE(IOException, stat.isMember(scoreKey), "A player results set is missing its score.");
	ENFORCE(IOException, stat.isMember(hitsKey), "A player results set is missing its hit count.");
	ENFORCE(IOException, stat.isMember(shotsKey), "A player results set is missing its shots list.");

	const Value& scoreValue = stat[scoreKey];
	const Value& hitsValue = stat[hitsKey];
	const Value& shotsValue = stat[shotsKey];

	ENFORCE(IOException, scoreValue.isInt(), "A player's score is not an integer.");
	ENFORCE(IOException, hitsValue.isInt(), "A player's hit count is not an integer.");
	ENFORCE(IOException, shotsValue.isArray(), "A player's shot list is not an array.");

	return PlayerStats(scoreValue.asInt(), hitsValue.asInt(), parseShots(shotsValue));
}

} // end anonymous namespace

ResultsResponsePayload::ResultsResponsePayload(int respTo, const std::string& message, StatsList&& playerStats) :
	ResponsePayload(respTo, ResponsePayload::Code::OK, message), // If we're sending a results response payload back, the request was ok.
	stats(move(playerStats))
{
	for (const auto& stat : stats) {
		ENFORCE(ArgumentException, stat.hits >= 0, "A player cannot have negative hits.");

		for (const auto& shot : stat.shots)
			ENFORCE(ArgumentException, shot.time >= 0, "A shot cannot happen before the game starts.");
	}
}

std::unique_ptr<ResultsResponsePayload> ResultsResponsePayload::fromJSON(const Json::Value& object)
{
	const auto responseInfo = ResponsePayload::fromJSON(object); // Get the basic response info

	ENFORCE(IOException, responseInfo->code == ResponsePayload::Code::OK,
	        "Full results response payloads must have an OK response code.");

	ENFORCE(IOException, object.isMember(playerStatsKey), "Results response payload is missing player stats");

	const Value& playerStatsValue = object[playerStatsKey];

	ENFORCE(IOException, playerStatsValue.isArray(), "The player stats value is not an array.");

	StatsList playerStats;

	for (const Value& stat : playerStatsValue) {
		playerStats.emplace_back(parseStats(stat));
	}

	return std::unique_ptr<ResultsResponsePayload>(
		new ResultsResponsePayload(responseInfo->respondingTo, responseInfo->message, move(playerStats)));
}

Json::Value ResultsResponsePayload::toJSON() const
{
	Value ret = ResponsePayload::toJSON();

	Value statsList(arrayValue);

	for (const auto& stat : stats) {
		Value statValue(objectValue);

		statValue[scoreKey] = stat.score;
		statValue[hitsKey] = stat.hits;

		Value shots(arrayValue);

		for (const auto& shot : stat.shots) {
			Value shotValue(objectValue);

			shotValue[hitKey] = shot.hit;
			shotValue[timeKey] = shot.time;

			Value movementValue(arrayValue);

			for (const auto& vec : shot.movement) {
				Value vecArray(arrayValue);
				vecArray.append(vec.x);
				vecArray.append(vec.y);
				vecArray.append(vec.z);

				movementValue.append(move(vecArray));
			}

			shotValue[movementKey] = move(movementValue);

			shots.append(move(shotValue));
		}

		statValue[shotsKey] = move(shots);

		statsList.append(move(statValue));
	}

	ret[playerStatsKey] = move(statsList);

	return ret;
}
