#include "MessageQueue.hpp"

#include "Exceptions.hpp"

using namespace std;

void MessageQueue::send(std::unique_ptr<Message>&& toSend)
{
	ENFORCE(Exceptions::ArgumentException, toSend != nullptr, "You can not enqueue a null message.");

	lock_guard<mutex> lock(qMutex);
	q.emplace_back(std::move(toSend));

	// Notify anyone waiting for additional files that more have arrived
	notifier.notify_one();
}

void MessageQueue::prioritySend(std::unique_ptr<Message>&& toSend)
{
	ENFORCE(Exceptions::ArgumentException, toSend != nullptr, "You can not enqueue a null message.");

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

void MessageQueue::clear()
{
	lock_guard<mutex> lock(qMutex);
	q.clear();
}
