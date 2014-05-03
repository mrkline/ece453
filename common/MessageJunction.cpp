#include "MessageJunction.hpp"

#include <chrono>

using namespace std::chrono;

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
	}
}
