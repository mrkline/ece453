#pragma once

#include <memory>

#include "MessageQueue.hpp"
#include "ResponseMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsResponseMessage.hpp"

/**
 * \brief Runs a game via a game state machine
 * \param in The MessageQueue on which the machine will receive messages
 * \param out The MessageQueue the machine will use to talk to the UI and hardware.
 *            It is assumed that the two destinations will be multiplexed elsewhere for simplicity here.
 * \param numberTargets The number of targets we currently have up in our hardware setup.
 * \param numberPlayers The number of guns we currently have in our hardware setup.
 *
 * Start this function in another thread, and use the message queues to interface it
 * with our UI and hardware.
 */
void runGame(MessageQueue& in, MessageQueue& out, int numberTargets, int numberPlayers);

/// A base class for a game state machine.
/// Each game type should derive a state machine class from this one.
class GameStateMachine {

public:

	/// Tracks the hit count and score of a player.
	struct Player {
		int hits;
		int score;

		Player() : hits(0), score(0) { }
	};

	/// The game's overall state
	enum class State {
		SETUP, ///< The state machine has been set up, but the game has not been started
		RUNNING, ///< The state machine is running (start has been called)
		OVER ///< The game is over.
	};

	/// The clock to use from the C++ standard library.
	/// We opt for steady_clock since it never shifts (see en.cppreference.com/w/cpp/chrono/steady_clock)
	typedef std::chrono::steady_clock Clock;

	/// Shorthand for the time_point of Clock
	typedef Clock::time_point TimePoint;

	/**
	 * \brief Constructs a game state machine
	 * \param numPlayers The number of players playing the game.
	 *                   The state machine does not make assumptions about how many players hardware supports,
	 *                   so that should be checked elsewhere.
	 * \param gameDuration The duration of the game, in seconds.
	 *                     Pass std::chrono::seconds::max for infinite (ish) duration.
	 * \param scoreToWin The winning score. Pass a negative value for no winning score
	 */
	GameStateMachine(int numTargets, int numPlayers,
	                 const std::chrono::seconds& gameDuration, int scoreToWin);

	/// Returns true if the game is running
	bool isRunning() { return gameState == State::RUNNING; }

	bool isOver() { return gameState == State::OVER; }

	State getState() { return gameState; }

	/**
	 * \brief Responds to a StartMessage to start the game
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StartMessage
	 * \returns A ResponseMessage indicating if the game was succesfully started or not.
	 */
	std::unique_ptr<ResponseMessage> start(int responseID, int respondingTo);

	/**
	 * \brief Responds to a StopMessage to stop the game
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StopMessage
	 * \returns A ResponseMessage indicating if the game was succesfully stopped or not.
	 */
	std::unique_ptr<ResponseMessage> stop(int responseID, int respondingTo);

	/**
	 * \brief Responds to a StatusMessage
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StatusMessage
	 * \returns A StatusResponseMessage indicating the game's current status
	 */
	virtual std::unique_ptr<StatusResponseMessage> getStatusResponse(int responseID, int respondingTo) = 0;

	/**
	 * \brief Responds to a ResultsMessage
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StatusMessage
	 * \returns A ResultsResponseMessage indicating the game's results,
	 *          or a ResponseMessage if the game is not at a point to return results.
	 */
	virtual std::unique_ptr<ResponseMessage> getResultsResponse(int responseID, int respondingTo) = 0;

	/**
	 * \brief Called on a fairly short (100 ms range) periodic interval to allow the state machine to update.
	 * \param messageID A unique ID that the state machine could use to send a message
	 * \returns A message if the machine wants to send one, otherwise null
	 */
	virtual std::unique_ptr<Message> onTick(int messageID);

protected:

	State gameState = State::SETUP;

	const int targetCount;

	std::vector<Player> players;

	TimePoint  gameEndTime;

	const std::chrono::seconds duration;

	const int winningScore;
};
