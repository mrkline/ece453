#include "Payload.hpp"

using namespace Json;

const std::unordered_map<Payload::Type, Json::StaticString> Payload::typeNames = {
	{Payload::PT_RESPONSE, StaticString("response")},
	{Payload::PT_SETUP, StaticString("setup")},
	{Payload::PT_START, StaticString("start")},
	{Payload::PT_STOP, StaticString("stop")},
	{Payload::PT_STATUS, StaticString("status")},
	{Payload::PT_STATUS_RESPONSE, StaticString("status response")},
	{Payload::PT_RESULTS, StaticString("results")},
	{Payload::PT_RESULTS_RESPONSE, StaticString("results response")},
	{Payload::PT_UNKNOWN, StaticString("unknown")}
};

const std::unordered_map<std::string, Payload::Type> Payload::typeLookup = {
	{"response", Payload::PT_RESPONSE},
	{"setup", Payload::PT_SETUP},
	{"start", Payload::PT_START},
	{"stop", Payload::PT_STOP},
	{"status", Payload::PT_STATUS},
	{"status response", Payload::PT_STATUS_RESPONSE},
	{"results", Payload::PT_RESULTS},
	{"results response", Payload::PT_RESULTS_RESPONSE},
	{"unknown", Payload::PT_UNKNOWN}
};

Payload::Type Payload::nameToType(const std::string& name)
{
	auto it = typeLookup.find(name);

	if (it == end(typeLookup))
		return Payload::PT_UNKNOWN;
	else
		return it->second;
}

StaticString Payload::typeToName(Type t)
{
	return typeNames.at(t);
}
