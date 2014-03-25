#pragma once

#include "TestUnit.hpp"

namespace Testing {

class MessageQueueTests : public TestUnit {

public:

	MessageQueueTests();

	const char* getUnitName() const override { return "MessageQueue"; }
};

} // end namespace Testing
