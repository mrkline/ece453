#include "SetupPayload.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

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
