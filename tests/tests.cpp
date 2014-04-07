#include <memory>
#include <queue>

#include "Test.hpp"

#include "MemoryUtilsTests.hpp"
#include "MessageTests.hpp"
#include "MessageQueueTests.hpp"

using namespace Testing;

int main()
{
	memoryUtilsTests();
	MessageTests();
	MessageQueueTests();
	return 0;
}
