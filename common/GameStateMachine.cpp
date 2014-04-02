#include "GameStateMachine.hpp"

#include <memory>

void gameStateMachine(MessageQueue& in, MessageQueue& out)
{
	// Do we want a timeout?
	for (std::unique_ptr<Message> msg = in.receive();
	     msg->getType() != Message::Type::EXIT;
	     msg = in.receive()) {

		(void)out;
	}
}
