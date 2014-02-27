#include "PayloadTests.hpp"

#include "SetupPayload.hpp"
#include "ResponsePayload.hpp"
#include "StatusResponsePayload.hpp"

namespace {

void SetupPayloadJSON()
{
	SetupPayload load(GT_POP_UP, 2, WC_TIME, 60, -1, SetupPayload::DataMap());
	Json::Value jrep = load.toJSON();
	std::cout << jrep;

	auto fromJSON = SetupPayload::fromJSON(jrep);
	TEST_ASSERT(load == *fromJSON);
}

void ResponsePayloadJSON()
{
	ResponsePayload load(42, ResponsePayload::RC_OK, "I am a response!");
	Json::Value jrep = load.toJSON();
	std::cout <<jrep;

	auto fromJSON = ResponsePayload::fromJSON(jrep);
	TEST_ASSERT(load == *fromJSON);
}

void StatusResponsePayloadJSON()
{
	StatusResponsePayload::PlayerList stats;
	stats.emplace_back(2, 4);
	stats.emplace_back(25, 64);
	stats.emplace_back(33, 89);


	StatusResponsePayload load(42, "I am a response!", true, 42, -1, std::move(stats));
	Json::Value jrep = load.toJSON();
	std::cout <<jrep;

	auto fromJSON = StatusResponsePayload::fromJSON(jrep);
	TEST_ASSERT(load == *fromJSON);
}

} // end anonymous namespace


Testing::PayloadTests::PayloadTests()
{
	add("SetupPayload -> JSON", &SetupPayloadJSON);
	add("ResponsePayload -> JSON", &ResponsePayloadJSON);
	add("StatusResponsePayload -> JSON", &StatusResponsePayloadJSON);
}
