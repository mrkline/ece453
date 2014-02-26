#pragma once

#include <memory>
#include <vector>

#include "ResponsePayload.hpp"

class StatusResponsePayload : public ResponsePayload {

public:

	struct PlayerStats {
		int score;
		int hits;
	};

	typedef std::vector<PlayerStats> PlayerList;

	StatusResponsePayload(int respTo, const std::string& message,
	                      bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats);

	static std::unique_ptr<StatusResponsePayload> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	virtual Type getType() const override { return PT_STATUS_RESPONSE; }

	const bool running;

	const int timeRemaining;

	const int winningScore;

	const PlayerList players;
};
