#pragma once

#include <memory>
#include <vector>

#include "Exceptions.hpp"
#include "ResponseMessage.hpp"

class StatusResponseMessage : public ResponseMessage {

public:

	struct PlayerStats {
		const score_t score;
		const shot_t hits;

		PlayerStats(score_t s, shot_t h) : score(s), hits(h) { }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits; }
	};

	typedef std::vector<PlayerStats> PlayerList;

	StatusResponseMessage(message_id_t id, message_id_t respTo, const std::string& message,
	                      bool isRunning, duration_t timeLeft, score_t winScore, PlayerList&& playerStats);

#ifdef WITH_JSON
	static std::unique_ptr<StatusResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

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
