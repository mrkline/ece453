#include "ResponsePayload.hpp"

#include "Exceptions.hpp"

using namespace Exceptions;

ResponsePayload::ResponsePayload(int respTo, Code c, const std::string& msg) :
	respondingTo(respTo),
	code(c),
	message(msg)
{
	enforce<ArgumentException>(respondingTo > 0, "Responding to a negative ID does not make sense.", __FUNCTION__);
}
