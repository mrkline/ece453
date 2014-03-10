#pragma once

#include <string>
#include <unordered_map>

#include <jsoncpp/json/json.h>

#include "Exceptions.hpp"

/// A Payload interface that provides copmile-time casting checks
class Payload {
public:

	/// Different types of message payloads.
	enum class Type {
		RESPONSE, ///< A generic response
		SETUP, ///< Set up a game
		START, ///< Start a game
		STOP, ///< Stop a running game
		STATUS, ///< Get the game status
		STATUS_RESPONSE, ///< Response to a status query
		RESULTS, ///< Get the game results after a match has finished
		RESULTS_RESPONSE, ///< Response to a results query
		UNKNOWN ///< An unknown/invalid payload type
	};

	static Type nameToType(const std::string& name);

	static Json::StaticString typeToName(Type t);

	virtual Type getType() const = 0;

	virtual Json::Value toJSON() const { return Json::Value(); }

private:

	const static std::unordered_map<Type, Json::StaticString> typeNames;

	const static std::unordered_map<std::string, Type> typeLookup; ///< \todo Rename me?
};

namespace std
{
	// Hasher for Payload::Type
	template<>
	struct hash<Payload::Type> {
		size_t operator()(Payload::Type t) const { return std::hash<int>()((int)t); }
	};
}
