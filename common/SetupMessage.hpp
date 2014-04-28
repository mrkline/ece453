#pragma once

#include <memory>
#include <unordered_map>

#include "Exceptions.hpp"
#include "Message.hpp"
#include "GameTypes.hpp"

/// A setup message
class SetupMessage : public Message {

public:

	typedef std::unordered_map<std::string, int> DataMap;

	SetupMessage(message_id_t id, GameType gType, board_id_t pCount, duration_t time, score_t score,
	             DataMap&& data = DataMap());

#ifdef WITH_JSON
	static std::unique_ptr<SetupMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	std::vector<uint8_t> getBinaryPayload() const override
	{
		THROW(Exceptions::InvalidOperationException,
		      "This messsage type does not support binary serialization");
	}

	Type getType() const override { return Type::SETUP; }

	/// The type of game to play
	const GameType gameType;

	/// The number of players playing.
	const board_id_t playerCount;

	/// If time is a win condition, this is the maximum game length, in seconds.
	/// If not, -1.
	const duration_t gameLength;

	/// If points is a win condition, this is the score a player must get to win.
	/// If not, -1
	const score_t winningScore;

	const DataMap gameData;

	bool operator==(const Message& o) const override;
};
