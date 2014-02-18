#include "TestUnit.hpp"

#include <cstdio>

void Testing::TestUnit::runUnit()
{
	const char* const name = getUnitName();

	while (!unrunTests.empty()) {
		auto& curr = unrunTests.front();
		const char* currName = curr.name;
		try {
			curr.testProc();
			printf("SUCCESS: %s test succeeded\n", currName);
			succeededTests.push_back(std::move(curr));
		}
		catch (const TestFailedException& ex) {
			printf("FAILURE: %s test failed on %s:%d\n", currName, ex.file, ex.line);
			if (ex.message != nullptr)
				printf("\t%s\n", ex.message);
			failedTests.push_back(std::move(curr));
		}
		catch (const Exceptions::Exception& ex) {
			printf("FAILURE: The method %s threw an exception with the following message:\n\t%s\n",
			       ex.callingFunction.c_str(), ex.message.c_str());
		}
		catch (const std::exception& ex) {
			printf("FAILURE: An exception was thrown with the following message:\n\t%s\n",
			       ex.what());
		}
		catch (...) {
			printf("FAILURE: %s test failed and threw an unknown exception\n", currName);
			failedTests.push_back(std::move(curr));
		}
		unrunTests.pop();
	}

	printf("\nResults for %s\n", name);

	if (!succeededTests.empty()) {
		printf("\tSucceeded Tests:\n");
		for (const auto& test : succeededTests)
			printf("\t\t%s\n", test.name);
	}

	if (!failedTests.empty()) {
		printf("\tFailed Tests:\n");
		for (const auto& test : failedTests)
			printf("\t\t%s\n", test.name);
	}
}
