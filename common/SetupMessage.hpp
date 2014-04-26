#pragma once

#include <memory>
#include <unordered_map>

#include "Message.hpp"
#include "GameTypes.hpp"

/// A setup message
class SetupMessage : public Message {

public:

	typedef std::unordered_map<std::string, int> DataMap;

	SetupMessage(uint16_t id, GameType gType, int pCount, int time, int score, DataMap&& data = DataMap());

	static std::unique_ptr<SetupMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;

	Type getType() const override { return Type::SETUP; }

	/// The type of game to play
	const GameType gameType;

	/// The number of players playing.
	const int playerCount;

	/// If time is a win condition, this is the maximum game length, in seconds.
	/// If not, -1.
	const int endTime;

	/// If points is a win condition, this is the score a player must get to win.
	/// If not, -1
	const int winningScore;

	const DataMap gameData;

	bool operator==(const Message& o) const override;
};
