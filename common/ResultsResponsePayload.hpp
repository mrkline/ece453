#pragma once

#include <memory>
#include <vector>

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

		bool operator==(const Shot& o) const { return hit == o.hit && time == o.time && movement == o.movement; }
	};

	struct PlayerStats {
		const int score; ///< The player's final score
		const int hits; ///< The player's final hit count
		const std::vector<Shot> shots; ///< A list of shots the player took

		PlayerStats(int s, int h, std::vector<Shot>&& t) : score(s), hits(h), shots(t) { }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits && shots == o.shots; }
	};

	typedef std::vector<PlayerStats> StatsList;

	ResultsResponsePayload(int respTo, const std::string& message, StatsList&& playerStats);

	static std::unique_ptr<ResultsResponsePayload> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return PT_RESULTS_RESPONSE; }

	const StatsList stats;
};
