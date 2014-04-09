#pragma once

#include <memory>

#include <jsoncpp/json/json.h>

class Message {

public:

	/// Different types of message payloads.
	enum class Type {
		EMPTY, ///< A message containing nothing but an ID
		RESPONSE, ///< A generic response
		SETUP, ///< Set up a game
		START, ///< Start a game
		STOP, ///< Stop a running game
		STATUS, ///< Get the game status
		STATUS_RESPONSE, ///< Response to a status query
		RESULTS, ///< Get the game results after a match has finished
		RESULTS_RESPONSE, ///< Response to a results query
		SHOT, ///< Represents a shot fired
		MOVEMENT, ///< Carries the motion of a previously-reported shot
		EXIT, ///< The entity receiving this message should exit/finish
		TEST, ///< A test payload that holds a string
		UNKNOWN ///< An unknown/invalid payload type
	};

	Message(int idNum);

	static std::unique_ptr<Message> fromJSON(const Json::Value& object);

	const static Json::StaticString typeKey;

	virtual Json::Value toJSON() const;

	virtual Type getType() const { return Type::EMPTY; }

	const int id;

	virtual bool operator==(const Message& o) const;
};

namespace std
{
	// Hasher for Message::Type
	template<>
	struct hash<Message::Type> {
		size_t operator()(Message::Type t) const { return std::hash<int>()((int)t); }
	};
}

std::unique_ptr<Message> JSONToMessage(const Json::Value& object);
