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

	Type getType() const override { return Type::START; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StartMessage*>(&o) != nullptr;
	}
};
