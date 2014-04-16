#include "MessageTests.hpp"

#include <random>

#include "Test.hpp"
#include "ResponseMessage.hpp"
#include "SetupMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "ShotMessage.hpp"
#include "MovementMessage.hpp"
#include "TargetControlMessage.hpp"
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

Movement makeMovement()
{
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<float> dis(-1, 1);
	auto genFloat = [&]() { return dis(gen); };

	Movement fakeMovement;
	for (int i = 0; i < 10; ++i)
		fakeMovement.emplace_back(genFloat(), genFloat(), genFloat());

	return fakeMovement;
}

unique_ptr<ResultsResponseMessage> makeResultsResponseMessage()
{
	typedef ResultsResponseMessage::PlayerStats PlayerStats;


	ShotWithMovement aShot(2, 4, 240, makeMovement());

	PlayerStats stat(20, 1, vector<ShotWithMovement>({aShot}));

	return unique_ptr<ResultsResponseMessage>(
		new ResultsResponseMessage(42, 21, "I'm some results!", vector<PlayerStats>({stat})));
}

unique_ptr<ShotMessage> makeShotMessage()
{
	return unique_ptr<ShotMessage>(
		new ShotMessage(42, Shot(2, 4, 240)));
}

unique_ptr<MovementMessage> makeMovementMessage()
{
	return unique_ptr<MovementMessage>(
		new MovementMessage(42, makeMovement()));
}

unique_ptr<TargetControlMessage> makeTargetControlMessage()
{
	return unique_ptr<TargetControlMessage>(
		new TargetControlMessage(42, TargetControlMessage::CommandList({TargetCommand(1, true),
		                                                                TargetCommand(3, false)})));
}

} // end anonymous namespace

void Testing::MessageTests()
{
	using Type = Message::Type;

	beginUnit("Message");
	test("Message -> JSON", []{ check(makeMessage(), Type::EMPTY); });
	test("ResponseMessage -> JSON", []{ check(makeResponseMessage(), Type::RESPONSE); });
	test("SetupMessage -> JSON", []{ check(makeSetupMessage(), Type::SETUP); });
	test("StartMessage -> JSON", []{ check(makeMessage<StartMessage>(), Type::START); });
	test("StopMessage -> JSON", []{ check(makeMessage<StopMessage>(), Type::STOP); });
	test("StatusMessage -> JSON", []{ check(makeMessage<StatusMessage>(), Type::STATUS); });
	test("StatusResponseMessage -> JSON", []{ check(makeStatusResponseMessage(), Type::STATUS_RESPONSE); });
	test("ResultsMessage -> JSON", []{ check(makeMessage<ResultsMessage>(), Type::RESULTS); });
	test("ResultsResponseMessage -> JSON", []{ check(makeResultsResponseMessage(), Type::RESULTS_RESPONSE); });
	test("ShotMessage -> JSON", []{ check(makeShotMessage(), Type::SHOT); });
	test("MovementMessage -> JSON", [] { check(makeMovementMessage(), Type::MOVEMENT); });
	test("TargetControlMessage -> JSON", [] { check(makeTargetControlMessage(), Type::TARGET_CONTROL); });
	test("ExitMessage -> JSON", []{ check(makeMessage<ExitMessage>(), Type::EXIT); });
}
