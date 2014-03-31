#include "TestUnit.hpp"

#include <cstdio>

void Testing::TestUnit::runUnit()
{
	while (!unrunTests.empty()) {
		auto& curr = unrunTests.front();
		const char* currName = curr.name;
		try {
			curr.testProc();
			printf("SUCCESS: %s test succeeded\n", currName);
		}
		catch (const Exceptions::Exception& ex) {
			printf("FAILURE: An exception was thrown at %s:%d with the following message:\n\t%s\n",
			       ex.file, ex.line, ex.message.c_str());
		}
		catch (const std::exception& ex) {
			printf("FAILURE: An exception was thrown with the following message:\n\t%s\n",
			       ex.what());
		}
		catch (...) {
			printf("FAILURE: %s test failed and threw an unknown exception\n", currName);
		}
		unrunTests.pop();
	}
}
