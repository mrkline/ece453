#pragma once

#include "TestUnit.hpp"

namespace Testing {

class SetupPayloadTests : public TestUnit {

public:

	SetupPayloadTests();

	const char* getUnitName() const override { return "SetupPayload"; }
};

} // end namespace Testing
