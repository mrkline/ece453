#include "MessageTests.hpp"

#include <random>

#include "Test.hpp"
#include "ResponseMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "StatusMessage.hpp"
#include "ResultsMessage.hpp"
#include "ExitMessage.hpp"

using namespace Exceptions;
using namespace Json;
using namespace std;


namespace {

void JSONCheck(const unique_ptr<Message>& load, Message::Type t)
{
	Json::Value jrep = load->toJSON();

	auto fromJSON = JSONToMessage(jrep);
	assert(fromJSON->getType() == load->getType());
	assert(load->getType() == t); // To make sure we don't forget to override getType()
	assert(*load == *fromJSON);
}

void binaryCheck(const unique_ptr<Message>& load, Message::Type t)
{
	vector<uint8_t> buf = load->toBinary();

	auto fromBinary = binaryToMessage(buf.data(), buf.size());
	assert(fromBinary->getType() == load->getType());
	assert(load->getType() == t);
	assert(*load == *fromBinary);
}

} // end anonymous namespace

namespace Testing {

std::unique_ptr<ResponseMessage> makeResponseMessage()
{
	return unique_ptr<ResponseMessage>(
		new ResponseMessage(0, 25, ResponseMessage::ResponseMessage::Code::OK, "I am a response!"));
}

std::unique_ptr<SetupMessage> makeSetupMessage(int16_t gameLength, int16_t maxScore)
{
	return unique_ptr<SetupMessage>(
		new SetupMessage(0, GameType::POP_UP, 2, gameLength, maxScore, SetupMessage::DataMap()));
}

std::unique_ptr<StatusResponseMessage> makeStatusResponseMessage()
{
	StatusResponseMessage::PlayerList stats;
	stats.emplace_back(2, 4);
	stats.emplace_back(25, 64);
	stats.emplace_back(33, 89);

	return unique_ptr<StatusResponseMessage>(
		new StatusResponseMessage(0, 56, "I am a response!", true, 42, -1, move(stats)));
}


std::unique_ptr<ResultsResponseMessage> makeResultsResponseMessage()
{
	typedef ResultsResponseMessage::PlayerStats PlayerStats;

	PlayerStats stat(20, 1, vector<Shot>({ Shot(2, 4, 240) }));

	return unique_ptr<ResultsResponseMessage>(
		new ResultsResponseMessage(0, 21, "I'm some results!", vector<PlayerStats>({stat})));
}

std::unique_ptr<ShotMessage> makeShotMessage()
{
	return unique_ptr<ShotMessage>(
		new ShotMessage(0, Shot(2, 4, 240)));
}

std::unique_ptr<TargetControlMessage> makeTargetControlMessage()
{
	return unique_ptr<TargetControlMessage>(
		new TargetControlMessage(0, TargetControlMessage::CommandList({TargetCommand(1, true),
		                                                                TargetCommand(3, false)})));
}

void MessageTests()
{
	using Type = Message::Type;

	beginUnit("Message");
	test("Message -> JSON", []{ JSONCheck(makeMessage(), Type::EMPTY); });
	test("ResponseMessage -> JSON", []{ JSONCheck(makeResponseMessage(), Type::RESPONSE); });
	test("SetupMessage -> JSON", []{ JSONCheck(makeSetupMessage(), Type::SETUP); });
	test("StartMessage -> JSON", []{ JSONCheck(makeMessage<StartMessage>(), Type::START); });
	test("StopMessage -> JSON", []{ JSONCheck(makeMessage<StopMessage>(), Type::STOP); });
	test("StatusMessage -> JSON", []{ JSONCheck(makeMessage<StatusMessage>(), Type::STATUS); });
	test("StatusResponseMessage -> JSON", []{ JSONCheck(makeStatusResponseMessage(), Type::STATUS_RESPONSE); });
	test("ResultsMessage -> JSON", []{ JSONCheck(makeMessage<ResultsMessage>(), Type::RESULTS); });
	test("ResultsResponseMessage -> JSON", []{ JSONCheck(makeResultsResponseMessage(), Type::RESULTS_RESPONSE); });
	test("ShotMessage -> JSON", []{ JSONCheck(makeShotMessage(), Type::SHOT); });
	test("TargetControlMessage -> JSON", [] { JSONCheck(makeTargetControlMessage(), Type::TARGET_CONTROL); });
	test("ExitMessage -> JSON", []{ JSONCheck(makeMessage<ExitMessage>(), Type::EXIT); });


	test("Message -> Binary", []{ binaryCheck(makeMessage(), Type::EMPTY); });
	test("ResponseMessage -> Binary", []{ binaryCheck(makeResponseMessage(), Type::RESPONSE); });
	test("StartMessage -> Binary", []{ binaryCheck(makeMessage<StartMessage>(), Type::START); });
	test("StopMessage -> Binary", []{ binaryCheck(makeMessage<StopMessage>(), Type::STOP); });
	test("ShotMessage -> Binary", []{ binaryCheck(makeShotMessage(), Type::SHOT); });
	test("TargetControlMessage -> Binary", [] { binaryCheck(makeTargetControlMessage(), Type::TARGET_CONTROL); });
}

} // end namespace Testing
