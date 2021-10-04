#include <stdint.h>
#include <stdlib.h>

#include "nocpe.h"
#include "nocpe_tools.h"
#include "cctrlib/list.h"
#include "netrace/netrace.h"

#pragma once

void nocpe_netrace_create(nt_packet_t nt_packet, List_t *wait_lists[]);
void nocpe_netrace_run();