#include "PopUpStateMachine.hpp"

#include <cassert>

#include "ShotMessage.hpp"
#include "TargetControlMessage.hpp"

using namespace std;
using namespace std::chrono;

PopUpStateMachine::PopUpStateMachine(board_id_t numTargets, board_id_t numPlayers,
                                     const std::chrono::seconds& gameDuration, score_t scoreToWin) :
	GameStateMachine(numTargets, numPlayers, gameDuration, scoreToWin),
	rng(std::random_device()()),
	delayDistribution(3, 6),
	targetDistribution(0, (board_id_t)(numTargets - 1)),
	state(PopUpState::STARTUP),
	transitionTime(),
	targetUp(),
	whichTarget(-1)
{
}

std::unique_ptr<ResponseMessage> PopUpStateMachine::onShot(uint16_t responseID, const ShotMessage& shot)
{
	auto msg = GameStateMachine::onShot(responseID, shot);

	// If there was some error, don't touch this.
	if (msg->code == ResponseMessage::Code::INVALID_REQUEST)
		return msg;

	// If we're in the up state and the right target is hit,
	if (state == PopUpState::UP && shot.shot.target == whichTarget) {
		// Award a score to the player who hit it first. Something like remaining milliseconds / 10.
		// TODO: We don't have to worry about hit messages arriving out of order, do we?
		//       For now, just make the winner the first hit we see.
		auto& roundWinner = players[shot.shot.player];
		++roundWinner.hits;
		// On the off-chance that due to some timing fluke, this arrives after the transition time,
		// Award at least 10 points. This is probably unnecessary, but it doesn't hurt to be sure.
		const score_t score = (score_t)(duration_cast<milliseconds>(transitionTime - Clock::now()).count() / 10);
		// Yes, this is verbose and dumb. See
		// http://stackoverflow.com/q/23317404/713961
		roundWinner.score = (score_t)(roundWinner.score + max((score_t)10, score));
		state = PopUpState::SHUTOFF;
	}

	return msg;
}

std::unique_ptr<Message> PopUpStateMachine::onTick(uint16_t messageID)
{
	// Run the base functionality.
	auto msg = GameStateMachine::onTick(messageID);
	assert(msg == nullptr);

	// We have nothing special to do if the game isn't running or we're not finishing up.
	// We fall back to startup when we're all done.
	if (gameState != State::RUNNING && state == PopUpState::STARTUP)
		return msg;

	switch (state) {
		case PopUpState::STARTUP:
			// The startup state does nothing but switch us to DELAY
			transitionToDelay();
			return nullptr;

		case PopUpState::DELAY:
			return duringDelay(messageID);

		case PopUpState::UP:
			duringUp();
			return nullptr;

		case PopUpState::SHUTOFF:
			return duringShutoff(messageID);
	}

	// Should be unreachable
	return msg;
}


std::unique_ptr<Message> PopUpStateMachine::duringDelay(uint16_t messageID)
{
	// The time the target will stay up before going back down
	static const seconds targetWindow(5);

	if (Clock::now() >= transitionTime) {
		// Pick a target
		whichTarget = targetDistribution(rng);
		// Update our state
		state = PopUpState::UP;
		// If nobody shoots this target in five seconds, drop back down
		transitionTime = Clock::now() + targetWindow;
		// Remember when we brought up the target for scoring purposes
		targetUp = Clock::now();
		// Actually turn the target on
		return unique_ptr<Message>(
			new TargetControlMessage(messageID, TargetCommand(whichTarget, true)));
	}
	else {
		return nullptr;
	}
}

void PopUpStateMachine::duringUp()
{
	// If nobody has shot the target in the five seconds it's been up, shut it down.
	if (Clock::now() >= transitionTime)
		state = PopUpState::SHUTOFF;
}

std::unique_ptr<Message> PopUpStateMachine::duringShutoff(uint16_t messageID)
{
	// If the game is over, fall back to startup. Otherwise, back to delay
	if (isOver())
		state = PopUpState::STARTUP;
	else
		transitionToDelay();

	// Shut the target off
	return unique_ptr<Message>(
		new TargetControlMessage(messageID, TargetCommand(whichTarget, false)));
}

void PopUpStateMachine::transitionToDelay()
{
	state = PopUpState::DELAY;
	transitionTime = Clock::now() + seconds(delayDistribution(rng));
}
