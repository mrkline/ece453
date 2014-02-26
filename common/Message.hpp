#pragma once

#include <memory>

#include "Payload.hpp"

class Message {

public:

	Message(Payload::Type t, int ver, int idNum, std::unique_ptr<Payload>&& load);

	/* TODO
	static std::unique_ptr<Message> fromJSON(const Json::Value& object);

	Json::Value toJSON() const;
	*/

	const Payload::Type type;

	const int version;

	const int id;

	const std::unique_ptr<Payload> payload;

};
