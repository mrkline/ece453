#include "MessageQueueTests.hpp"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <memory>
#include <random>

#include "Test.hpp"
#include "MessageQueue.hpp"
#include "TestMessage.hpp"
#include "Message.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;

namespace {

/// Utility function for making test messages to send
unique_ptr<Message> makeTestMessage(int value = 0)
{
	unique_ptr<TestMessage> ret(new TestMessage(42));
	ret->val["test"] = "Hi, I'm a test message!";
	ret->val["val"] = value;
	return move(ret);
}

void singleThread()
{
	MessageQueue q;

	for (int msgNum = 0; msgNum < 5; ++msgNum)
		q.send(makeTestMessage(msgNum));

	for (int msgNum = 0; msgNum < 5; ++msgNum)
		assert(*q.receive() == *makeTestMessage(msgNum));

}

void multipleThreads()
{
	MessageQueue q;

	thread receivingThread([&q] {
		for (int msgNum = 0; msgNum < 5; ++msgNum)
			assert(*q.receive() == *makeTestMessage(msgNum));
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

	assert(*q.receive() == *makeTestMessage(-1));
	assert(*q.receive() == *makeTestMessage(1));
}

void timeout()
{
	MessageQueue q;

	thread sendingThread([&q] {
		this_thread::sleep_for(chrono::milliseconds(50));
		q.send(makeTestMessage());
	});

	assert(q.receive(chrono::milliseconds(10)) == nullptr);

	this_thread::sleep_for(chrono::milliseconds(50));
	assert(*q.receive(chrono::milliseconds(10)) == *makeTestMessage());

	sendingThread.join();
}

} // end anonymous namespace

void Testing::MessageQueueTests()
{
	beginUnit("MessageQueue");
	test("Single thread", &singleThread);
	test("Multiple threads", &multipleThreads);
	test("Priority", &priority);
	test("Timeout", &timeout);
}
