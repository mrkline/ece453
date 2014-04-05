#include "GameStateMachine.hpp"

using namespace std;

void runGame(MessageQueue& in, MessageQueue& out)
{
	using Code = ResponseMessage::Code;

	unique_ptr<GameStateMachine> machine;

	int uid = 0;

	// Do we want a timeout?
	for (unique_ptr<Message> msg = in.receive();
	     msg->getType() != Message::Type::EXIT;
	     msg = in.receive()) {

		// Some convenience lambdas so the switch statement below is less cluttered
		const auto doSetup = [&] {
			if (machine != nullptr && !machine->isRunning()) {
				return unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "You cannot setup a new game while one is in progress"));
			}
			else {
				// Do game setup

				return unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::OK, "Game set up."));
			}
		};

		const auto start = [&] {
			if (machine == nullptr) {
				return unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "You must set up a game before starting it."));
			}
			else {
				return machine->start(uid++, msg->id);
			}
		};

		const auto stop = [&] {
			if (machine == nullptr) {
				return unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
					                    "A game has not even been setup yet. There is nothing to stop."));
			}
			else {
				return machine->stop(uid++, msg->id);
			}
		};

		const auto getStatus = [&] {
			if (machine == nullptr) {
				return unique_ptr<StatusResponseMessage>(
					new StatusResponseMessage(uid++, msg->id, "No game has been setup yet.",
					                          false, -1, -1, StatusResponseMessage::PlayerList()));
			}
			else {
				return machine->getStatusResponse(++uid, msg->id);
			}
		};

		const auto wat = [&] {
			return unique_ptr<ResponseMessage>(
				new ResponseMessage(uid++, msg->id, Code::INVALID_REQUEST,
				                    "The request is invalid."));
		};

		using Type = Message::Type;

		try {
			switch (msg->getType()) {

				case Type::SETUP:
					out.send(doSetup());
					break;

				case Type::START:
					out.send(start());
					break;

				case Type::STOP:
					out.send(stop());
					break;

				case Type::STATUS:
					out.send(getStatus());
					break;

				default: // We don't know what this is.
					out.send(wat());
					break;
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

std::unique_ptr<ResponseMessage> GameStateMachine::start(int responseID, int respondingTo)
{
	if (isRunning()) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, respondingTo, ResponseMessage::Code::INVALID_REQUEST,
			                    "A game is already started."));
	}

	// Child implmementations will pick up from here
	return nullptr;
}

std::unique_ptr<ResponseMessage> GameStateMachine::stop(int responseID, int respondingTo)
{
	if (!isRunning()) {
		return unique_ptr<ResponseMessage>(
			new ResponseMessage(responseID, respondingTo, ResponseMessage::Code::INVALID_REQUEST,
			                    "There is no running game to stop."));
	}

	// Child implementations will pick up from here
	return nullptr;
}
