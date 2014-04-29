#include "TargetControlMessage.hpp"

#include <cassert>

#include "BinaryMessage.hpp"
#include "Exceptions.hpp"

using namespace std;
using namespace Exceptions;

#ifdef WITH_JSON
using namespace Json;
#endif

namespace {

#ifdef WITH_JSON
const StaticString idKey("target ID");
const StaticString onKey("target on");

const StaticString commandsKey("commands");
#endif

} // end anonymous namespace

#ifdef WITH_JSON
Json::Value TargetCommand::toJSON() const
{
	Value ret(objectValue);
	ret[idKey] = id;
	ret[onKey] = on;
	return ret;
}

TargetCommand TargetCommand::fromJSON(const Json::Value& object)
{
	ENFORCE(IOException, object.isObject(), "The provided JSON value is not an object.");

	const Value& idValue = object[idKey];
	const Value& onValue = object[onKey];

	ENFORCE(IOException, idValue.isInt(), "The target ID is not an integer.");
	ENFORCE(IOException, onValue.isBool(), "The target ID is not a bool.");

	return TargetCommand((board_id_t)idValue.asInt(), onValue.asBool());
}
#endif

TargetControlMessage::TargetControlMessage(message_id_t id, CommandList&& comms) :
	Message(id),
	commands(move(comms))
{
}

TargetControlMessage::TargetControlMessage(message_id_t id, const TargetCommand& comm) :
	Message(id),
	commands({comm})
{
}

#ifdef WITH_JSON
std::unique_ptr<TargetControlMessage> TargetControlMessage::fromJSON(const Json::Value& object)
{
	auto msg = Message::fromJSON(object);

	ENFORCE(IOException, object.isMember(commandsKey), "Target control message is missing its commands");

	const Value& commandsValue = object[commandsKey];

	ENFORCE(IOException, commandsValue.isArray(), "The commands list is not an array.");

	CommandList list;

	for (const Value& command : commandsValue)
		list.emplace_back(TargetCommand::fromJSON(command));

	return unique_ptr<TargetControlMessage>(
		new TargetControlMessage(msg->id, move(list)));
}

Json::Value TargetControlMessage::toJSON() const
{
	Value ret = Message::toJSON();

	Value commandArray(arrayValue);

	for (const auto& command : commands)
		commandArray.append(command.toJSON());

	ret[commandsKey] = move(commandArray);

	return ret;
}
#endif

std::unique_ptr<TargetControlMessage> TargetControlMessage::fromBinary(uint8_t* buf, size_t len)
{
	auto msg = Message::fromBinary(buf, len);
	auto load = BinaryMessage::getPayload(buf);
	ENFORCE(IOException, load.second > 3, "The payload cannot fit target commands.");
	
	const uint8_t numCommands = *load.first;
	ENFORCE(IOException, load.second - 1 == numCommands * 2,
	        "The payload is the incorrect size for target commands.");

	++load.first;

	CommandList comms;

	for (uint8_t i = 0; i < numCommands; ++i) {
		comms.emplace_back(load.first[0], (bool)load.first[1]);
		load.first += 2;
	}

	return unique_ptr<TargetControlMessage>(
		new TargetControlMessage(msg->id, move(comms)));
}

std::vector<uint8_t> TargetControlMessage::getBinaryPayload() const
{
	assert(Message::getBinaryPayload().size() == 0);
	vector<uint8_t> ret;
	ret.emplace_back((uint8_t)commands.size());

	for (const auto& command : commands) {
		ret.emplace_back(command.id);
		ret.emplace_back((uint8_t)command.on);
	}

	return ret;
}


bool TargetControlMessage::operator==(const Message& o) const
{
	if (!Message::operator==(o))
		return false;

	auto tcm = dynamic_cast<const TargetControlMessage*>(&o);
	if (tcm == nullptr)
		return false;

	return commands == tcm->commands;
}
