#pragma once

#include <vector>

#include "ResponsePayload.hpp"
#include "Vector3.hpp"

class ResultsResponsePayload : public ResponsePayload {

public:

	struct Shot {
		bool hit; ///< True if the shot was a hit
		int time; ///< The timestamp of the shot, in milliseconds since the game started
		// A vector of vectors. Surely this will cause no confusion.
		std::vector<Vector3> movement; ///< Provides accelerometer data leading up to the shot
	};

	struct PlayerStats {
		int score;
		int hits;
		std::vector<Shot> shots;
	};

	typedef std::vector<PlayerStats> StatsList;

	const StatsList stats;

	ResultsResponsePayload(int respTo, StatsList&& playerStats);
};
