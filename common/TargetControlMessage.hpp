#pragma once

#include <vector>

#include "Message.hpp"

struct TargetCommand {
	int8_t id; ///< The ID of the target
	bool on; ///< true if the LEDs should be on

	TargetCommand(int8_t i, bool o) : id(i), on(o) { }

	Json::Value toJSON() const;

	static TargetCommand fromJSON(const Json::Value& object);

	bool operator==(const TargetCommand& o) const { return id == o.id && on == o.on; }
};

/// A message sent by the state machine while the game is running to control the targets
class TargetControlMessage : public Message {

public:


	typedef std::vector<TargetCommand> CommandList;

	TargetControlMessage(uint16_t id, CommandList&& comms);

	TargetControlMessage(uint16_t id, const TargetCommand& comm);

	static std::unique_ptr<TargetControlMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return Type::TARGET_CONTROL; }

	bool operator==(const Message& o) const override;

	const CommandList commands;
};
