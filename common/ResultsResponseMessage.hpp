#pragma once

#include <memory>
#include <vector>

#include "Exceptions.hpp"
#include "ResponseMessage.hpp"
#include "Shot.hpp"

/// Responds to a ResultsMessage message with the end-of-match results,
/// provided the ResultsMessage was sent after a match ended.
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

	/**
	 * \brief Constructs a results response message
	 * \param id The message ID
	 * \param respTo The ID of the ResultsMessage we are responding to
	 * \param message The message (if any) for the response
	 * \param playerStats A list of ResultsResponseMessage::PlayerStats to send
	 */
	ResultsResponseMessage(message_id_t id, message_id_t respTo, const std::string& message,
	                       StatsList&& playerStats);

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<ResultsResponseMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	/// The ResultsResponseMessage is not serializable to binary
	/// \throws Exceptions::InvalidOperationException upon being called
	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	Type getType() const override { return Type::RESULTS_RESPONSE; }

	bool operator==(const Message& o) const override;

	const StatsList stats;
};
