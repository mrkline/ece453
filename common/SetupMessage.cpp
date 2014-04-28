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
const StaticString gameLengthKey("game length");
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

SetupMessage::SetupMessage(message_id_t id, GameType gType, board_id_t pCount,
                           duration_t time, score_t score, DataMap&& data) :
	Message(id),
	gameType(gType),
	playerCount(pCount),
	gameLength(time),
	winningScore(score),
	gameData(move(data))
{
	ENFORCE(ArgumentException, playerCount > 0, "You must have at least one player.");
	ENFORCE(ArgumentException, gameLength >= -1, "Invalid game duration");
	ENFORCE(ArgumentException, winningScore >= -1, "Invalid winning score");
}

#ifdef WITH_JSON
std::unique_ptr<SetupMessage> SetupMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(gameTypeKey), "Setup message is missing the game type");
	ENFORCE(IOException, object.isMember(playerCountKey), "Setup message is missing the player count");
	ENFORCE(IOException, object.isMember(gameLengthKey), "Setup message is missing the end time");
	ENFORCE(IOException, object.isMember(winningScoreKey), "Setup message is missing the winning score");
	ENFORCE(IOException, object.isMember(gameDataKey), "Setup message is missing additional game data");

	const Value& gameTypeValue = object[gameTypeKey];
	const Value& playerCountValue = object[playerCountKey];
	const Value& gameLengthValue = object[gameLengthKey];
	const Value& winningScoreValue = object[winningScoreKey];
	const Value& gameDataValue = object[gameDataKey];

	ENFORCE(IOException, gameTypeValue.isInt(), "The game type is not an integer.");
	ENFORCE(IOException, playerCountValue.isInt(), "The player count is not an integer.");
	ENFORCE(IOException, gameLengthValue.isInt(), "The end time is not an integer.");
	ENFORCE(IOException, winningScoreValue.isInt(), "The winning score is not an integer.");
	ENFORCE(IOException, gameDataValue.isObject(), "The additional game data is not an object.");

	return std::unique_ptr<SetupMessage>(
		new SetupMessage(msg->id,
		                 (GameType)gameTypeValue.asInt(), (board_id_t)playerCountValue.asInt(),
		                 (duration_t)gameLengthValue.asInt(), (score_t)winningScoreValue.asInt(),
		                 parseGameData(gameDataValue)));
}

Json::Value SetupMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[gameTypeKey] = (int)gameType;
	ret[playerCountKey] = playerCount;
	ret[gameLengthKey] = gameLength;
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
		&& gameLength == sm->gameLength
		&& winningScore == sm->winningScore
		&& gameData == sm->gameData;
}
