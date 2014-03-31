#pragma once

#include <memory>
#include <vector>

#include "ResponseMessage.hpp"

class StatusResponseMessage : public ResponseMessage {

public:

	struct PlayerStats {
		const int score;
		const int hits;

		PlayerStats(int s, int h) : score(s), hits(h) { }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits; }
	};

	typedef std::vector<PlayerStats> PlayerList;

	StatusResponseMessage(int id, int respTo, const std::string& message,
	                      bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats);

	static std::unique_ptr<StatusResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	virtual Type getType() const override { return Type::STATUS_RESPONSE; }

	const bool running;

	const int timeRemaining;

	const int winningScore;

	const PlayerList players;

	bool operator==(const Message& o) const override;
};