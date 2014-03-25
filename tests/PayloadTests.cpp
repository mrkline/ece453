#include "PayloadTests.hpp"

#include <random>

#include "SetupPayload.hpp"
#include "ResponsePayload.hpp"
#include "StatusResponsePayload.hpp"
#include "ResultsResponsePayload.hpp"
#include "Message.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;

namespace {

unique_ptr<ResponsePayload> makeResponsePayload()
{
	return unique_ptr<ResponsePayload>(
		new ResponsePayload(42, ResponsePayload::ResponsePayload::Code::OK, "I am a response!"));
}

void ResponsePayloadJSON()
{
	auto load = makeResponsePayload();
	Json::Value jrep = load->toJSON();

	auto fromJSON = ResponsePayload::fromJSON(jrep);
	TEST_ASSERT(*load == *fromJSON);
}

unique_ptr<SetupPayload> makeSetupPayload()
{
	return unique_ptr<SetupPayload>(
		new SetupPayload(GameType::POP_UP, 2, WC_TIME, 60, -1, SetupPayload::DataMap()));
}

void SetupPayloadJSON()
{
	auto load = makeSetupPayload();
	Json::Value jrep = load->toJSON();

	auto fromJSON = SetupPayload::fromJSON(jrep);
	TEST_ASSERT(*load == *fromJSON);
}

unique_ptr<StatusResponsePayload> makeStatusResponsePayload()
{
	StatusResponsePayload::PlayerList stats;
	stats.emplace_back(2, 4);
	stats.emplace_back(25, 64);
	stats.emplace_back(33, 89);

	return unique_ptr<StatusResponsePayload>(
		new StatusResponsePayload(42, "I am a response!", true, 42, -1, move(stats)));
}

void StatusResponsePayloadJSON()
{
	auto load = makeStatusResponsePayload();
	Json::Value jrep = load->toJSON();

	auto fromJSON = StatusResponsePayload::fromJSON(jrep);
	TEST_ASSERT(*load == *fromJSON);
}

unique_ptr<ResultsResponsePayload> makeResultsResponsePayload()
{
	typedef ResultsResponsePayload::Shot Shot;
	typedef ResultsResponsePayload::PlayerStats PlayerStats;

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dis(-1, 1);
	auto genFloat = [&]() { return dis(gen); };

	vector<Vector3> fakeMovement;
	for (int i = 0; i < 10; ++i)
		fakeMovement.emplace_back(genFloat(), genFloat(), genFloat());

	Shot aShot(false, 240, move(fakeMovement));

	PlayerStats stat(20, 1, vector<Shot>({aShot}));

	return unique_ptr<ResultsResponsePayload>(
		new ResultsResponsePayload(21, "I'm some results!", vector<PlayerStats>({stat})));
}

void ResultsResponsePayloadJSON()
{
	auto load = makeResultsResponsePayload();
	Json::Value jrep = load->toJSON();

	auto fromJSON = ResultsResponsePayload::fromJSON(jrep);
	TEST_ASSERT(*load == *fromJSON);
}

void MessageJSON()
{
	Message response(Payload::Type::RESPONSE, 1, 1, makeResponsePayload());
	Json::Value jresponse = response.toJSON();
	auto responseFromJSON = Message::fromJSON(jresponse);
	TEST_ASSERT(response == *responseFromJSON);

	Message setup(Payload::Type::SETUP, 1, 2, makeSetupPayload());
	Json::Value jsetup = setup.toJSON();
	auto setupFromJSON = Message::fromJSON(jsetup);
	TEST_ASSERT(setup == *setupFromJSON);

	Message start(Payload::Type::START, 1, 3, nullptr);
	Json::Value jstart = start.toJSON();
	auto startFromJSON = Message::fromJSON(jstart);
	TEST_ASSERT(start == *startFromJSON);

	Message stop(Payload::Type::STOP, 1, 4, nullptr);
	Json::Value jstop = stop.toJSON();
	auto stopFromJSON = Message::fromJSON(jstop);
	TEST_ASSERT(stop == *stopFromJSON);

	Message status(Payload::Type::STATUS, 1, 5, nullptr);
	Json::Value jstatus = status.toJSON();
	auto statusFromJSON = Message::fromJSON(jstatus);
	TEST_ASSERT(status == *statusFromJSON);

	Message statusResponse(Payload::Type::STATUS_RESPONSE, 1, 6, makeStatusResponsePayload());
	Json::Value jstatusResponse = statusResponse.toJSON();
	auto statusResponseFromJSON = Message::fromJSON(jstatusResponse);
	TEST_ASSERT(statusResponse == *statusResponseFromJSON);

	Message results(Payload::Type::RESULTS, 1, 7, nullptr);
	Json::Value jresults = results.toJSON();
	auto resultsFromJSON = Message::fromJSON(jresults);
	TEST_ASSERT(results == *resultsFromJSON);

	Message resultsResponse(Payload::Type::RESULTS_RESPONSE, 1, 8, makeResultsResponsePayload());
	Json::Value jresultsResponse = resultsResponse.toJSON();
	auto resultsResponseFromJSON = Message::fromJSON(jresultsResponse);
	TEST_ASSERT(resultsResponse == *resultsResponseFromJSON);
}

} // end anonymous namespace


Testing::PayloadTests::PayloadTests()
{
	add("ResponsePayload -> JSON", &ResponsePayloadJSON);
	add("SetupPayload -> JSON", &SetupPayloadJSON);
	add("StatusResponsePayload -> JSON", &StatusResponsePayloadJSON);
	add("ResultsResponsePayload -> JSON", &ResultsResponsePayloadJSON);
	add("Messages -> JSON", &MessageJSON);
}
