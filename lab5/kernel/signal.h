#pragma once

#include "exception.h"

#define SIGKILL 9
#define SIG_NUM 32

typedef void (*signal_handler_t)();

void signal_handle(trap_frame* tf);
