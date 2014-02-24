#pragma once

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

	virtual Type getType() const { return PT_UNKNOWN; }
};
