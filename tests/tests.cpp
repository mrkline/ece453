#include <memory>
#include <queue>

#include "Test.hpp"

#include "MessageTests.hpp"
#include "MessageQueueTests.hpp"

using namespace Testing;

int main()
{
	MessageTests();
	MessageQueueTests();
	return 0;
}
