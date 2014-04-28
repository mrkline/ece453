#include "BinaryMessageTests.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

#include "BinaryMessage.hpp"
#include "Test.hpp"

using namespace std;
using namespace BinaryMessage;

namespace {

void intConversions()
{
	vector<uint8_t> buf;

	appendInt(buf, (uint16_t)42);
	assert(buf.size() == 2);
	assert(extractUInt16(buf.data()) == 42);
	buf.clear();

	appendInt(buf, (int16_t)-42);
	assert(buf.size() == 2);
	assert(extractInt16(buf.data()) == -42);
	buf.clear();

	appendInt(buf, (uint32_t)1337);
	assert(buf.size() == 4);
	assert(extractUInt32(buf.data()) == 1337);
	buf.clear();

	appendInt(buf, (int32_t)-2564);
	assert(buf.size() == 4);
	assert(extractInt32(buf.data()) == -2564);
}

void sanity()
{
	std::array<uint8_t, 0> emptyPayload;
	vector<uint8_t> buf = makeMessage(Message::Type::EMPTY, 42,
	                                  begin(emptyPayload), end(emptyPayload));

	assert(isValidMessage(buf.data(), buf.size()));
	assert(getType(buf.data()) == Message::Type::EMPTY);
	assert(getID(buf.data()) == 42);

	std::vector<uint8_t> load = { 'p','a','y','l','o','a','d' };
	buf = makeMessage(Message::Type::EMPTY, 42,
	                        begin(load), end(load));
	assert(isValidMessage(buf.data(), buf.size()));
	assert(getType(buf.data()) == Message::Type::EMPTY);
	assert(getID(buf.data()) == 42);

	// Check getPayload
	auto lp = getPayload(buf.data());
	// Sizes should match
	assert(lp.second == load.size());
	// Data should match
	for (auto i = 0u; i < load.size(); ++i)
		assert(lp.first[i] == lp.first[i]);
}

void badCRC()
{
	std::vector<uint8_t> load = { 'p','a','y','l','o','a','d' };
	auto buf = makeMessage(Message::Type::EMPTY, 42, begin(load), end(load));
	assert(isValidMessage(buf.data(), buf.size()));

	// Change one of the payload bytes
	buf[7] = 'P';

	string why;
	assert(!isValidMessage(buf.data(), buf.size(), &why));
	assert(why.find("CRC") != string::npos);
}

} // end anonymous namespace

void Testing::BinaryMessageTests()
{
	beginUnit("Binary message serialization");
	test("int -> buffer -> int conversions", &intConversions);
	test("sanity", &sanity);
	test("Bad CRC", &badCRC);
}
