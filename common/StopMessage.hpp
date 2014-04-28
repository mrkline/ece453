#pragma once

#include "Message.hpp"

class StopMessage : public Message {

public:

	StopMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	static std::unique_ptr<StopMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StopMessage>(new StopMessage(Message::fromJSON(object)->id));
	}
#endif

	static std::unique_ptr<StopMessage> fromBinary(uint8_t* buf, size_t len)
	{
		return std::unique_ptr<StopMessage>(new StopMessage(Message::fromBinary(buf, len)->id));
	}

	Type getType() const override { return Type::STOP; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StopMessage*>(&o) != nullptr;
	}
};
