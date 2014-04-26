#pragma once

#include <memory>
#include <vector>

#include "ResponseMessage.hpp"
#include "Shot.hpp"

class ResultsResponseMessage : public ResponseMessage {

public:

	struct PlayerStats {
		const int16_t score; ///< The player's final score
		const int16_t hits; ///< The player's final hit count
		const std::vector<ShotWithMovement> shots; ///< A list of shots the player took

		PlayerStats(int16_t s, int16_t h, std::vector<ShotWithMovement>&& t) :
			score(s),
			hits(h),
			shots(std::move(t))
		{ }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits && shots == o.shots; }
	};

	typedef std::vector<PlayerStats> StatsList;

	ResultsResponseMessage(uint16_t id, uint16_t respTo, const std::string& message, StatsList&& playerStats);

#ifdef WITH_JSON
	static std::unique_ptr<ResultsResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	Type getType() const override { return Type::RESULTS_RESPONSE; }

	bool operator==(const Message& o) const override;

	const StatsList stats;
};
