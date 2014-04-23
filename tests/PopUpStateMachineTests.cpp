#include "PopUpStateMachineTests.hpp"

#include <thread>

#include "GameStateMachine.hpp"
#include "PopUpStateMachine.hpp"
#include "ExitMessage.hpp"
#include "Test.hpp"

using namespace std;

/// Macro to quickly set up a test environment for the game state machine(s)
#define MACHINE_ENVIRONMENT \
	MessageQueue in, out; \
	thread stateThread(&runGame, ref(in), ref(out), 2, 2);

/// Tests that the output queue from MACHINE_ENVIRONMENT is empty
#define ASSERT_EMPTY_OUT \
	assert(out.empty());

/// Macro to send a thread to the input queue from MACHINE_ENVIRONMENT
#define SEND(msg) \
	auto toSend = msg; \
	const auto id = toSend->id; \
	in.send(move(msg));

/// Macro to join the thread from MACHINE_ENVIRONMENT to exit
#define EXIT \
	in.send(unique_ptr<Message>(new ExitMessage(1))); \
	stateThread.join();

namespace {

using Code = ResponseMessage::Code;

using namespace Testing;

void sanity()
{
	MACHINE_ENVIRONMENT;
	EXIT;
	ASSERT_EMPTY_OUT;
}

} // end anonymous namespace

void Testing::PopUpStateMachineTests()
{
	beginUnit("PopUpStateMachine");
	test("Sanity", &sanity);
}
