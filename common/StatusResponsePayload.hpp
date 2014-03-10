#pragma once

#include <memory>
#include <vector>

#include "ResponsePayload.hpp"

class StatusResponsePayload : public ResponsePayload {

public:

	struct PlayerStats {
		const int score;
		const int hits;

		PlayerStats(int s, int h) : score(s), hits(h) { }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits; }
	};

	typedef std::vector<PlayerStats> PlayerList;

	StatusResponsePayload(int respTo, const std::string& message,
	                      bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats);

	static std::unique_ptr<StatusResponsePayload> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	virtual Type getType() const override { return Type::STATUS_RESPONSE; }

	const bool running;

	const int timeRemaining;

	const int winningScore;

	const PlayerList players;

	bool operator==(const StatusResponsePayload& o) const;
};
