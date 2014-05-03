#pragma once

#include "Message.hpp"
#include "Exceptions.hpp"

/**
 * \brief Sent to query the current status of the system
 *
 * The game need not be running for this to be sent -
 * the game state machine will always respond.
 */
class StatusMessage : public Message {

public:

	StatusMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<StatusMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StatusMessage>(new StatusMessage(Message::fromJSON(object)->id));
	}
#endif

	/// The StatusMessage is not serializable to binary
	/// \throws Exceptions::InvalidOperationException upon being called
	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	Type getType() const override { return Type::STATUS; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StatusMessage*>(&o) != nullptr;
	}
};
