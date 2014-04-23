#include <memory>
#include <queue>

#include "Test.hpp"

#include "MemoryUtilsTests.hpp"
#include "MessageTests.hpp"
#include "MessageQueueTests.hpp"
#include "GameStateMachineTests.hpp"
#include "PopUpStateMachineTests.hpp"

using namespace Testing;

int main()
{
	memoryUtilsTests();
	MessageTests();
	MessageQueueTests();
	GameStateMachineTests();
	PopUpStateMachineTests();
	return 0;
}
