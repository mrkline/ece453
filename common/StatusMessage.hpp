#pragma once

#include "Message.hpp"

class StatusMessage : public Message {

public:

	StatusMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	static std::unique_ptr<StatusMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StatusMessage>(new StatusMessage(Message::fromJSON(object)->id));
	}
#endif

	Type getType() const override { return Type::STATUS; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StatusMessage*>(&o) != nullptr;
	}
};
