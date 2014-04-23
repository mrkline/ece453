#pragma once

#include <random>

#include "GameStateMachine.hpp"

class PopUpStateMachine : public GameStateMachine {

public:

	/**
	 * \brief Constructs a state machine for the pop-up game type
	 * \param numPlayers The number of players playing the game.
	 *                   The state machine does not make assumptions about how many players hardware supports,
	 *                   so that should be checked elsewhere.
	 * \param gameDuration The duration of the game, in seconds.
	 *                     Pass std::chrono::seconds::max for infinite (ish) duration.
	 * \param scoreToWin The winning score. Pass a negative value for no winning score
	 */
	PopUpStateMachine(int numTargets, int numPlayers,
	                  const std::chrono::seconds& gameDuration, int scoreToWin);

	std::unique_ptr<ResponseMessage> onShot(int responseID, const ShotMessage& shot) override;

	std::unique_ptr<Message> onTick(int messageID) override;

private:

	enum class PopUpState {
		STARTUP, ///< An initial state where we do nothing until the first tick
		DELAY, ///< The delay between targets popping up
		UP, ///< When a target is up and waiting for a player to shoot it
		SHUTOFF ///< Shut off the target after it has been hit
	};

	void duringStartup();

	std::unique_ptr<Message> duringDelay(int messageID);

	std::unique_ptr<Message> duringShutoff(int messageID);

	/// A pseusdo-random number generator for creating delay times;
	std::mt19937 rng;

	/// The random distribution for delay times
	std::uniform_int_distribution<> delayDistribution;

	/// The random distribution of the target to pop up
	std::uniform_int_distribution<> targetDistribution;

	PopUpState state;

	/// When applicable, stores the transition time when we move to the next state
	TimePoint transitionTime;

	/// Stores the time at which a target went up
	TimePoint targetUp;

	/// Stores the target we brought up
	int whichTarget;
};
