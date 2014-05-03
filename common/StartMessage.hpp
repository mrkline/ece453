#pragma once

#include "Message.hpp"

/**
 * \brief Sent to start a game.
 *
 * Note that a SetupMessage must have already been successfully sent before this is called.
 */
class StartMessage : public Message {

public:

	StartMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<StartMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StartMessage>(new StartMessage(Message::fromJSON(object)->id));
	}
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<StartMessage> fromBinary(uint8_t* buf, size_t len)
	{
		return std::unique_ptr<StartMessage>(new StartMessage(Message::fromBinary(buf, len)->id));
	}

	Type getType() const override { return Type::START; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StartMessage*>(&o) != nullptr;
	}
};
