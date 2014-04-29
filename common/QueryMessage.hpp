#pragma once

#include "Message.hpp"

class QueryMessage : public Message {

public:

	/// Allows a gun and a target to have the same ID at the cost of one more byte.
	/// Feel free to remove this if we want all boards to have their own IDs
	enum class BoardType : int8_t {
		GUN,
		TARGET
	};

	QueryMessage(message_id_t idNum, board_id_t id, BoardType bt);

#ifdef WITH_JSON
	static std::unique_ptr<QueryMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	static std::unique_ptr<QueryMessage> fromBinary(uint8_t* buf, size_t len);

	std::vector<uint8_t> getBinaryPayload() const override;

	const board_id_t boardID;

	const BoardType type;

	Type getType() const override { return Type::QUERY; }

	bool operator==(const Message& o) const override;
};
