#include "MessageJunction.hpp"
#include "MemoryUtils.hpp"

#include "Exceptions.hpp"
#include "ResponseMessage.hpp"

#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace Exceptions;

typedef std::chrono::steady_clock Clock;

void runMessageJunction(MessageQueue& toSM, MessageQueue& fromSM,
                        MessageQueue& toUI, MessageQueue& fromUI,
                        MessageQueue& toSys, MessageQueue& fromSys)
{
	// Somewhat arbitrarily chosen, but currently 1/3 of the game state machine tick time.
	static const auto timeSlice = milliseconds(33);

	while (true) {
		// Only spend a certain amount of time handling each source,
		// so we don't starve another.
		const auto start = Clock::now();
		const auto stateMachineEnd = start + timeSlice;
		const auto userInterfaceEnd = stateMachineEnd + timeSlice;
		const auto systemEnd = userInterfaceEnd + timeSlice;

		for (auto msg = fromSM.receiveUntil(stateMachineEnd);
			msg != nullptr;
			msg = fromSM.receiveUntil(stateMachineEnd)) {

			// Try casting to a response.
			// Responses from the state machine are going to the UI.
			// Otherwise they are commands and go to the system
			auto response = unique_dynamic_cast<ResponseMessage>(move(msg));

			if (response != nullptr)
				toUI.send(move(response));
			else
				toSys.send(move(msg));
		}

		for (auto msg = fromUI.receiveUntil(userInterfaceEnd);
			msg != nullptr;
			msg = fromUI.receiveUntil(stateMachineEnd)) {

			// Try casting to a response.
			// Things shouldn't be sending messages to the UI,
			// so it shouldn't be responding.
			// Messages from the UI go to the state machine
			auto response = unique_dynamic_cast<ResponseMessage>(move(msg));

			if (response != nullptr)
				THROW(InvalidOperationException, "Response from UI: " + response->message);
			else
				toSM.send(move(msg));
		}

		for (auto msg = fromSys.receiveUntil(systemEnd);
			msg != nullptr;
			msg = fromSys.receiveUntil(systemEnd)) {

			// Messages from the system go to the state machine.
			// TODO: Make a copy and send it to the UI?
			toSM.send(move(msg));
		}
	}
}
