#include "MessageTests.hpp"

#include <random>

#include "SetupMessage.hpp"
#include "ResponseMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "Message.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;

namespace {

unique_ptr<ResponseMessage> makeResponseMessage()
{
	return unique_ptr<ResponseMessage>(
		new ResponseMessage(42, 25, ResponseMessage::ResponseMessage::Code::OK, "I am a response!"));
}

void ResponseMessageJSON()
{
	auto load = makeResponseMessage();
	Json::Value jrep = load->toJSON();

	auto fromJSON = JSONToMessage(jrep);
	TEST_ASSERT(fromJSON->getType() == load->getType());
	TEST_ASSERT(*load == *fromJSON);
}

unique_ptr<SetupMessage> makeSetupMessage()
{
	return unique_ptr<SetupMessage>(
		new SetupMessage(42, GameType::POP_UP, 2, WC_TIME, 60, -1, SetupMessage::DataMap()));
}

void SetupMessageJSON()
{
	auto load = makeSetupMessage();
	Json::Value jrep = load->toJSON();

	auto fromJSON = JSONToMessage(jrep);
	TEST_ASSERT(fromJSON->getType() == load->getType());
	TEST_ASSERT(*load == *fromJSON);
}

unique_ptr<StatusResponseMessage> makeStatusResponseMessage()
{
	StatusResponseMessage::PlayerList stats;
	stats.emplace_back(2, 4);
	stats.emplace_back(25, 64);
	stats.emplace_back(33, 89);

	return unique_ptr<StatusResponseMessage>(
		new StatusResponseMessage(42, 56, "I am a response!", true, 42, -1, move(stats)));
}

void StatusResponseMessageJSON()
{
	auto load = makeStatusResponseMessage();
	Json::Value jrep = load->toJSON();

	auto fromJSON = JSONToMessage(jrep);
	TEST_ASSERT(fromJSON->getType() == load->getType());
	TEST_ASSERT(*load == *fromJSON);
}

unique_ptr<ResultsResponseMessage> makeResultsResponseMessage()
{
	typedef ResultsResponseMessage::Shot Shot;
	typedef ResultsResponseMessage::PlayerStats PlayerStats;

	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dis(-1, 1);
	auto genFloat = [&]() { return dis(gen); };

	vector<Vector3> fakeMovement;
	for (int i = 0; i < 10; ++i)
		fakeMovement.emplace_back(genFloat(), genFloat(), genFloat());

	Shot aShot(false, 240, move(fakeMovement));

	PlayerStats stat(20, 1, vector<Shot>({aShot}));

	return unique_ptr<ResultsResponseMessage>(
		new ResultsResponseMessage(42, 21, "I'm some results!", vector<PlayerStats>({stat})));
}

void ResultsResponseMessageJSON()
{
	auto load = makeResultsResponseMessage();
	Json::Value jrep = load->toJSON();

	auto fromJSON = JSONToMessage(jrep);
	TEST_ASSERT(fromJSON->getType() == load->getType());
	TEST_ASSERT(*load == *fromJSON);
}

} // end anonymous namespace


Testing::MessageTests::MessageTests()
{
	add("ResponseMessage -> JSON", &ResponseMessageJSON);
	add("SetupMessage -> JSON", &SetupMessageJSON);
	add("StatusResponseMessage -> JSON", &StatusResponseMessageJSON);
	add("ResultsResponseMessage -> JSON", &ResultsResponseMessageJSON);
}
