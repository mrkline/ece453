#include "ResultsResponseMessage.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;
using namespace Json;

namespace {

// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString playerStatsKey("player stats");
const StaticString scoreKey("score");
const StaticString hitsKey("hits");
const StaticString shotsKey("shots");

// For laziness
typedef ResultsResponseMessage::PlayerStats PlayerStats;

std::vector<ShotWithMovement> parseShots(const Value& shots)
{
	std::vector<ShotWithMovement> ret;

	for (const Value& shot : shots)
		ret.emplace_back(ShotWithMovement::fromJSON(shot));

	return ret;
}

PlayerStats parseStats(const Value& stat)
{
	ENFORCE(IOException, stat.isMember(scoreKey), "A player results set is missing its score.");
	ENFORCE(IOException, stat.isMember(hitsKey), "A player results set is missing its hit count.");
	ENFORCE(IOException, stat.isMember(shotsKey), "A player results set is missing its shots list.");

	const Value& scoreValue = stat[scoreKey];
	const Value& hitsValue = stat[hitsKey];
	const Value& shotsValue = stat[shotsKey];

	ENFORCE(IOException, scoreValue.isInt(), "A player's score is not an integer.");
	ENFORCE(IOException, hitsValue.isInt(), "A player's hit count is not an integer.");
	ENFORCE(IOException, shotsValue.isArray(), "A player's shot list is not an array.");

	return PlayerStats(scoreValue.asInt(), hitsValue.asInt(), parseShots(shotsValue));
}

} // end anonymous namespace

ResultsResponseMessage::ResultsResponseMessage(uint16_t id, uint16_t respTo,
                                               const std::string& message, StatsList&& playerStats) :
	// If we're sending a results response payload back, the request was ok.
	ResponseMessage(id, respTo, ResponseMessage::Code::OK, message),
	stats(move(playerStats))
{
	for (const auto& stat : stats) {
		ENFORCE(ArgumentException, stat.hits >= 0, "A player cannot have negative hits.");
	}
}

std::unique_ptr<ResultsResponseMessage> ResultsResponseMessage::fromJSON(const Json::Value& object)
{
	const auto responseInfo = ResponseMessage::fromJSON(object); // Get the basic response info

	ENFORCE(IOException, responseInfo->code == ResponseMessage::Code::OK,
	        "Full results response payloads must have an OK response code.");

	ENFORCE(IOException, object.isMember(playerStatsKey), "Results response payload is missing player stats");

	const Value& playerStatsValue = object[playerStatsKey];

	ENFORCE(IOException, playerStatsValue.isArray(), "The player stats value is not an array.");

	StatsList playerStats;

	for (const Value& stat : playerStatsValue) {
		playerStats.emplace_back(parseStats(stat));
	}

	return std::unique_ptr<ResultsResponseMessage>(
		new ResultsResponseMessage(responseInfo->id, responseInfo->respondingTo, responseInfo->message,
		                           move(playerStats)));
}

Json::Value ResultsResponseMessage::toJSON() const
{
	Value ret = ResponseMessage::toJSON();

	Value statsList(arrayValue);

	for (const auto& stat : stats) {
		Value statValue(objectValue);

		statValue[scoreKey] = stat.score;
		statValue[hitsKey] = stat.hits;

		Value shots(arrayValue);

		for (const auto& shot : stat.shots)
			shots.append(shot.toJSON());

		statValue[shotsKey] = move(shots);

		statsList.append(move(statValue));
	}

	ret[playerStatsKey] = move(statsList);

	return ret;
}

bool ResultsResponseMessage::operator==(const Message& o) const
{
	if (!ResponseMessage::operator==(o))
		return false;

	auto rrm = dynamic_cast<const ResultsResponseMessage*>(&o);

	if (rrm == nullptr)
		return false;

	return stats == rrm->stats;
}
