#pragma once

#include "Message.hpp"

/// Indicates that the receiver should finish/exit.
/// Not used with the boards, but internally with the state machine and UI
class ExitMessage : public Message {
public:

	ExitMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<ExitMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<ExitMessage>(new ExitMessage(Message::fromJSON(object)->id));
	}
#endif

	/// The ExitMessage is not serializable to binary
	/// \throws Exceptions::InvalidOperationException upon being called
	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	Type getType() const override { return Type::EXIT; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const ExitMessage*>(&o) != nullptr;
	}
};
