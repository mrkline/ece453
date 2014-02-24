#pragma once

#include <vector>

#include "ResponsePayload.hpp"

class StatusResponsePayload : public ResponsePayload {

public:

	struct PlayerStats {
		int score;
		int hits;
	};

	typedef std::vector<PlayerStats> PlayerList;

	StatusResponsePayload(int respTo, bool isRunning, int timeLeft, int winScore, PlayerList&& playerStats);

	const bool running;

	const int timeRemaining;

	const int winningScore;

	const PlayerList players;

	virtual Type getType() const override { return PT_STATUS_RESPONSE; }
};
