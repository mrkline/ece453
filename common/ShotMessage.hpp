#pragma once

#include <memory>

#include "Message.hpp"
#include "Shot.hpp"

/// A message indicating the arrival of a shot
class ShotMessage : public Message {

public:

	ShotMessage(message_id_t id, const Shot& s);

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<ShotMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<ShotMessage> fromBinary(uint8_t* buf, size_t len);

	/*
	 * \brief Gets the shot message's binary payload
	 *
	 * A shot message has the following payload:
	 * - One signed byte for the player/gun that fired the shot
	 * - One signed byte for the target that was hit
	 *   (or -1 if the message is coming from a gun)
	 * - A 32-bit signed integer representing the timestamp of the shot,
	 *   in milliseconds since the game started.
	 *
	 * \see Shot::toBinary
	 */
	std::vector<uint8_t> getBinaryPayload() const override;

	Type getType() const override { return Type::SHOT; }

	bool operator==(const Message& o) const override;

	Shot shot;
};
