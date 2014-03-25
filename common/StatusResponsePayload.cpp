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
		ENFORCE(IOException, obj.isObject(), "An element of the player stats is not an object.");

		ENFORCE(IOException, obj.isMember(scoreKey), "An element of the player stats is missing the score.");
		ENFORCE(IOException, obj.isMember(hitsKey), "An element of the player stats is missing the hits value.");

		const Value& scoreValue = obj[scoreKey];
		const Value& hitsValue = obj[hitsKey];

		ENFORCE(IOException, scoreValue.isInt(), "An element of the player stats does not have an integer score.");
		ENFORCE(IOException, hitsValue.isInt(), "An element of the player stats does not have an integer hits value.");

		ret.emplace_back(scoreValue.asInt(), hitsValue.asInt());
	}

	return ret;
}

} // End anonymous namespace

StatusResponsePayload::StatusResponsePayload(int respTo, const std::string& message,
	                                         bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats) :
	// If we're sending a full status response payload back, the request was ok.
	ResponsePayload(respTo, ResponsePayload::Code::OK, message),
	running(isRunning),
	timeRemaining(timeLeft),
	winningScore(winScore),
	players(move(playerStats))
{
	ENFORCE(ArgumentException, timeRemaining >= -1, "Invalid remaining time");
	ENFORCE(ArgumentException, winningScore >= -1, "Invalid winning score");
	ENFORCE(ArgumentException, players.size() > 0, "You must have at least one player.");
}

std::unique_ptr<StatusResponsePayload> StatusResponsePayload::fromJSON(const Json::Value& object)
{
	const auto responseInfo = ResponsePayload::fromJSON(object); // Get the basic response info

	ENFORCE(IOException, responseInfo->code == ResponsePayload::Code::OK,
	                     "Full status response payloads must have an OK response code.");

	ENFORCE(IOException, object.isMember(runningKey), "Status response payload is missing the \"running\" flag");
	ENFORCE(IOException, object.isMember(timeRemainingKey), "Status response payload has no time remaining");
	ENFORCE(IOException, object.isMember(winningScoreKey), "Status response payload has no winning score");
	ENFORCE(IOException, object.isMember(playerStatsKey), "Status response payload has no player statistics");

	const Value& runningValue = object[runningKey];
	const Value& timeRemainingValue = object[timeRemainingKey];
	const Value& winningScoreValue = object[winningScoreKey];
	const Value& playerStatsValue = object[playerStatsKey];

	ENFORCE(IOException, runningValue.isBool(), "The \"running\" flag is not a boolean.");
	ENFORCE(IOException, timeRemainingValue.isInt(), "The time remaining value is not an integer.");
	ENFORCE(IOException, winningScoreValue.isInt(), "The winning score value is not an integer.");
	ENFORCE(IOException, playerStatsValue.isArray(), "The player stats value is not an array.");

	return std::unique_ptr<StatusResponsePayload>(
		new StatusResponsePayload(responseInfo->respondingTo, responseInfo->message,
		                          runningValue.asBool(), timeRemainingValue.asInt(), winningScoreValue.asInt(),
		                          parseStats(playerStatsValue)));
}

Json::Value StatusResponsePayload::toJSON() const
{
	Value ret = ResponsePayload::toJSON();

	ret[runningKey] = running;
	ret[timeRemainingKey] = timeRemaining;
	ret[winningScoreKey] = winningScore;

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


bool StatusResponsePayload::operator==(const StatusResponsePayload& o) const
{
	return ResponsePayload::operator==(o)
		&& running == o.running
		&& timeRemaining == o.timeRemaining
		&& winningScore == o.winningScore
		&& players == o.players;
}
