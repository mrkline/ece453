#include "ResultsResponsePayload.hpp"

#include <utility>

#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

ResultsResponsePayload::ResultsResponsePayload(int respTo, StatsList&& playerStats) :
	ResponsePayload(respTo, RC_OK, ""), // If we're sending a full status response payload back, the request was ok.
	stats(move(playerStats))
{
	for (const auto& stat : stats) {
		enforce<ArgumentException>(stat.hits >= 0, "A player cannot have negative hits.", __FUNCTION__);

		for (const auto& shot : stat.shots)
			enforce<ArgumentException>(shot.time >= 0, "A shot cannot happen before the game starts.", __FUNCTION__);
	}
}
