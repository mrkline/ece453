#include "GameStateMachine.hpp"

using namespace std;

void runGame(MessageQueue& in, MessageQueue& out)
{
	unique_ptr<GameStateMachine> machine;

	int uid = 0;

	// Do we want a timeout?
	for (unique_ptr<Message> msg = in.receive();
	     msg->getType() != Message::Type::EXIT;
	     msg = in.receive()) {

		// Some convenience lambdas so the switch statement below is less cluttered
		const auto getStatus = [&] {
			if (machine == nullptr) {
				return unique_ptr<StatusResponseMessage>(
					new StatusResponseMessage(uid++, msg->id, "No game has been setup yet.",
					                          false, -1, -1, StatusResponseMessage::PlayerList()));
			}
			else {
				return machine->getStatusResponse();
			}
		};

		const auto doSetup = [&] {
			if (machine != nullptr && !machine->isRunning()) {
				return unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, ResponseMessage::Code::INVALID_REQUEST,
					                    "You cannot setup a new game while one is in progress"));
			}
			else {
				// Do game setup

				return unique_ptr<ResponseMessage>(
					new ResponseMessage(uid++, msg->id, ResponseMessage::Code::OK, "Game set up."));
			}
		};

		const auto wat = [&] {
			return unique_ptr<ResponseMessage>(
				new ResponseMessage(uid++, msg->id, ResponseMessage::Code::INVALID_REQUEST,
				                    "The request is invalid."));
		};

		try {
			switch (msg->getType()) {

				case Message::Type::SETUP:
					out.send(doSetup());

				case Message::Type::STATUS:
					out.send(getStatus());

				default: // We don't know what this is.
					out.send(wat());
					break;
			}
		}
		catch (const std::exception& e) {
			// Close the input queue, respond to all waiting messages with INTERNAL_ERROR,
			// and return.
			// We'll restart this thread, which is simpler than trying to recover
		}
	}
}
