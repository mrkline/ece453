#pragma once

#include <chrono>
#include <mutex>
#include <condition_variable>

#include <type_traits>

/**
 * \brief Fires off a some action after a given amount of time
 * \tparam A The action to take
 */
template <typename T = std::chrono::milliseconds>
class DelayedAction  {
public:
	template <typename A>
	DelayedAction(T del, A act) :
		over(false),
		mtx(),
		cv(),
		delay(del),
		action(act),
		worker(&DelayedAction::threadProc, this)
	{ }

	~DelayedAction()
	{
		// If we haven't exited yet, do it now
		runEarly();
		worker.join();
	}

	void runEarly()
	{
		std::lock_guard<std::mutex> guard(mtx);
		over = true;
	}

	bool isOver()
	{
		std::lock_guard<std::mutex> guard(mtx);
		return over;
	}

private:

	void threadProc()
	{
		std::unique_lock<std::mutex> lock(mtx);
		if (!cv.wait_for(lock, delay, [&] { return over; }))
			over = true; // If we weren't over yet, we're over now.
		action();
	}

	bool over;
	std::mutex mtx;
	std::condition_variable cv;
	T delay;
	std::function<void()> action;
	std::thread worker;
};
