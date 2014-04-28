#include "GameStateMachineTests.hpp"

#include <thread>

#include "Test.hpp"
#include "MessageTests.hpp"
#include "MemoryUtils.hpp"
#include "MessageQueue.hpp"
#include "GameStateMachine.hpp"
#include "SetupMessage.hpp"
#include "StatusMessage.hpp"
#include "ResultsMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "ShotMessage.hpp"
#include "ExitMessage.hpp"

using namespace std;

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
	in.send(move(msg)); \
}

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

void earlyStart()
{
	MACHINE_ENVIRONMENT;
	SEND(makeMessage<StartMessage>())
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	// We shouldn't be able to start before actually setting up.
	assert(ack->code == Code::INVALID_REQUEST);
	assert(ack->respondingTo == id);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyStop()
{
	MACHINE_ENVIRONMENT;
	SEND(makeMessage<StopMessage>());
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	// We shouldn't be able to stop before actually setting up and starting.
	assert(ack->code == Code::INVALID_REQUEST);
	assert(ack->respondingTo == id);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyShot()
{
	MACHINE_ENVIRONMENT;
	SEND(makeShotMessage());
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	assert(ack != nullptr);
	// We shouldn't be able to shoot before actually setting up and starting
	assert(ack->code == Code::INVALID_REQUEST);
	assert(ack->respondingTo == id);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyStatus()
{
	MACHINE_ENVIRONMENT;
	SEND(makeMessage<StatusMessage>());
	auto ack = unique_dynamic_cast<StatusResponseMessage>(out.receive());
	// Status should be not running
	assert(ack != nullptr);
	assert(ack->code == Code::OK);
	assert(ack->respondingTo == id);
	assert(!ack->running);
	EXIT;
	ASSERT_EMPTY_OUT;
}

void earlyResults()
{
	MACHINE_ENVIRONMENT;
	SEND(makeMessage<ResultsMessage>());
	auto ack = unique_dynamic_cast<ResponseMessage>(out.receive());
	// We can't get results until after a game
	assert(ack != nullptr);
	assert(ack->code == Code::INVALID_REQUEST);
	assert(ack->respondingTo == id);
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

} // end anonymous namespace

void Testing::GameStateMachineTests()
{
	beginUnit("GameStateMachine");
	test("Sanity", &sanity);
	test("Early start", &earlyStart);
	test("Early stop", &earlyStop);
	test("Early shot", &earlyShot);
	test("Early status", &earlyStatus);
	test("Early results", &earlyResults);
	test("Setup", &setup);
}
