#include "SetupPayloadTests.hpp"

#include "SetupPayload.hpp"

namespace {

void serialize()
{
	SetupPayload load(GT_POP_UP, 2, WC_TIME, 60, -1, SetupPayload::DataMap());
	Json::Value jrep = load.toJSON();
	std::cout << jrep;

	auto fromJSON = SetupPayload::fromJSON(jrep);
	TEST_ASSERT(load == *fromJSON);
}

} // end anonymous namespace


Testing::SetupPayloadTests::SetupPayloadTests()
{
	add("Serialization", &serialize);
}
