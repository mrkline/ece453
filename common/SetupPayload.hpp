#pragma once

#include <memory>
#include <unordered_map>

#include "Payload.hpp"
#include "GameTypes.hpp"

/// A payload for setup messages
class SetupPayload : public Payload {

public:

	typedef std::unordered_map<std::string, int> DataMap;

	SetupPayload(GameType gType, int pCount, uint8_t winConds, int time, int score, DataMap&& data);

	static std::unique_ptr<SetupPayload> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return PT_SETUP; }

	/// The type of game to play
	const GameType gameType;

	/// The number of players playing.
	const int playerCount;

	/// A bitmask of WC_TIME and/or WC_POINTS
	/// \see WinCondition
	const uint8_t winConditions;

	/// If time is a win condition, this is the maximum game length, in seconds.
	/// If not, -1.
	const int endTime;

	/// If points is a win condition, this is the score a player must get to win.
	/// If not, -1
	const int winningScore;

	const DataMap gameData;

	bool operator==(const SetupPayload& o) const;

	bool operator!=(const SetupPayload& o) const { return !(*this == o); }
};
