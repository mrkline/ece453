#pragma once

#include <memory>
#include <vector>

#include "ResponseMessage.hpp"
#include "Shot.hpp"

class ResultsResponseMessage : public ResponseMessage {

public:

	struct PlayerStats {
		const score_t score; ///< The player's final score
		const shot_t hits; ///< The player's final hit count
		const std::vector<Shot> shots; ///< A list of shots the player took

		PlayerStats(score_t s, shot_t h, std::vector<Shot>&& t) :
			score(s),
			hits(h),
			shots(std::move(t))
		{ }

		bool operator==(const PlayerStats& o) const { return score == o.score && hits == o.hits && shots == o.shots; }
	};

	typedef std::vector<PlayerStats> StatsList;

	ResultsResponseMessage(message_id_t id, message_id_t respTo, const std::string& message, StatsList&& playerStats);

#ifdef WITH_JSON
	static std::unique_ptr<ResultsResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	Type getType() const override { return Type::RESULTS_RESPONSE; }

	bool operator==(const Message& o) const override;

	const StatsList stats;
};
