#include "PopUpStateMachineTests.hpp"

#include <thread>

#include "GameStateMachine.hpp"
#include "SetupMessage.hpp"
#include "StartMessage.hpp"
#include "ExitMessage.hpp"
#include "Test.hpp"
#include "MessageTests.hpp"
#include "MemoryUtils.hpp"

using namespace std;
using namespace chrono;

/// Macro to quickly set up a test environment for the game state machine(s)
#define MACHINE_ENVIRONMENT \
	MessageQueue in, out; \
	thread stateThread(&runGame, ref(in), ref(out), 2, 2); \
	int id = -1; \
	(void)id; // No unused warnings please

/// Tests that the output queue from MACHINE_ENVIRONMENT is empty
#define ASSERT_EMPTY_OUT \
	assert(out.empty());

/// Macro to send a thread to the input queue from MACHINE_ENVIRONMENT
#define SEND(msg) { \
	auto toSend = msg; \
	id = toSend->id; \
	in.send(move(toSend)); \
}

/// Macro to join the thread from MACHINE_ENVIRONMENT to exit
#define EXIT \
	in.send(unique_ptr<Message>(new ExitMessage(1))); \
	stateThread.join();

namespace {

using Code = ResponseMessage::Code;

using namespace Testing;

typedef steady_clock Clock;

unique_ptr<ShotMessage> shootAt(int8_t targetID, int time)
{
	static uint16_t fireID = 9001;
	// Player 1 is our hero. He is always the guy who hits.
	return unique_ptr<ShotMessage>(
		new ShotMessage(fireID++, Shot(1, targetID, time)));
}

void sanity()
{
	MACHINE_ENVIRONMENT;
	EXIT;
	ASSERT_EMPTY_OUT;
}

void setup()
{
	MACHINE_ENVIRONMENT;
	SEND(makeSetupMessage());
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == id);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void noShoot()
{
	const int gameDuration = 30;
	printf("Running a %d second pop-up game, but no player shoots.\n", gameDuration);
	fflush(stdout);
	MACHINE_ENVIRONMENT;

	SEND(makeSetupMessage(gameDuration));
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == id);

	SEND(makeMessage<StartMessage>());
	ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == id);

	auto endTime = Clock::now() + seconds(gameDuration + 1);
	int8_t lastTarget = -1;
	int messagesReceived = 0;
	for (unique_ptr<Message> msg = out.receiveUntil(endTime); msg != nullptr; msg = out.receiveUntil(endTime)) {

		assert(msg->getType() == Message::Type::TARGET_CONTROL);

		auto tm = unique_dynamic_cast<TargetControlMessage>(move(msg));
		assert(tm != nullptr);
		assert(tm->commands.size() == 1); // We should only be controlling one target at a time.

		const auto& command = tm->commands[0];

		// Even messages should turn targets on, odd ones should turn them off.
		if (messagesReceived++ % 2 == 0) {
			assert(command.on == true);
			printf("Turning target on\n");
			fflush(stdout);
		}
		else {
			assert(command.on == false);
			assert(command.id == lastTarget);
			printf("Turning target off\n");
			fflush(stdout);
		}

		lastTarget = command.id;
	}
	EXIT;
	ASSERT_EMPTY_OUT;
}

void shoot()
{
	const int gameDuration = 30;
	printf("Running a %d second pop-up game, with shooting.\n", gameDuration);
	fflush(stdout);
	MACHINE_ENVIRONMENT;

	SEND(makeSetupMessage(gameDuration));
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == id);

	SEND(makeMessage<StartMessage>());
	ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == id);

	auto startTime = Clock::now();
	auto endTime = Clock::now() + seconds(gameDuration + 1);
	int8_t lastTarget = -1;
	int messagesReceived = 0;
	for (unique_ptr<Message> msg = out.receiveUntil(endTime); msg != nullptr; msg = out.receiveUntil(endTime)) {

		switch (messagesReceived++ % 3) {
			case 0: {
				auto tm = unique_dynamic_cast<TargetControlMessage>(move(msg));
				assert(tm != nullptr);
				assert(tm->commands.size() == 1); // We should only be controlling one target at a time.
				assert(tm->commands[0].on == true);
				lastTarget = tm->commands[0].id;
				printf("Turning target on. Firing!\n");
				fflush(stdout);
				SEND(shootAt(lastTarget, (int)duration_cast<milliseconds>(Clock::now() - startTime).count()));
				break;
			}

			case 1: {
				ack = unique_dynamic_cast<ResponseMessage>(move(msg));
				assert(ack != nullptr);
				assert(ack->code == Code::OK);
				assert(ack->respondingTo == id);
				printf("Shot acknowledged.\n");
				fflush(stdout);
				break;
			}

			case 2: {
				auto tm = unique_dynamic_cast<TargetControlMessage>(move(msg));
				assert(tm != nullptr);
				assert(tm->commands.size() == 1); // We should only be controlling one target at a time.
				assert(tm->commands[0].on == false);
				assert(tm->commands[0].id == lastTarget);
				printf("Turning target off\n");
				fflush(stdout);
				break;
			}
		}

	}
	EXIT;
	ASSERT_EMPTY_OUT;
}

} // end anonymous namespace

void Testing::PopUpStateMachineTests()
{
	beginUnit("PopUpStateMachine");
	test("Sanity", &sanity);
	test("Setup", &setup);
	test("No-shoot run", &noShoot);
	test("Shooting run", &shoot);
}
