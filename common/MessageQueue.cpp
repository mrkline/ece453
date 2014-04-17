#include "MessageQueue.hpp"

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

void MessageQueue::send(std::unique_ptr<Message>&& toSend)
{
	ENFORCE(ArgumentException, toSend != nullptr, "You can not enqueue a null message.");
	ENFORCE(InvalidOperationException, !closed, "The queue has been closed. You cannot send until it is reset.");

	lock_guard<mutex> lock(qMutex);
	q.emplace_back(std::move(toSend));

	// Notify anyone waiting for additional files that more have arrived
	notifier.notify_one();
}

void MessageQueue::prioritySend(std::unique_ptr<Message>&& toSend)
{
	ENFORCE(ArgumentException, toSend != nullptr, "You can not enqueue a null message.");
	ENFORCE(InvalidOperationException, !closed, "The queue has been closed. You cannot send until it is reset.");

	lock_guard<mutex> lock(qMutex);
	q.emplace_front(std::move(toSend));

	// Notify anyone waiting for additional files that more have arrived
	notifier.notify_one();
}

std::unique_ptr<Message> MessageQueue::receive()
{
	// If we are not allowed to dequeue right now, just wait the expected time and return
	unique_lock<mutex> lock(qMutex);
	notifier.wait(lock, [this] { return !q.empty(); });
	auto ret = std::move(q.front());
	q.pop_front();
	return ret;
}

std::unique_ptr<Message> MessageQueue::receive(const std::chrono::milliseconds& timeout)
{
	// If we are not allowed to dequeue right now, just wait the expected time and return
	unique_lock<mutex> lock(qMutex);
	if (notifier.wait_for(lock, timeout, [this] { return !q.empty(); })) {
		auto ret = std::move(q.front());
		q.pop_front();
		return ret;
	}
	else {
		return nullptr;
	}
}

std::unique_ptr<Message> MessageQueue::receiveBefore(const std::chrono::time_point<std::chrono::steady_clock>& time)
{
	unique_lock<mutex> lock(qMutex);
	if (notifier.wait_until(lock, time, [this] { return !q.empty(); })) {
		auto ret = std::move(q.front());
		q.pop_front();
		return ret;
	}
	else {
		return nullptr;
	}
}

bool MessageQueue::empty()
{
	unique_lock<mutex> lock(qMutex);
	return q.empty();
}

void MessageQueue::close()
{
	closed = true;
}

void MessageQueue::reset()
{
	lock_guard<mutex> lock(qMutex);
	q.clear();
	closed = false;
}
