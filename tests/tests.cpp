#include <memory>
#include <queue>

#include "Test.hpp"

#include "MemoryUtilsTests.hpp"
#include "MessageTests.hpp"
#include "MessageQueueTests.hpp"
#include "GameStateMachineTests.hpp"

using namespace Testing;

int main()
{
	memoryUtilsTests();
	MessageTests();
	MessageQueueTests();
	GameStateMachineTests();
	return 0;
}
