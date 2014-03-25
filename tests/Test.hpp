#ifndef __MKB_TEST_HPP__
#define __MKB_TEST_HPP__

#include <functional>

#include "Exceptions.hpp"

#define TEST_ASSERT(c) Testing::test(c, __FILE__, __LINE__)
#define TEST_ASSERT_THROWN(e, t) Testing::testThrown<t>(e, __FILE__, __LINE__);

namespace Testing {

/// Thrown out of Test::Run() if a test fails in an anticipated manner
class TestFailedException {
public:
	TestFailedException(const char* f, int l) :
		file(f),
		line(l)
	{ }

	const char* const file;
	const int line;
};

inline void test(bool cond, const char* file, int line)
{
	if (!cond)
		throw TestFailedException(file, line);
}

template <typename T, typename E>
inline void testThrown(E expr, const char* file, int line)
{
	try { expr(); }
	catch (const T& thrown) {
		return;
	}

	// We didn't catch it
	throw TestFailedException(file, line);
}

/// Represents a single test, a series of which is run by each TestUnit
class Test {
public:
	const char* name;
	std::function<void()> testProc;

	template <typename P>
	Test(const char* n, P proc) : name(n), testProc(std::forward<P>(proc)) { }

	Test(Test&&) = default;

	Test(const Test&) = delete;
	Test& operator=(const Test&) = delete;
};

} // end namespace Testing

#endif
