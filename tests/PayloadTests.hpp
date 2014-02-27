#pragma once

#include "TestUnit.hpp"

namespace Testing {

class PayloadTests : public TestUnit {

public:

	PayloadTests();

	const char* getUnitName() const override { return "Payload"; }
};

} // end namespace Testing
