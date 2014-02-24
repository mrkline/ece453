#include "StatusResponsePayload.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

StatusResponsePayload::StatusResponsePayload(int respTo,
	                                         bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats) :
	ResponsePayload(respTo, RC_OK, ""), // If we're sending a full status response payload back, the request was ok.
	running(isRunning),
	timeRemaining(timeLeft),
	winningScore(winScore),
	players(move(playerStats))
{
	enforce<ArgumentException>(timeRemaining >= -1, "Invalid remaining time", __FUNCTION__);
	enforce<ArgumentException>(winningScore >= -1, "Invalid winning score", __FUNCTION__);
	enforce<ArgumentException>(players.size() > 0, "You must have at least one player.", __FUNCTION__);
}
