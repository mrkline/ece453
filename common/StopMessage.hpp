#pragma once

#include "Message.hpp"

class StopMessage : public Message {

public:

	StopMessage(int id) : Message(id) { }

	static std::unique_ptr<StopMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StopMessage>(new StopMessage(Message::fromJSON(object)->id));
	}

	Type getType() const override { return Type::STOP; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StopMessage*>(&o) != nullptr;
	}
};