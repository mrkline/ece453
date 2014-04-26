#pragma once

#include <memory>
#include <vector>

#include "ResponseMessage.hpp"

class StatusResponseMessage : public ResponseMessage {

public:

	struct PlayerStats {
		const int16_t score;
		const int16_t hits;

		PlayerStats(int16_t s, int16_t h) : score(s), hits(h) { }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits; }
	};

	typedef std::vector<PlayerStats> PlayerList;

	StatusResponseMessage(uint16_t id, uint16_t respTo, const std::string& message,
	                      bool isRunning, int16_t timeLeft, int16_t winScore, PlayerList&& playerStats);

#ifdef WITH_JSON
	static std::unique_ptr<StatusResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	virtual Type getType() const override { return Type::STATUS_RESPONSE; }

	const bool running;

	const int16_t timeRemaining;

	const int16_t winningScore;

	const PlayerList players;

	bool operator==(const Message& o) const override;
};
