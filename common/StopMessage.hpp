#pragma once

#include "Message.hpp"

/**
 * \brief Sent to stop a game.
 *
 * Note that the game must be running before this is called.
 */
class StopMessage : public Message {

public:

	StopMessage(message_id_t id) : Message(id) { }

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<StopMessage> fromJSON(const Json::Value& object)
	{
		return std::unique_ptr<StopMessage>(new StopMessage(Message::fromJSON(object)->id));
	}
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<StopMessage> fromBinary(uint8_t* buf, size_t len)
	{
		return std::unique_ptr<StopMessage>(new StopMessage(Message::fromBinary(buf, len)->id));
	}

	Type getType() const override { return Type::STOP; }

	bool operator==(const Message& o) const override
	{
		return Message::operator==(o) && dynamic_cast<const StopMessage*>(&o) != nullptr;
	}
};
