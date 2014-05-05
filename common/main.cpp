#include <thread>

#include "MessageJunction.hpp"
#include "GameStateMachine.hpp"
#include "TCPMessageBridge.hpp"
#include "MessageQueue.hpp"

using namespace std;

int main()
{
	MessageQueue toSM, fromSM;
	MessageQueue toUI, fromUI;
	MessageQueue toSys, fromSys;

	// TODO: Figure out our controller setup from the controller link
	thread smThread(&runGame, ref(toSM), ref(fromSM), 2, 2);
	thread uiThread(&runTCPMessageServer, ref(toUI), ref(fromUI));
	thread junctionThread(&runMessageJunction, ref(toSM), ref(fromSM),
	                                           ref(toUI), ref(fromUI),
	                                           ref(toSys), ref(fromSys));

	// We'll never return here unless something goes wrong,
	// so don't bother joining for now.
	return 0;
}
