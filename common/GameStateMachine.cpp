#include "GameStateMachine.hpp"

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

void runGame(MessageQueue& in, MessageQueue& out, int numberTargets, int numberPlayers)
{
	ENFORCE(ArgumentException, numberTargets > 0, "You must have at least one target.");
	ENFORCE(ArgumentException, numberPlayers > 0, "You must have at least one player.");

	using Code = ResponseMessage::Code;

	// A pointer to the state machine running the game
	unique_ptr<GameStateMachine> machine;

	// A unique ID we can use for sending messages (each message must have its own unique ID)
	int uid = 0;

	// The interval in which we should call the state machine's onTick if there are no unprocessed messages.
	static const auto tickInterval = chrono::milliseconds(100);

	// The next time at which we should call onTick
	auto nextTick = chrono::steady_clock::now() + tickInterval;

	// Receive messages as they come in until we get an exit message,
	// and tick in the meantime if we don't receive one.
	for (unique_ptr<Message> msg; msg == nullptr || msg->getType() != Message::Type::EXIT;
		msg = in.receiveBefore(nextTick)) {

		// These are just convenience lambda functions so the switch statement below is less cluttered

		// Setup a new state machine, or complain if now is not the time to do so.
		const auto doSetup = [&] {
			if (machine != nullptr && !machine->isRunning()) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "You cannot setup a new game while one is in progress")));
			}
			else {
				// Do game setup

				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::OK, "Game set up.")));
			}
		};

		// Start the state machine, or complain if now is not the time to do so.
		const auto start = [&] {
			if (machine == nullptr) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "You must set up a game before starting it.")));
			}
			else {
				out.send(machine->start(uid++, msg->id));
			}
		};

		// Stop the state machine, or complain if now is not the time to do so.
		const auto stop = [&] {
			if (machine == nullptr) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "A game has not even been setup yet. There is nothing to stop.")));
			}
			else {
				out.send(machine->stop(uid++, msg->id));
			}
		};

		// Get the status from the state machine,
		// or return our own if there is not a state machine to ask.
		const auto getStatus = [&] {
			if (machine == nullptr) {
				out.send(unique_ptr<StatusResponseMessage>(
					new StatusResponseMessage(uid++, msg->id, "No game has been setup yet.",
					                          false, -1, -1, StatusResponseMessage::PlayerList())));
			}
			else {
				out.send(machine->getStatusResponse(++uid, msg->id));
			}
		};

		// Respond to invalid requests.
		const auto wat = [&] {
			out.send(unique_ptr<ResponseMessage>(
				new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
				                    "The request is invalid.")));
		};


		try {
			// If there is no message, that means we need to tick.
			if (msg == nullptr) {
				if (machine != nullptr) {
					auto toSend = machine->onTick(uid++);
					if (toSend != nullptr)
						out.send(move(toSend));
				}

				// Bump up the next tick
				while (chrono::steady_clock::now() > nextTick)
					nextTick += tickInterval;
			}
			else {
				// Respond to messages. See the lambda functions above.
				using Type = Message::Type;
				switch (msg->getType()) {

					case Type::SETUP:
						doSetup();
						break;

					case Type::START:
						start();
						break;

					case Type::STOP:
						stop();
						break;

					case Type::STATUS:
						getStatus();
						break;

					default: // We don't know what this is.
						wat();
						break;
				}
			}
		}
		catch (const std::exception& e) {
			// Close the input queue, respond to all waiting messages with INTERNAL_ERROR,
			// and return.
			// We'll restart this thread, which is simpler than trying to recover
			in.close();
			for (; msg->getType() != Message::Type::EXIT; msg = in.receive()) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INTERNAL_ERROR,
					                    string("The game state machine encountered the following error:\n")
					                    + e.what())));
			}
			return;
		}
	}
}

GameStateMachine::GameStateMachine(int numTargets, int numPlayers,
	                               const std::chrono::seconds& gameDuration, int scoreToWin) :
	targetCount(numTargets),
	players(numPlayers),
	gameEndTime(),
	duration(gameDuration),
	winningScore(scoreToWin)
{
	ENFORCE(ArgumentException, numTargets > 0, "You must have at least one target.");
	ENFORCE(ArgumentException, numPlayers > 0, "You must have at least one player.");
}

std::unique_ptr<ResponseMessage> GameStateMachine::start(int responseID, int respondingTo)
{
	if (isRunning()) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, respondingTo, ResponseMessage::Code::INVALID_REQUEST,
			                    "A game is already started."));
	}

	// Zero player info
	for (auto& player : players)
		player = Player();

	// Set the game's end time
	gameEndTime = Clock::now() + duration;
	// And we're off!
	gameState = State::RUNNING;

	return unique_ptr<ResponseMessage>(
		new ResponseMessage(responseID, respondingTo, ResponseMessage::Code::OK,
		                    "The game has been successfully started"));
}

std::unique_ptr<ResponseMessage> GameStateMachine::stop(int responseID, int respondingTo)
{
	if (!isRunning()) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, respondingTo, ResponseMessage::Code::INVALID_REQUEST,
			                    "There is no running game to stop."));
	}

	gameState = State::OVER;

	return unique_ptr<ResponseMessage>(
		new ResponseMessage(responseID, respondingTo, ResponseMessage::Code::OK,
		                    "The game has been successfully stopped"));
}


std::unique_ptr<Message> GameStateMachine::onTick(int)
{
	if (any_of(begin(players), end(players), [this](const Player& p) { return p.score >= winningScore; }))
		gameState = State::OVER;

	if (Clock::now() >= gameEndTime)
		gameState = State::OVER;

	return nullptr;
}
