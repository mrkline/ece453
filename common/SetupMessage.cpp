#include "SetupMessage.hpp"

#include <algorithm>
#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString gameTypeKey("game type");
const StaticString playerCountKey("player count");
const StaticString endTimeKey("end time");
const StaticString winningScoreKey("winning score");
const StaticString gameDataKey("game data");

const StaticString timeConditionValue("time");
const StaticString pointsConditionValue("points");

SetupMessage::DataMap parseGameData(const Value& data)
{
	SetupMessage::DataMap ret;

	for (ValueIterator it = begin(data); it != end(data); ++it) {
		const Value& val = *it;
		ENFORCE(IOException, val.isInt(), "The game data contained a value that was not an integer.");
		ret[it.key().asString()] = val.asInt();
	}

	return ret;
}
#endif

} // End anonymous namespace

SetupMessage::SetupMessage(uint16_t id, GameType gType, int pCount, int time, int score, DataMap&& data) :
	Message(id),
	gameType(gType),
	playerCount(pCount),
	endTime(time),
	winningScore(score),
	gameData(move(data))
{
	ENFORCE(ArgumentException, playerCount > 0, "You must have at least one player.");
	ENFORCE(ArgumentException, endTime >= -1, "Invalid game duration");
	ENFORCE(ArgumentException, winningScore >= -1, "Invalid winning score");
}

#ifdef WITH_JSON
std::unique_ptr<SetupMessage> SetupMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(gameTypeKey), "Setup message is missing the game type");
	ENFORCE(IOException, object.isMember(playerCountKey), "Setup message is missing the player count");
	ENFORCE(IOException, object.isMember(endTimeKey), "Setup message is missing the end time");
	ENFORCE(IOException, object.isMember(winningScoreKey), "Setup message is missing the winning score");
	ENFORCE(IOException, object.isMember(gameDataKey), "Setup message is missing additional game data");

	const Value& gameTypeValue = object[gameTypeKey];
	const Value& playerCountValue = object[playerCountKey];
	const Value& endTimeValue = object[endTimeKey];
	const Value& winningScoreValue = object[winningScoreKey];
	const Value& gameDataValue = object[gameDataKey];

	ENFORCE(IOException, gameTypeValue.isInt(), "The game type is not an integer.");
	ENFORCE(IOException, playerCountValue.isInt(), "The player count is not an integer.");
	ENFORCE(IOException, endTimeValue.isInt(), "The end time is not an integer.");
	ENFORCE(IOException, winningScoreValue.isInt(), "The winning score is not an integer.");
	ENFORCE(IOException, gameDataValue.isObject(), "The additional game data is not an object.");

	return std::unique_ptr<SetupMessage>(
		new SetupMessage(msg->id,
		                 (GameType)gameTypeValue.asInt(), playerCountValue.asInt(),
		                 endTimeValue.asInt(), winningScoreValue.asInt(), parseGameData(gameDataValue)));
}

Json::Value SetupMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[gameTypeKey] = (int)gameType;
	ret[playerCountKey] = playerCount;
	ret[endTimeKey] = endTime;
	ret[winningScoreKey] = winningScore;

	Value data(objectValue);

	for (const auto& pair : gameData)
		data[pair.first] = pair.second;

	ret[gameDataKey] = move(data);

	return ret;
}
#endif


bool SetupMessage::operator==(const Message& o) const
{
	if (!Message::operator==(o))
		return false;

	auto sm = dynamic_cast<const SetupMessage*>(&o);

	if (sm == nullptr)
		return false;

	return gameType == sm->gameType
		&& playerCount == sm->playerCount
		&& endTime == sm->endTime
		&& winningScore == sm->winningScore
		&& gameData == sm->gameData;
}
