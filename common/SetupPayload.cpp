#include "SetupPayload.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString gameTypeKey("game type");
const StaticString playerCountKey("player count");
const StaticString winConditionsKey("win conditions");
const StaticString endTimeKey("end time");
const StaticString winningScoreKey("winning score");
const StaticString gameDataKey("game data");

SetupPayload::DataMap parseGameData(const Value& data)
{
	SetupPayload::DataMap ret;

	for (ValueIterator it = begin(data); it != end(data); ++it) {
		const Value& val = *it;
		enforce<IOException>(val.isInt(), "The game data contained a value that was not an integer.", __FUNCTION__);
		ret[it.key().asString()] = val.asInt();
	}

	return ret;
}

} // End anonymous namespace

SetupPayload::SetupPayload(GameType gType, int pCount, uint8_t winConds, int time, int score, DataMap&& data) :
	gameType(gType),
	playerCount(pCount),
	winConditions(winConds),
	endTime(time),
	winningScore(score),
	gameData(move(data))
{
	enforce<ArgumentException>(playerCount > 0, "You must have at least one player.", __FUNCTION__);
	enforce<ArgumentException>(winConditions == WC_TIME || winConditions == WC_POINTS
	                           || winConditions == (WC_TIME | WC_POINTS),
	                           "Invalid win condition flags", __FUNCTION__);
	enforce<ArgumentException>(endTime >= -1, "Invalid game duration", __FUNCTION__);
	enforce<ArgumentException>(winningScore >= -1, "Invalid winning score", __FUNCTION__);
}

std::unique_ptr<SetupPayload> SetupPayload::fromJson(const Json::Value& object)
{
	enforce<IOException>(object.isMember(gameTypeKey), "Setup payload is missing the game type", __FUNCTION__);
	enforce<IOException>(object.isMember(playerCountKey), "Setup payload is missing the player count", __FUNCTION__);
	enforce<IOException>(object.isMember(winConditionsKey), "Setup payload is missing the win condition(s)",
	                     __FUNCTION__);
	enforce<IOException>(object.isMember(endTimeKey), "Setup payload is missing the end time", __FUNCTION__);
	enforce<IOException>(object.isMember(winningScoreKey), "Setup payload is missing the winning score", __FUNCTION__);
	enforce<IOException>(object.isMember(gameDataKey), "Setup payload is missing additional game data", __FUNCTION__);

	const Value& gameTypeValue = object[gameTypeKey];
	const Value& playerCountValue = object[playerCountKey];
	const Value& winConditionsValue = object[winConditionsKey];
	const Value& endTimeValue = object[endTimeKey];
	const Value& winningScoreValue = object[winningScoreKey];
	const Value& gameDataValue = object[gameDataKey];

	enforce<IOException>(gameTypeValue.isInt(), "The game type is not an integer.", __FUNCTION__);
	enforce<IOException>(playerCountValue.isInt(), "The player count is not an integer.", __FUNCTION__);
	enforce<IOException>(winConditionsValue.isInt(), "The win conditions are not an integer/bitmask.", __FUNCTION__);
	enforce<IOException>(endTimeValue.isInt(), "The end time is not an integer.", __FUNCTION__);
	enforce<IOException>(winningScoreValue.isInt(), "The winning score is not an integer.", __FUNCTION__);
	enforce<IOException>(gameDataValue.isObject(), "The additional game data is not an object.", __FUNCTION__);

	const int rawWinConditions = winConditionsValue.asInt();

	enforce<IOException>(rawWinConditions >= 0 && rawWinConditions < 0xff,
	                     "The win conditions are not a valid bitmask.",
	                     __FUNCTION__);

	return std::unique_ptr<SetupPayload>(
		new SetupPayload((GameType)gameTypeValue.asInt(), playerCountValue.asInt(), (uint8_t)rawWinConditions,
		                 endTimeValue.asInt(), winningScoreValue.asInt(), parseGameData(gameDataValue)));
}
