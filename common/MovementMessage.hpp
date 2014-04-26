#pragma once

#include <memory>

#include "Message.hpp"
#include "Shot.hpp"

/// A message indicating the arrival of a shot
class MovementMessage : public Message {

public:

	MovementMessage(uint16_t id, ShotWithMovement&& m);

#ifdef WITH_JSON
	static std::unique_ptr<MovementMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	Type getType() const override { return Type::MOVEMENT; }

	bool operator==(const Message& o) const override;

	/// We carry the shot, not just the movement,
	/// as we use the shot info as a unique ID for the movement,
	/// matching it to our already recorded shot.
	ShotWithMovement movement;
};
