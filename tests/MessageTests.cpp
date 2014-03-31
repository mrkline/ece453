#include "MessageTests.hpp"

#include <random>

#include "ResponseMessage.hpp"
#include "SetupMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "ExitMessage.hpp"
#include "Message.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;

namespace {

template <typename M = Message>
unique_ptr<M> makeMessage()
{
	return unique_ptr<M>(new M(42));
}

void check(const unique_ptr<Message>& load, Message::Type t)
{
	Json::Value jrep = load->toJSON();

	auto fromJSON = JSONToMessage(jrep);
	assert(fromJSON->getType() == load->getType());
	assert(load->getType() == t); // To make sure we don't forget to override getType()
	assert(*load == *fromJSON);
}

unique_ptr<ResponseMessage> makeResponseMessage()
{
	return unique_ptr<ResponseMessage>(
		new ResponseMessage(42, 25, ResponseMessage::ResponseMessage::Code::OK, "I am a response!"));
}

unique_ptr<SetupMessage> makeSetupMessage()
{
	return unique_ptr<SetupMessage>(
		new SetupMessage(42, GameType::POP_UP, 2, WC_TIME, 60, -1, SetupMessage::DataMap()));
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

} // end anonymous namespace

Testing::MessageTests::MessageTests()
{
	using Type = Message::Type;

	add("Message -> JSON", []{ check(makeMessage(), Type::EMPTY); });
	add("ResponseMessage -> JSON", []{ check(makeResponseMessage(), Type::RESPONSE); });
	add("SetupMessage -> JSON", []{ check(makeSetupMessage(), Type::SETUP); });
	add("StartMessage -> JSON", []{ check(makeMessage<StartMessage>(), Type::START); });
	add("StopMessage -> JSON", []{ check(makeMessage<StopMessage>(), Type::STOP); });
	add("StatusMessage -> JSON", []{ check(makeMessage<StatusMessage>(), Type::STATUS); });
	add("StatusResponseMessage -> JSON", []{ check(makeStatusResponseMessage(), Type::STATUS_RESPONSE); });
	add("ResultsMessage -> JSON", []{ check(makeMessage<ResultsMessage>(), Type::RESULTS); });
	add("ResultsResponseMessage -> JSON", []{ check(makeResultsResponseMessage(), Type::RESULTS_RESPONSE); });
	add("ExitMessage -> JSON", []{ check(makeMessage<ExitMessage>(), Type::EXIT); });
}
