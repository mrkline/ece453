#include "StatusResponsePayload.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString runningKey("running");
const StaticString timeRemainingKey("time remaining");
const StaticString winningScoreKey("winning score");
const StaticString playerStatsKey("player stats");
const StaticString scoreKey("score");
const StaticString hitsKey("hits");

StatusResponsePayload::PlayerList parseStats(const Value& stats)
{
	StatusResponsePayload::PlayerList ret;

	for (const Value& obj : stats) {
		enforce<IOException>(obj.isObject(), "An element of the player stats is not an object.", __FUNCTION__);

		enforce<IOException>(obj.isMember(scoreKey), "An element of the player stats is missing the score.",
		                     __FUNCTION__);
		enforce<IOException>(obj.isMember(hitsKey), "An element of the player stats is missing the hits value.",
		                     __FUNCTION__);

		const Value& scoreValue = obj[scoreKey];
		const Value& hitsValue = obj[hitsKey];

		enforce<IOException>(scoreValue.isInt(), "An element of the player stats does not have an integer score.",
		                     __FUNCTION__);
		enforce<IOException>(hitsValue.isInt(), "An element of the player stats does not have an integer hits value.",
		                    __FUNCTION__);

		StatusResponsePayload::PlayerStats stat;
		stat.score = scoreValue.asInt();
		stat.hits = hitsValue.asInt();

		ret.emplace_back(stat);
	}

	return ret;
}

} // End anonymous namespace

StatusResponsePayload::StatusResponsePayload(int respTo, const std::string& message,
	                                         bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats) :
	ResponsePayload(respTo, RC_OK, message), // If we're sending a full status response payload back, the request was ok.
	running(isRunning),
	timeRemaining(timeLeft),
	winningScore(winScore),
	players(move(playerStats))
{
	enforce<ArgumentException>(timeRemaining >= -1, "Invalid remaining time", __FUNCTION__);
	enforce<ArgumentException>(winningScore >= -1, "Invalid winning score", __FUNCTION__);
	enforce<ArgumentException>(players.size() > 0, "You must have at least one player.", __FUNCTION__);
}

std::unique_ptr<StatusResponsePayload> StatusResponsePayload::fromJSON(const Json::Value& object)
{
	const auto responseInfo = ResponsePayload::fromJSON(object); // Get the basic response info

	enforce<IOException>(responseInfo->code == RC_OK, "Full status response payloads must have an OK response code.",
	                     __FUNCTION__);

	enforce<IOException>(object.isMember(runningKey), "Status response payload is missing the \"running\" flag",
	                     __FUNCTION__);
	enforce<IOException>(object.isMember(timeRemainingKey), "Status response payload has no time remaining",
	                                     __FUNCTION__);
	enforce<IOException>(object.isMember(winningScoreKey), "Status response payload has no winning score",
	                                     __FUNCTION__);
	enforce<IOException>(object.isMember(playerStatsKey), "Status response payload has no player statistics",
	                     __FUNCTION__);

	const Value& runningValue = object[runningKey];
	const Value& timeRemainingValue = object[timeRemainingKey];
	const Value& winningScoreValue = object[winningScoreKey];
	const Value& playerStatsValue = object[playerStatsKey];

	enforce<IOException>(runningValue.isBool(), "The \"running\" flag is not a boolean.", __FUNCTION__);
	enforce<IOException>(timeRemainingValue.isInt(), "The time remaining value is not an integer.", __FUNCTION__);
	enforce<IOException>(winningScoreValue.isInt(), "The winning score value is not an integer.", __FUNCTION__);
	enforce<IOException>(playerStatsValue.isArray(), "The player stats value is not an array.", __FUNCTION__);

	return std::unique_ptr<StatusResponsePayload>(
		new StatusResponsePayload(responseInfo->respondingTo, responseInfo->message,
		                          runningValue.asBool(), timeRemainingValue.asInt(), winningScoreValue.asInt(),
		                          parseStats(playerStatsValue)));
}

Json::Value StatusResponsePayload::toJSON() const
{
	Value ret(objectValue);

	ret[runningKey] = running;
	ret[timeRemainingKey] = timeRemaining;
	ret[winningScore] = winningScore;

	Value playerList(arrayValue);

	for (const auto& player : players) {
		Value stat(objectValue);
		stat[scoreKey] = player.score;
		stat[hitsKey] = player.hits;
		playerList.append(move(stat));
	}

	ret[playerStatsKey] = move(playerList);

	return ret;
}
