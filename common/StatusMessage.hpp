#pragma once

#include "Message.hpp"

class StatusMessage : public Message {

public:

	StatusMessage(uint16_t id) : Message(id) { }

	static std::unique_ptr<StatusMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StatusMessage>(new StatusMessage(Message::fromJSON(object)->id));
	}

	Type getType() const override { return Type::STATUS; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StatusMessage*>(&o) != nullptr;
	}
};
