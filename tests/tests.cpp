#include <memory>
#include <queue>

#include "Test.hpp"

#include "MemoryUtilsTests.hpp"
#include "MessageTests.hpp"
#include "MessageQueueTests.hpp"
#include "GameStateMachineTests.hpp"
#include "PopUpStateMachineTests.hpp"
#include "BinaryMessageTests.hpp"

using namespace Testing;

int main()
{
	memoryUtilsTests();
	MessageTests();
	MessageQueueTests();
	BinaryMessageTests();
	GameStateMachineTests();
	// Slowest ones last
	PopUpStateMachineTests();
	return 0;
}
