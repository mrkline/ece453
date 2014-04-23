#include "PopUpStateMachine.hpp"

#include <cassert>

#include "TargetControlMessage.hpp"

using namespace std;

PopUpStateMachine::PopUpStateMachine(int numTargets, int numPlayers,
                                     const std::chrono::seconds& gameDuration, int scoreToWin) :
	GameStateMachine(numTargets, numPlayers, gameDuration, scoreToWin),
	rng(std::random_device()()),
	delayDistribution(5, 10),
	targetDistribution(0, numTargets - 1),
	state(PopUpState::STARTUP),
	transitionTime(),
	targetUp(),
	whichTarget(-1)
{
}

std::unique_ptr<ResponseMessage> PopUpStateMachine::onShot(int responseID, const ShotMessage& shot)
{
	auto msg = GameStateMachine::onShot(responseID, shot);

	// If there was some error, don't touch this.
	if (msg->code == ResponseMessage::Code::INVALID_REQUEST)
		return msg;

	// TODO: Pick up here
	// If we're in the up state and the right target is hit,
	// award a score to the player who hit it first. Something like milliseconds / 10.
	// Ignore hits from before our target went up
}

std::unique_ptr<Message> PopUpStateMachine::onTick(int messageID)
{
	// Run the base functionality.
	auto msg = GameStateMachine::onTick(messageID);
	assert(msg == nullptr);

	// We have nothing special to do if the game isn't running or we're not finishing up
	if (gameState != State::RUNNING && state != PopUpState::UP)
		return msg;

	switch (state) {
		case PopUpState::STARTUP:
			duringStartup();
			return nullptr;

		case PopUpState::DELAY:
			return duringDelay(messageID);

		case PopUpState::UP:
			// Do nothing. Whil up, we're handled by onShot
			return nullptr;
	}
}


void PopUpStateMachine::duringStartup()
{
	state = PopUpState::DELAY;
	transitionTime = Clock::now() + chrono::seconds(delayDistribution(rng));
}

std::unique_ptr<Message> PopUpStateMachine::duringDelay(int messageID)
{
	if (Clock::now() >= transitionTime) {
		// Pick a target
		whichTarget = targetDistribution(rng);
		// Update our state
		gameState = PopUpState::UP;
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

std::unique_ptr<Message> PopUpStateMachine::duringShutoff(int messageID)
{
	// TODO: Pick up here. Switch to delay, set the transitionTime,
	// and send a shutoff command to the target we lit last time
}
