#pragma once

#include "Message.hpp"

class ExitMessage : public Message {
public:

	ExitMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	static std::unique_ptr<ExitMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<ExitMessage>(new ExitMessage(Message::fromJSON(object)->id));
	}
#endif

	Type getType() const override { return Type::EXIT; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const ExitMessage*>(&o) != nullptr;
	}
};
