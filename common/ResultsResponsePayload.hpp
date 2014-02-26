#pragma once

#include <memory>
#include <vector>

#include <jsoncpp/json/json.h>

#include "ResponsePayload.hpp"
#include "Vector3.hpp"

class ResultsResponsePayload : public ResponsePayload {

public:

	struct Shot {
		const bool hit; ///< True if the shot was a hit
		const int time; ///< The timestamp of the shot, in milliseconds since the game started
		// A vector of vectors. Surely this will cause no confusion.
		const std::vector<Vector3> movement; ///< Provides accelerometer data leading up to the shot

		Shot(bool h, int t, std::vector<Vector3>&& m) : hit(h), time(t), movement(m) { }
	};

	struct PlayerStats {
		const int score; ///< The player's final score
		const int hits; ///< The player's final hit count
		const std::vector<Shot> shots; ///< A list of shots the player took

		PlayerStats(int s, int h, std::vector<Shot>&& t) : score(s), hits(h), shots(t) { }
	};

	typedef std::vector<PlayerStats> StatsList;

	const StatsList stats;

	ResultsResponsePayload(int respTo, const std::string& message, StatsList&& playerStats);

	static std::unique_ptr<ResultsResponsePayload> fromJSON(const Json::Value& object);
};
