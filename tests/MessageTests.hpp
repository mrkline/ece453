#pragma once

#include "TestUnit.hpp"

namespace Testing {

class MessageTests : public TestUnit {

public:

	MessageTests();

	const char* getUnitName() const override { return "Message"; }
};

} // end namespace Testing
