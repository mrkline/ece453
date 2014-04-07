#include "MemoryUtilsTests.hpp"

#include "Test.hpp"
#include "MemoryUtils.hpp"

using namespace std;

namespace {

class A { public: virtual ~A() { } };
class B : public A { };
class C { };

void goodCast()
{
	B* dumbB = new B;
	unique_ptr<A> smartA(dumbB);

	unique_ptr<B> smartB = unique_dynamic_cast<B>(move(smartA));

	assert(smartB != nullptr);
	assert(smartB.get() == dumbB);
	assert(smartA == nullptr);
}

void badCast()
{
	B* dumbB = new B;
	unique_ptr<A> smartA(dumbB);

	unique_ptr<C> badC = unique_dynamic_cast<C>(move(smartA));

	assert(badC == nullptr);
	assert(smartA != nullptr);
	assert(smartA.get() == dumbB);
}

void nullCast()
{
	unique_ptr<A> smartA;
	assert(smartA == nullptr);

	unique_ptr<B> nullB = unique_dynamic_cast<B>(move(smartA));
	assert(smartA == nullptr);
	assert(nullB == nullptr);
}

} // end anonymous namespace

void Testing::memoryUtilsTests()
{
	beginUnit("GameStateMachine");
	test("Good cast", &goodCast);
	test("Bad cast", &badCast);
	test("Null cast", &nullCast);
}
