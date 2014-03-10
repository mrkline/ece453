#pragma once

#include <string>
#include <unordered_map>

#include <jsoncpp/json/json.h>

#include "Exceptions.hpp"

/// A Payload interface that provides copmile-time casting checks
class Payload {
public:

	/// Different types of message payloads.
	enum Type {
		PT_RESPONSE, ///< A generic response
		PT_SETUP, ///< Set up a game
		PT_START, ///< Start a game
		PT_STOP, ///< Stop a running game
		PT_STATUS, ///< Get the game status
		PT_STATUS_RESPONSE, ///< Response to a status query
		PT_RESULTS, ///< Get the game results after a match has finished
		PT_RESULTS_RESPONSE, ///< Response to a results query
		PT_UNKNOWN ///< An unknown/invalid payload type
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
