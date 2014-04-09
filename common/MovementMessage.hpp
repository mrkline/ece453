#pragma once

#include <memory>

#include "Message.hpp"
#include "Shot.hpp"

/// A message indicating the arrival of a shot
class MovementMessage : public Message {

public:

	MovementMessage(int id, Movement&& m);

	static std::unique_ptr<MovementMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return Type::MOVEMENT; }

	bool operator==(const Message& o) const override;

	Movement movement;
};
