#pragma once

#include "Message.hpp"

class ResultsMessage : public Message {

public:

	ResultsMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	static std::unique_ptr<ResultsMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<ResultsMessage>(new ResultsMessage(Message::fromJSON(object)->id));
	}
#endif

	Type getType() const override { return Type::RESULTS; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const ResultsMessage*>(&o) != nullptr;
	}
};
