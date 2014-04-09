#pragma once

#include <memory>

#include "Message.hpp"
#include "Shot.hpp"

/// A message indicating the arrival of a shot
class ShotMessage : public Message {

public:

	ShotMessage(int id, const Shot& s);

	static std::unique_ptr<ShotMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return Type::SHOT; }

	bool operator==(const Message& o) const override;

	Shot shot;
};