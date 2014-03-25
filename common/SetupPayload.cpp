#include "SetupPayload.hpp"

#include <algorithm>
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

const StaticString timeConditionValue("time");
const StaticString pointsConditionValue("points");

SetupPayload::DataMap parseGameData(const Value& data)
{
	SetupPayload::DataMap ret;

	for (ValueIterator it = begin(data); it != end(data); ++it) {
		const Value& val = *it;
		ENFORCE(IOException, val.isInt(), "The game data contained a value that was not an integer.");
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
	ENFORCE(ArgumentException, playerCount > 0, "You must have at least one player.");
	ENFORCE(ArgumentException, winConditions != 0, "There are no win conditions.");
	ENFORCE(ArgumentException, ((winConditions & WinCondition::WC_TIME) | (winConditions & WinCondition::WC_POINTS)) != 0,
	                           "Invalid win condition flags");
	ENFORCE(ArgumentException, endTime >= -1, "Invalid game duration");
	ENFORCE(ArgumentException, winningScore >= -1, "Invalid winning score");
}

std::unique_ptr<SetupPayload> SetupPayload::fromJSON(const Json::Value& object)
{
	ENFORCE(IOException, object.isMember(gameTypeKey), "Setup payload is missing the game type");
	ENFORCE(IOException, object.isMember(playerCountKey), "Setup payload is missing the player count");
	ENFORCE(IOException, object.isMember(winConditionsKey), "Setup payload is missing the win condition(s)");
	ENFORCE(IOException, object.isMember(endTimeKey), "Setup payload is missing the end time");
	ENFORCE(IOException, object.isMember(winningScoreKey), "Setup payload is missing the winning score");
	ENFORCE(IOException, object.isMember(gameDataKey), "Setup payload is missing additional game data");

	const Value& gameTypeValue = object[gameTypeKey];
	const Value& playerCountValue = object[playerCountKey];
	const Value& winConditionsValue = object[winConditionsKey];
	const Value& endTimeValue = object[endTimeKey];
	const Value& winningScoreValue = object[winningScoreKey];
	const Value& gameDataValue = object[gameDataKey];

	ENFORCE(IOException, gameTypeValue.isInt(), "The game type is not an integer.");
	ENFORCE(IOException, playerCountValue.isInt(), "The player count is not an integer.");
	ENFORCE(IOException, winConditionsValue.isArray(), "The win conditions are not an array.");
	ENFORCE(IOException, endTimeValue.isInt(), "The end time is not an integer.");
	ENFORCE(IOException, winningScoreValue.isInt(), "The winning score is not an integer.");
	ENFORCE(IOException, gameDataValue.isObject(), "The additional game data is not an object.");

	uint8_t winMask = 0;

	ENFORCE(IOException, winConditionsValue.size() <= 2, "There are more win conditions than we recognize.");

	/// \todo Fix up JSONCPP so that this works
	/*
	ENFORCE(IOException, all_of(begin(winConditionsValue), end(winConditionsValue), [](const Value& v) {
			if (!v.isString())
				return false;

			const auto str = v.asString();
			return str == pointsConditionValue.c_str() || str == timeConditionValue.c_str();
		}), "Some of the win conditions are not recongized");
	*/

	for (const Value& v : winConditionsValue) {
		ENFORCE(IOException, v.isString(), "A win condition is not a string.");
		const auto str = v.asString();
		bool points = str == pointsConditionValue.c_str();
		bool time = str == timeConditionValue.c_str();
		ENFORCE(IOException, points || time, "A win condition is not recognized as valid.");

		if (points)
			winMask |= WC_POINTS;
		if (time)
			winMask |= WC_TIME;
	}

	return std::unique_ptr<SetupPayload>(
		new SetupPayload((GameType)gameTypeValue.asInt(), playerCountValue.asInt(), winMask,
		                 endTimeValue.asInt(), winningScoreValue.asInt(), parseGameData(gameDataValue)));
}

Json::Value SetupPayload::toJSON() const
{
	Value ret(objectValue);

	ret[gameTypeKey] = (int)gameType;
	ret[playerCountKey] = playerCount;
	ret[endTimeKey] = endTime;
	ret[winningScoreKey] = winningScore;

	Value data(objectValue);

	for (const auto& pair : gameData)
		data[pair.first] = pair.second;

	ret[gameDataKey] = move(data);

	Value winArray(arrayValue);

	if ((winConditions & WC_TIME) == WC_TIME)
		winArray.append(timeConditionValue);

	if ((winConditions & WC_POINTS) == WC_POINTS)
		winArray.append(pointsConditionValue);

	ret[winConditionsKey] = move(winArray);

	return ret;
}


bool SetupPayload::operator==(const SetupPayload& o) const
{
	return gameType == o.gameType
		&& playerCount == o.playerCount
		&& winConditions == o.winConditions
		&& endTime == o.endTime
		&& winningScore == o.winningScore
		&& gameData == o.gameData;
}
