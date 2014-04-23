#include "GameStateMachine.hpp"

#include <sstream>

#include "Exceptions.hpp"
#include "MemoryUtils.hpp"
#include "TargetControlMessage.hpp"
#include "ShotMessage.hpp"
#include "MovementMessage.hpp"

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

		// Shut off all target LEDs. Useful at the stop point.
		const auto lightsOut = [&] {
			TargetControlMessage::CommandList cl;

			for (int i = 0; i < numberTargets; ++i)
				cl.emplace_back(i, false);

			out.send(unique_ptr<TargetControlMessage>(
				new TargetControlMessage(uid++, move(cl))));
		};

		// Setup a new state machine, or complain if now is not the time to do so.
		const auto doSetup = [&] {
			if (machine != nullptr && !machine->isRunning()) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "You cannot set up a new game while one is in progress")));
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
					                    "A game has not even been set up yet. There is nothing to stop.")));
			}
			else {
				out.send(machine->stop(uid++, msg->id));
				lightsOut();
			}
		};

		// Respond to a shot message
		const auto onShot = [&](const ShotMessage& shot) {
			if (machine == nullptr) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, shot.id, Code::INVALID_REQUEST,
					                    "A game has not been set up. A shot message should not be arriving now.")));
			}
			else {
				out.send(machine->onShot(uid++, shot));
			}
		};

		// Respond to a movement message
		const auto onMovement = [&](const MovementMessage& movement) {
			if (machine == nullptr) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, movement.id, Code::INVALID_REQUEST,
					                    "A game has not been set up. A movement message should not be arriving now.")));
			}
			else {
				out.send(machine->onMovement(uid++, movement));
			}
		};

		// Get the status from the state machine,
		// or return our own if there is not a state machine to ask.
		const auto getStatus = [&] {
			if (machine == nullptr) {
				out.send(unique_ptr<StatusResponseMessage>(
					new StatusResponseMessage(uid++, msg->id, "No game has been set up yet.",
					                          false, -1, -1, StatusResponseMessage::PlayerList())));
			}
			else {
				out.send(machine->getStatusResponse(++uid, msg->id));
			}
		};

		// Gets the results from the state machine,
		// or return our own if there is no state machine to ask.
		const auto getResults = [&] {
			if (machine == nullptr) {
				out.send(unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "A game has not been set up. There are no results to get.")));
			}
			else {
				out.send(machine->getResultsResponse(uid++, msg->id));
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

					case Type::SHOT:
						onShot(*unique_dynamic_cast<ShotMessage>(move(msg)));
						break;

					case Type::MOVEMENT:
						onMovement(*unique_dynamic_cast<MovementMessage>(move(msg)));

					case Type::STATUS:
						getStatus();
						break;

					case Type::RESULTS:
						getResults();
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
	winningScore(scoreToWin),
	shotsWithoutMovement(),
	shotsWithMovement()
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

	// Zero shots
	shotsWithoutMovement.clear();
	shotsWithMovement.clear();

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


std::unique_ptr<ResponseMessage> GameStateMachine::onShot(int responseID, const ShotMessage& shot)
{
	if (gameState == State::SETUP) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, shot.id, ResponseMessage::Code::INVALID_REQUEST,
			                    "Shots cannot be registered before the game even starts."));
	}

	shotsWithoutMovement.emplace(shot.shot);

	stringstream ss;
	ss << "Shot fired at " << shot.shot.time << " registered";

	return unique_ptr<ResponseMessage>(
		new ResponseMessage(responseID, shot.id, ResponseMessage::Code::OK, ss.str()));
}

std::unique_ptr<ResponseMessage> GameStateMachine::onMovement(int responseID, const MovementMessage& movement)
{
	if (gameState == State::SETUP) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, movement.id, ResponseMessage::Code::INVALID_REQUEST,
			                    "Movement cannot be registered before the game even starts."));
	}

	auto it = shotsWithoutMovement.find(movement.movement);

	if (it == end(shotsWithoutMovement)) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, movement.id, ResponseMessage::Code::INVALID_REQUEST,
			                    "The movement's shot was never seen before."));
	}

	// Take the shot out of the "without movement" set
	shotsWithoutMovement.erase(it);

	// Place it into the set of shots with recorded movement.
	shotsWithMovement.emplace(movement.movement);

	stringstream ss;
	ss << "Movement for shot fired at " << movement.movement.time << " registered";

	return unique_ptr<ResponseMessage>(
		new ResponseMessage(responseID, movement.id, ResponseMessage::Code::OK, ss.str()));
}

std::unique_ptr<Message> GameStateMachine::onTick(int)
{
	if (any_of(begin(players), end(players), [this](const Player& p) { return p.score >= winningScore; }))
		gameState = State::OVER;

	if (Clock::now() >= gameEndTime)
		gameState = State::OVER;

	return nullptr;
}
