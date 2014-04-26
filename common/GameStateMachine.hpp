#pragma once

#include <memory>
#include <set>
#include <unordered_set>

#include "MessageQueue.hpp"
#include "ResponseMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsResponseMessage.hpp"

// Forward declarations. We don't need to include the hearders because we just have references here.
// We'll include the headers in the .cpp file
class ShotMessage;
class MovementMessage;

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
void runGame(MessageQueue& in, MessageQueue& out, int8_t numberTargets, int numberPlayers);

/// A base class for a game state machine.
/// Each game type should derive a state machine class from this one.
class GameStateMachine {

public:

	/// Tracks the hit count and score of a player.
	struct Player {
		int score;
		int hits;

		Player() : score(0), hits(0) { }

		Player(int s, int h) : score(s), hits(h) { }
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
	std::unique_ptr<ResponseMessage> start(uint16_t responseID, uint16_t respondingTo);

	/**
	 * \brief Responds to a StopMessage to stop the game
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StopMessage
	 * \returns A ResponseMessage indicating if the game was succesfully stopped or not.
	 */
	std::unique_ptr<ResponseMessage> stop(uint16_t responseID, uint16_t respondingTo);

	/**
	 * \brief Responds to a ShotMessage and records the shot
	 * \param responseID an ID for the returning acknowledgement
	 * \param shot The shot message
	 * \returns An acknowledgement for the shot message
	 */
	virtual std::unique_ptr<ResponseMessage> onShot(uint16_t responseID, const ShotMessage& shot);

	/**
	 * \brief Responds to a MovementMessage and matches the movement to the shot
	 * \param responseID an ID for the returning acknowledgement
	 * \param movement The movement message
	 * \returns An acknowledgement for the shot message
	 */
	std::unique_ptr<ResponseMessage> onMovement(uint16_t responseID, const MovementMessage& movement);

	/**
	 * \brief Responds to a StatusMessage
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StatusMessage
	 * \returns A StatusResponseMessage indicating the game's current status
	 */
	std::unique_ptr<StatusResponseMessage> getStatusResponse(uint16_t responseID, uint16_t respondingTo);

	/**
	 * \brief Responds to a ResultsMessage
	 * \param responseID An ID for the returning message
	 * \param respondingTo The ID of the StatusMessage
	 * \returns A ResultsResponseMessage indicating the game's results,
	 *          or a ResponseMessage if the game is not at a point to return results.
	 */
	std::unique_ptr<ResponseMessage> getResultsResponse(uint16_t responseID, uint16_t respondingTo);

	/**
	 * \brief Called on a fairly short (100 ms range) periodic interval to allow the state machine to update.
	 * \param messageID A unique ID that the state machine could use to send a message
	 * \returns A message if the machine wants to send one, otherwise null
	 */
	virtual std::unique_ptr<Message> onTick(uint16_t messageID);

protected:

	State gameState = State::SETUP;

	const int targetCount;

	std::vector<Player> players;

	TimePoint  gameEndTime;

	const std::chrono::seconds duration;

	const int winningScore;

	/// When we receive a shot, it is put here until its corresponding movement arrives.
	std::unordered_set<Shot> shotsWithoutMovement;

	/// After we receive a shot's movement, it is put here, ordered by time
	std::set<ShotWithMovement> shotsWithMovement;
};
