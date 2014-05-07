#pragma once

#include <string>

#include "MessageQueue.hpp"

/// Accepts a incoming connections and passes Messages (one per line) over TCP
void runTCPMessageServer(MessageQueue& in, MessageQueue& out);

void runTCPMessageClient(MessageQueue& in, MessageQueue& out, std::string server);
