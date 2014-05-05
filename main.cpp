#include <thread>
#include <cstdio>

#include "common/MessageJunction.hpp"
#include "common/GameStateMachine.hpp"
#include "common/TCPMessageBridge.hpp"
#include "common/MessageQueue.hpp"

using namespace std;

int main()
{
	MessageQueue toSM, fromSM;
	MessageQueue toUI, fromUI;
	MessageQueue toSys, fromSys;

	// TODO: Figure out our controller setup from the controller link
	printf("Lighting up state machine...\n");
	fflush(stdout);
	thread smThread(&runGame, ref(toSM), ref(fromSM), 2, 2);

	printf("Lighting up UI communications...\n");
	fflush(stdout);
	thread uiThread(&runTCPMessageServer, ref(toUI), ref(fromUI));

	printf("Lighting up the message juntion...\n");
	fflush(stdout);
	thread junctionThread(&runMessageJunction, ref(toSM), ref(fromSM),
	                                           ref(toUI), ref(fromUI),
	                                           ref(toSys), ref(fromSys));

	// We should never finish, but call join so that we stall here forever.
	junctionThread.join();
	smThread.join();
	uiThread.join();
	// We have a problem if we got here
	return 1;
}
