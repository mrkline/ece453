#pragma once

#include "Message.hpp"

class ExitMessage : public Message {
public:

	ExitMessage(uint16_t id) : Message(id) { }

	static std::unique_ptr<ExitMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<ExitMessage>(new ExitMessage(Message::fromJSON(object)->id));
	}

	Type getType() const override { return Type::EXIT; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const ExitMessage*>(&o) != nullptr;
	}
};
