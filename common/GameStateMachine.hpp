#pragma once

#include <memory>

#include "MessageQueue.hpp"
#include "ResponseMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsResponseMessage.hpp"

void runGame(MessageQueue& in, MessageQueue& out);

class GameStateMachine {

public:

	// Todo: Add more state stuff later
	struct Player {
		int hits;
		int score;

		Player() : hits(0), score(0) { }
	};

	enum class State {
		SETUP,
		RUNNING,
		OVER
	};

	bool isRunning() { return gameState == State::RUNNING; }

	bool isOver() { return gameState == State::OVER; }

	State getState() { return gameState; }

	virtual std::unique_ptr<ResponseMessage> start(int repsonseID, int respondingTo);

	virtual std::unique_ptr<ResponseMessage> stop(int responseID, int respondingTo);

	virtual std::unique_ptr<StatusResponseMessage> getStatusResponse(int responseID, int respondingTo) = 0;

	virtual std::unique_ptr<ResultsResponseMessage> getResultsResponse(int responseID, int respondingTo) = 0;

protected:

	State gameState;
};
