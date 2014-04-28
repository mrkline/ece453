#pragma once

#include "Message.hpp"

class StartMessage : public Message {

public:

	StartMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	static std::unique_ptr<StartMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StartMessage>(new StartMessage(Message::fromJSON(object)->id));
	}
#endif

	static std::unique_ptr<StartMessage> fromBinary(uint8_t* buf, size_t len)
	{
		return std::unique_ptr<StartMessage>(new StartMessage(Message::fromBinary(buf, len)->id));
	}

	Type getType() const override { return Type::START; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StartMessage*>(&o) != nullptr;
	}
};
