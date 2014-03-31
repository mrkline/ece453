#ifndef __MKB_TEST_UNIT_HPP__
#define __MKB_TEST_UNIT_HPP__

#include <memory>
#include <list>
#include <queue>

#include "Test.hpp"

namespace Testing {

/**
 * \brief Provides a base abstract class for test units.
 *
 * A test unit is a group of related tests related to a given component.
 */
class TestUnit {
public:
	TestUnit() : unrunTests() { }

	/// Returns this unit's name
	virtual const char* getUnitName() const = 0;

	/// Executes all Tests and prints a summary of the unit
	void runUnit();

protected:
	std::queue<Test> unrunTests;

	/// Convenience function to add to unrun tests
	template <typename P>
	void add(const char* testName, P proc) { unrunTests.emplace(testName, proc); }
};

} // end namespace Testing

#endif
