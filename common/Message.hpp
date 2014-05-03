#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#ifdef WITH_JSON
#include <jsoncpp/json/json.h>
#endif

#include "GameTypes.hpp"

/**
 * \brief A message to pass between entities.
 *
 * Messages are the underpinning of our entire system.
 * They are passed from the UI to the game state machine,
 * from the state machine to the various game elements (targets and guns), and back.
 * All are serializable to JSON so that they can be sent via TCP to and from the UI,
 * and those that are sent to guns and targets are also serializable to a binary format.
 *
 * All messages contain a unique ID that a receiver uses to acknowledge back to the sender.
 * \see ResponseMessage
 */
class Message {

public:

	/// Different types of message payloads.
	enum class Type : int8_t {
		EMPTY, ///< A message containing nothing but an ID
		RESPONSE, ///< A generic response
		QUERY, ///< Query a board to see if it's in the system
		SETUP, ///< Set up a game
		START, ///< Start a game
		STOP, ///< Stop a running game
		STATUS, ///< Get the game status
		STATUS_RESPONSE, ///< Response to a status query
		RESULTS, ///< Get the game results after a match has finished
		RESULTS_RESPONSE, ///< Response to a results query
		SHOT, ///< Represents a shot fired
		MOVEMENT, ///< Carries the motion of a previously-reported shot
		TARGET_CONTROL, ///< A message to set target lights on or off
		EXIT, ///< The entity receiving this message should exit/finish
		TEST, ///< A test payload that holds a string
		UNKNOWN ///< An unknown/invalid payload type
	};

	static const std::unordered_map<Message::Type, std::string> nameLookup;

	static const std::unordered_map<std::string, Message::Type> typeLookup;

	/// Constructs a base message, which only contains an ID.
	Message(message_id_t idNum);

	virtual ~Message() { }

#ifdef WITH_JSON
	/// Deserializes a message from a JSON object.
	/// \warning Do not call this directly. Call JSONToMessage instead.
	static std::unique_ptr<Message> fromJSON(const Json::Value& object);

	/// The JSON key used to store the type value of a message
	const static Json::StaticString typeKey;

	/// Serializes a message to a JSON object
	virtual Json::Value toJSON() const;
#endif

	/// \brief Deserializes a message from a binary buffer
	/// \warning Do not call this directly. Call binaryToMessage instead.
	static std::unique_ptr<Message> fromBinary(uint8_t* buf, size_t len);

	/**
	 * \brief Returns a binary representation of a Message
	 * \see BinaryMessage.hpp
	 */
	std::vector<uint8_t> toBinary() const;

	/**
	 * \brief Returns a binary representation of the message's payload
	 *
	 * Since all binary representations of message differ only in type and payload,
	 * toBinary is not a virtual method - it remains the same for all messages.
	 * Messages derived from this base class should override this function
	 * in order to fill messages with their unique message type's content.
	 *
	 * A plain message has no payload, so this returns an empty vector.
	 */
	virtual std::vector<uint8_t> getBinaryPayload() const;

	/**
	 * \brief Gets the message's type
	 *
	 * If you have a Message pointer but can get its type from this function,
	 * you can cast that pointer to the correct derived type.
	 * For other cases where hierarchies are involved
	 * (say A inhereits B inherits C and you have an A but want to cast it to B)
	 * dynamic_cast works as usual.
	 */
	virtual Type getType() const { return Type::EMPTY; }

	/// The message's type.
	const message_id_t id;

	/// Comparison operator.
	/// Returns true iff the other message is the same type
	/// with the same contenets.
	virtual bool operator==(const Message& o) const;
};

namespace std
{
	/// Hasher for Message::Type
	template<>
	struct hash<Message::Type> {
		size_t operator()(Message::Type t) const { return std::hash<int>()((int)t); }
	};
}

#ifdef WITH_JSON
/// Deserializes a JSON object into the correct message type and returns a pointer to it
std::unique_ptr<Message> JSONToMessage(const Json::Value& object);
#endif

/// Deserializes a binary representaiton into the correct message type and returns a pointer to it.
std::unique_ptr<Message> binaryToMessage(uint8_t* buf, size_t len);
