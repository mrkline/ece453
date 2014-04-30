#pragma once

#include "Message.hpp"

/// A request to get the results of a match.
/// This can only be requested after a match is over.
class ResultsMessage : public Message {

public:

	ResultsMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<ResultsMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<ResultsMessage>(new ResultsMessage(Message::fromJSON(object)->id));
	}
#endif

	/// The ResultsMessage is not serializable to binary
	/// \throws Exceptions::InvalidOperationException upon being called
	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	Type getType() const override { return Type::RESULTS; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const ResultsMessage*>(&o) != nullptr;
	}
};
