#pragma once

#include <memory>

#include "Message.hpp"
#include "SetupMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ShotMessage.hpp"
#include "TargetControlMessage.hpp"

namespace Testing {

void MessageTests();

template <typename M = Message>
std::unique_ptr<M> makeMessage()
{
	return std::unique_ptr<M>(new M(0));
}

std::unique_ptr<ResponseMessage> makeResponseMessage();

std::unique_ptr<SetupMessage> makeSetupMessage(int16_t gameLength = 30, int16_t maxScore = -1);

std::unique_ptr<StatusResponseMessage> makeStatusResponseMessage();

std::unique_ptr<ResultsResponseMessage> makeResultsResponseMessage();

std::unique_ptr<ShotMessage> makeShotMessage();

std::unique_ptr<TargetControlMessage> makeTargetControlMessage();

} // end namespace Testing
