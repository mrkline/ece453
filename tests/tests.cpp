#include <memory>
#include <queue>

#include "Test.hpp"
#include "TestUnit.hpp"

using namespace Testing;
using namespace std;

namespace {

typedef unique_ptr<TestUnit> UnitPtr;

queue<UnitPtr> testUnits;

} // end anonymous namespace

int main()
{
	// Push tests here
	
	while (!testUnits.empty()) {
		const auto& curr = testUnits.front();
		try {
			printf("Beginning test unit %s...\n\n", curr->getUnitName());
			curr->runUnit();
		}
		catch (...) {
			printf("The test unit %s threw an unexpected exception.\n",
			       curr->getUnitName());
		}
		testUnits.pop();
	}
	return 0;
}