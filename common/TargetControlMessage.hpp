#pragma once

#include <vector>

#include "Message.hpp"

/**
 * \brief A command to send a target to turn its LEDs on and off
 * \todo If time permits, we will add color to this command
 */
struct TargetCommand {
	board_id_t id; ///< The ID of the target
	bool on; ///< true if the LEDs should be on

	TargetCommand(board_id_t i, bool o) : id(i), on(o) { }

#ifdef WITH_JSON
	Json::Value toJSON() const;

	static TargetCommand fromJSON(const Json::Value& object);
#endif

	bool operator==(const TargetCommand& o) const { return id == o.id && on == o.on; }
};

/// A message sent by the state machine while the game is running to control the targets
class TargetControlMessage : public Message {

public:


	typedef std::vector<TargetCommand> CommandList;

	TargetControlMessage(message_id_t id, CommandList&& comms);

	TargetControlMessage(message_id_t id, const TargetCommand& comm);

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<TargetControlMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<TargetControlMessage> fromBinary(uint8_t* buf, size_t len);

	/**
	 * \brief Gets the target control message's binary payload
	 *
	 * A target control message has the following payload:
	 * - One unsigned byte indicating the number of commands to follow
	 * - For each command:
	 *   - One byte for the target ID the command pertains to
	 *   - One byte indicating whether or not the target's lights should be on.
	 */
	std::vector<uint8_t> getBinaryPayload() const override;

	Type getType() const override { return Type::TARGET_CONTROL; }

	bool operator==(const Message& o) const override;

	const CommandList commands;
};
