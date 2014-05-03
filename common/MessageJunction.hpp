#pragma once

#include "MessageQueue.hpp"

void runMessageJunction(MessageQueue& toSM, MessageQueue& fromSM,
                        MessageQueue& toUI, MessageQueue& fromUI,
                        MessageQueue& toSys, MessageQueue& fromSys);
