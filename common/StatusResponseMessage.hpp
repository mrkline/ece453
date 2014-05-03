#pragma once

#include <memory>
#include <vector>

#include "Exceptions.hpp"
#include "ResponseMessage.hpp"

/// Sent as a response to a StatusMessage to indicate the current state of the system
class StatusResponseMessage : public ResponseMessage {

public:

	struct PlayerStats {
		const score_t score;
		const shot_t hits;

		PlayerStats(score_t s, shot_t h) : score(s), hits(h) { }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits; }
	};

	typedef std::vector<PlayerStats> PlayerList;

	/**
	 * \brief Constructs a status response message
	 * \param id The message ID
	 * \param respTo The ID of the StautsMessage we are responding to
	 * \param message The response message, if any
	 * \param isRunning true if a game is currently running
	 * \param timeLeft The time remaining in the game.
	 *                 -1 if the game has no time limit or if no game is currently running.
	 * \param winScore The score required to win the game.
	 *                 -1 if the game has no score limit or if no game is currently running.
	 * \param playerStats A list of player stats. Empty if the game is not currently running.
	 */
	StatusResponseMessage(message_id_t id, message_id_t respTo, const std::string& message,
	                      bool isRunning, duration_t timeLeft, score_t winScore, PlayerList&& playerStats);

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<StatusResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	/// The StatusResponseMessage is not serializable to binary
	/// \throws Exceptions::InvalidOperationException upon being called
	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	virtual Type getType() const override { return Type::STATUS_RESPONSE; }

	const bool running;

	const duration_t timeRemaining;

	const score_t winningScore;

	const PlayerList players;

	bool operator==(const Message& o) const override;
};
