#include "Payload.hpp"

using namespace Json;

const std::unordered_map<Payload::Type, Json::StaticString> Payload::typeNames = {
	{Payload::Type::RESPONSE, StaticString("response")},
	{Payload::Type::SETUP, StaticString("setup")},
	{Payload::Type::START, StaticString("start")},
	{Payload::Type::STOP, StaticString("stop")},
	{Payload::Type::STATUS, StaticString("status")},
	{Payload::Type::STATUS_RESPONSE, StaticString("status response")},
	{Payload::Type::RESULTS, StaticString("results")},
	{Payload::Type::RESULTS_RESPONSE, StaticString("results response")},
	{Payload::Type::UNKNOWN, StaticString("unknown")}
};

const std::unordered_map<std::string, Payload::Type> Payload::typeLookup = {
	{"response", Payload::Type::RESPONSE},
	{"setup", Payload::Type::SETUP},
	{"start", Payload::Type::START},
	{"stop", Payload::Type::STOP},
	{"status", Payload::Type::STATUS},
	{"status response", Payload::Type::STATUS_RESPONSE},
	{"results", Payload::Type::RESULTS},
	{"results response", Payload::Type::RESULTS_RESPONSE},
	{"unknown", Payload::Type::UNKNOWN}
};

Payload::Type Payload::nameToType(const std::string& name)
{
	auto it = typeLookup.find(name);

	if (it == end(typeLookup))
		return Payload::Type::UNKNOWN;
	else
		return it->second;
}

StaticString Payload::typeToName(Type t)
{
	return typeNames.at(t);
}
