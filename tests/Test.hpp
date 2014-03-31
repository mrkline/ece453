#ifndef __MKB_TEST_HPP__
#define __MKB_TEST_HPP__

#include <cassert>
#include <functional>

#include "Exceptions.hpp"

namespace Testing {

template <typename T, typename E>
inline void testThrown(E expr, const char* file, int line)
{
	try { expr(); }
	catch (const T& thrown) {
		return;
	}

	// We didn't catch it
	assert(false);
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
