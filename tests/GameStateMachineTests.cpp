#include "GameStateMachineTests.hpp"

#include <thread>

#include "Test.hpp"
#include "MemoryUtils.hpp"
#include "MessageQueue.hpp"
#include "GameStateMachine.hpp"
#include "SetupMessage.hpp"
#include "StatusMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "ExitMessage.hpp"

using namespace std;

/// Macro to quickly set up a test environment for the game state machine(s)
#define MACHINE_ENVIRONMENT \
	MessageQueue in, out; \
	thread stateThread(&runGame, ref(in), ref(out), 2, 2);

/// Tests that the output queue from MACHINE_ENVIRONMENT is empty
#define ASSERT_EMPTY_OUT \
	assert(out.empty());

/// Macro to join the thread from MACHINE_ENVIRONMENT to exit
#define EXIT \
	in.send(unique_ptr<Message>(new ExitMessage(1))); \
	stateThread.join();

/// Macro to send a thread to the input queue from MACHINE_ENVIRONMENT
#define SEND(msg) \
	in.send(unique_ptr<Message>(msg));

namespace {

using Code = ResponseMessage::Code;

void sanity()
{
	MACHINE_ENVIRONMENT;
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyStart()
{
	MACHINE_ENVIRONMENT;
	SEND(new StartMessage(0));
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	// We shouldn't be able to start before actually setting up.
	assert(ack->code == Code::INVALID_REQUEST);
	assert(ack->respondingTo == 0);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyStop()
{
	MACHINE_ENVIRONMENT;
	SEND(new StopMessage(0));
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	// We shouldn't be able to stop before actually setting up and starting.
	assert(ack->code == Code::INVALID_REQUEST);
	assert(ack->respondingTo == 0);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyStatus()
{
	MACHINE_ENVIRONMENT;
	SEND(new StatusMessage(0));
	auto ack = unique_dynamic_cast<StatusResponseMessage>(out.receive());
	// Status should be not running
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == 0);
	assert(!ack->running);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void setup()
{
	MACHINE_ENVIRONMENT;
	SEND(new SetupMessage(0, GameType::POP_UP, 1, WinCondition::WC_TIME, 60, -1));
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == 0);
	EXIT;
	ASSERT_EMPTY_OUT;
}

} // end anonymous namespace

void Testing::GameStateMachineTests()
{
	beginUnit("GameStateMachine");
	test("Sanity", &sanity);
	test("Early start", &earlyStart);
	test("Early stop", &earlyStop);
	test("Early status", &earlyStatus);
	test("Setup", &setup);
}
