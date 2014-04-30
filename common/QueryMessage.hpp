#pragma once

#include "Message.hpp"

/**
 * \brief A query message sent to boards at the start of operation
 *
 * This is done to see what boards have successfully connected to the sytsem.
 * Boards will respond by acknowledging queries directed at them.
 */
class QueryMessage : public Message {

public:

	/// Allows a gun and a target to have the same ID at the cost of one more byte.
	/// Feel free to remove this if we want all boards to have their own IDs
	enum class BoardType : int8_t {
		GUN,
		TARGET
	};

	/**
	 * \brief Constructs a QueryMessage
	 * \param idNum The unique message ID
	 * \param id The board ID to query
	 * \param bt The type of board being queried
	 */
	QueryMessage(message_id_t idNum, board_id_t id, BoardType bt);

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<QueryMessage> fromJSON(const Json::Value& object);

	Json::Value toJSON() const override;
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<QueryMessage> fromBinary(uint8_t* buf, size_t len);

	/*
	 * \brief Gets the query's binary payload
	 *
	 * A query message has a two-byte payload:
	 * - The first byte is the board ID
	 * - The second byte is the type (see QueryMessage::BoardType)
	 */
	std::vector<uint8_t> getBinaryPayload() const override;

	const board_id_t boardID;

	const BoardType type;

	Type getType() const override { return Type::QUERY; }

	bool operator==(const Message& o) const override;
};
