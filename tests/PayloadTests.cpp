#include "PayloadTests.hpp"

#include <random>

#include "SetupPayload.hpp"
#include "ResponsePayload.hpp"
#include "StatusResponsePayload.hpp"
#include "ResultsResponsePayload.hpp"

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

void ResultsResponsePayloadJSON()
{
	typedef ResultsResponsePayload::Shot Shot;
	typedef ResultsResponsePayload::PlayerStats PlayerStats;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(-1, 1);
	auto genFloat = [&]() { return dis(gen); };

	std::vector<Vector3> fakeMovement;
	for (int i = 0; i < 10; ++i)
		fakeMovement.emplace_back(genFloat(), genFloat(), genFloat());

	Shot aShot(false, 240, std::move(fakeMovement));

	PlayerStats stat(20, 1, std::vector<Shot>({aShot}));

	ResultsResponsePayload load(21, "I'm some results!", std::vector<PlayerStats>({stat}));
	Json::Value jrep = load.toJSON();
	std::cout <<jrep;

	auto fromJSON = ResultsResponsePayload::fromJSON(jrep);
	TEST_ASSERT(load == *fromJSON);
}

} // end anonymous namespace


Testing::PayloadTests::PayloadTests()
{
	add("SetupPayload -> JSON", &SetupPayloadJSON);
	add("ResponsePayload -> JSON", &ResponsePayloadJSON);
	add("StatusResponsePayload -> JSON", &StatusResponsePayloadJSON);
	add("ResultsResponsePayload -> JSON", &ResultsResponsePayloadJSON);
}
