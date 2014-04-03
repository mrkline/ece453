#pragma once

#include <memory>

#include "MessageQueue.hpp"
#include "StatusResponseMessage.hpp"

void runGame(MessageQueue& in, MessageQueue& out);

class GameStateMachine {

public:

	// Todo: Add more state stuff later
	struct Player {
		int hits;
		int score;

		Player() : hits(0), score(0) { }
	};

	enum class Status {
		SETUP,
		RUNNING,
		OVER
	};

	bool isRunning() { return gameStatus == Status::RUNNING; }

	bool isOver() { return gameStatus == Status::OVER; }

	Status getStatus() { return gameStatus; }

	virtual std::unique_ptr<StatusResponseMessage> getStatusResponse() = 0;

private:

	Status gameStatus;
};
