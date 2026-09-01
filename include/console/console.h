#pragma once

#ifdef ARDUINO

#include "swarm/swarm_core.h"

void console_begin();
void console_poll(SwarmCore& core);

#endif
