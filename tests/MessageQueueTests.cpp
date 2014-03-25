#include "MessageQueueTests.hpp"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <memory>
#include <random>

#include "MessageQueue.hpp"
#include "TestPayload.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;

namespace {

/// Utility function for making test messages to send
unique_ptr<Message> makeTestMessage(int value = 0)
{
	unique_ptr<TestPayload> load(new TestPayload);
	load->val["test"] = "Hi, I'm a test message!";
	load->val["val"] = value;
	unique_ptr<Message> ret(new Message(Payload::Type::TEST, 1, 42, move(load)));
	return ret;
}

void singleThread()
{
	MessageQueue q;

	for (int msgNum = 0; msgNum < 5; ++msgNum)
		q.send(makeTestMessage(msgNum));

	for (int msgNum = 0; msgNum < 5; ++msgNum)
		TEST_ASSERT(*q.receive() == *makeTestMessage(msgNum));

}

void multipleThreads()
{
	MessageQueue q;

	thread receivingThread([&q] {
		for (int msgNum = 0; msgNum < 5; ++msgNum)
			TEST_ASSERT(*q.receive() == *makeTestMessage(msgNum));
	});

	this_thread::yield(); // Give the receiving thread a chance to take off

	thread sendingThread([&q] {
		for (int msgNum = 0; msgNum < 5; ++msgNum)
			q.send(makeTestMessage(msgNum));
	});

	receivingThread.join();
	sendingThread.join();
}

void priority()
{
	MessageQueue q;

	q.send(makeTestMessage(1));
	q.prioritySend(makeTestMessage(-1));

	TEST_ASSERT(*q.receive() == *makeTestMessage(-1));
	TEST_ASSERT(*q.receive() == *makeTestMessage(1));
}

void timeout()
{
	MessageQueue q;

	thread sendingThread([&q] {
		this_thread::sleep_for(chrono::milliseconds(50));
		q.send(makeTestMessage());
	});

	TEST_ASSERT(q.receive(chrono::milliseconds(10)) == nullptr);

	this_thread::sleep_for(chrono::milliseconds(50));
	TEST_ASSERT(*q.receive(chrono::milliseconds(10)) == *makeTestMessage());

	sendingThread.join();
}

} // end anonymous namespace


Testing::MessageQueueTests::MessageQueueTests()
{
	add("Single thread", &singleThread);
	add("Multiple threads", &multipleThreads);
	add("Priority", &priority);
	add("Timeout", &timeout);
}
