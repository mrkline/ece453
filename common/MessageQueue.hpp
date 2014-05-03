#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <deque>

#include "Message.hpp"

/// A thread-safe message queue
class MessageQueue {

public:

	MessageQueue() : q(), qMutex(), closed(false), notifier() { }

	/**
	 * \brief Places a message at the back of the queue
	 * \param toSend An rvalue unique_ptr of the message to send.
	 *               An rvalue is used so that once sent,
	 *               the message can no longer be modified in its current context.
	 */
	void send(std::unique_ptr<Message>&& toSend);

	/**
	 * \brief Places a message at the front of the queue
	 * \param toSend An rvalue unique_ptr of the message to send.
	 *               An rvalue is used so that once sent,
	 *               the message can no longer be modified in its current context.
	 */
	void prioritySend(std::unique_ptr<Message>&& toSend);

	/// Returns a const pointer to the top message
	const Message* peek();

	/// Dequeues a message, blocking indefinitely if the queue is empty
	std::unique_ptr<Message> receive();

	/**
	 * \brief Dequeues a message, blocking for the given timeout duration if the queue is empty
	 * \param timeout The time interval, in milliseconds, to wait if the queue is empty
	 * \returns A dequeued message or nullptr if the timeout is reached
	 */
	template <typename Rep, typename Period>
	std::unique_ptr<Message> receive(const std::chrono::duration<Rep, Period>& timeout)
	{
		// If we are not allowed to dequeue right now, just wait the expected time and return
		std::unique_lock<std::mutex> lock(qMutex);
		if (notifier.wait_for(lock, timeout, [this] { return !q.empty(); })) {
			auto ret = std::move(q.front());
			q.pop_front();
			return ret;
		}
		else {
			return nullptr;
		}
	}

	template <typename Clock, typename Duration>
	std::unique_ptr<Message> receiveUntil(const std::chrono::time_point<Clock, Duration>& time)
	{
		std::unique_lock<std::mutex> lock(qMutex);
		if (notifier.wait_until(lock, time, [this] { return !q.empty(); })) {
			auto ret = std::move(q.front());
			q.pop_front();
			return ret;
		}
		else {
			return nullptr;
		}
	}

	/// Returns true if the queue is empty
	bool empty();

	/// Closes the message queue from further insertions until it is reset
	void close();

	bool isClosed() const { return closed; }

	bool isOpen() const {return !closed; }

	/// Resets the queue - clears it and uncloses it
	void reset();

	// Disallow copy and assign
	MessageQueue(const MessageQueue&) = delete;
	MessageQueue& operator=(const MessageQueue&) = delete;

private:

	std::deque<std::unique_ptr<Message>> q;
	std::mutex qMutex;
	std::atomic_bool closed;
	std::condition_variable notifier;
};
