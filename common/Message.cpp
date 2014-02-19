#include "Message.hpp"

#include <cassert>

#include "Exceptions.hpp"

using namespace Exceptions;

Message::Message(Payload::Type t, int ver, int idNum, std::unique_ptr<Payload>&& load) :
	type(t),
	version(ver),
	id(idNum),
	payload(std::move(load))
{
	enforce<ArgumentException>(version > 0, "The version number cannot be negative.", __FUNCTION__);
	enforce<ArgumentException>(id > 0, "The ID cannot be negative.", __FUNCTION__);
	enforce<ArgumentException>(load == nullptr || type == load->getType(), "The type must match the payload's type.",
	                           __FUNCTION__);
}
