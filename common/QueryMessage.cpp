#include "QueryMessage.hpp"

#include <cassert>
#include <limits>

#include "BinaryMessage.hpp"
#include "Exceptions.hpp"

using namespace Exceptions;
using namespace std;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
// Using StaticString allows JSONCPP to make some optimzations because it knows the strings are static.
const StaticString idKey("board ID");
const StaticString boardTypeKey("board type");
#endif

} // end anonymous namespace

QueryMessage::QueryMessage(message_id_t idNum, board_id_t id, BoardType bt) :
	Message(idNum),
	boardID(id),
	type(bt)
{
}

#ifdef WITH_JSON
std::unique_ptr<QueryMessage> QueryMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(idKey), "Query payload is missing the board ID");
	ENFORCE(IOException, object.isMember(boardTypeKey), "Query payload is missing the board type");

	const Value& idValue = object[idKey];
	const Value& typeValue = object[boardTypeKey];

	ENFORCE(IOException, idValue.isInt(), "The board ID is not an integer.");
	ENFORCE(IOException, typeValue.isString(), "The board type is not a string");

	const int rawID = idValue.asInt();
	ENFORCE(IOException, rawID >= 0, "The board ID cannot be negative");
	ENFORCE(IOException, rawID <= numeric_limits<message_id_t>::max(),
	        "The board ID has too high of a value");

	const board_id_t id = (board_id_t)rawID;

	const string rawType = typeValue.asString();
	BoardType type;

	if (rawType == "target")
		type = BoardType::TARGET;
	else if (rawType == "gun")
		type = BoardType::GUN;
	else
		THROW(IOException, "The board type is an unknown value.");

	return unique_ptr<QueryMessage>(
		new QueryMessage(msg->id, id, type));
}

Json::Value QueryMessage::toJSON() const
{
	Value ret = Message::toJSON();

	ret[idKey] = boardID;
	ret[boardTypeKey] = type == BoardType::TARGET ? "target" : "gun";

	return ret;
}
#endif

std::unique_ptr<QueryMessage> QueryMessage::fromBinary(uint8_t* buf, size_t len)
{
	auto msg = Message::fromBinary(buf, len);

	auto load = BinaryMessage::getPayload(buf);

	ENFORCE(IOException, load.second >= sizeof(board_id_t) + 1, "The provided message is too small");

	static_assert(sizeof(message_id_t) == 2, "Someone changed the message ID size");
	board_id_t id = (board_id_t)load.first[0];
	BoardType bt = (BoardType)load.first[1];

	return std::unique_ptr<QueryMessage>(
		new QueryMessage(msg->id, id, bt));
}

std::vector<uint8_t> QueryMessage::getBinaryPayload() const
{
	assert(Message::getBinaryPayload().size() == 0);

	vector<uint8_t> ret;
	ret.emplace_back((uint8_t)boardID);
	ret.emplace_back((uint8_t)type);
	return ret;
}

bool QueryMessage::operator==(const Message& o) const
{
	if (!Message::operator==(o))
		return false;

	auto qm = dynamic_cast<const QueryMessage*>(&o);

	if (qm == nullptr)
		return false;

	return boardID == qm->boardID
		&& type == qm->type;
}
