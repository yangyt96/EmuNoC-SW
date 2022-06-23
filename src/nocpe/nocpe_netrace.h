/*
COPYRIGHT(c) 2022
INSTITUTE FOR COMMUNICATION TECHNOLOGIES AND EMBEDDED SYSTEMS
RWTH AACHEN
GERMANY

This confidential and proprietary software may be used, copied,
modified, merged, published or distributed according to the
permissions and/or limitations granted by an authorizing license
agreement.

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

Author: 1. Tan Yee Yang (tan@ice.rwth-aachen.de)
        2. Jan Moritz Joseph (joseph@ice.rwth-aachen.de)
*/

#include <stdint.h>
#include <stdlib.h>

#include "nocpe.h"
#include "nocpe_tools.h"
#include "cctrlib/list.h"
#include "netrace/netrace.h"

#pragma once

void nocpe_netrace_create(nt_packet_t nt_packet, List_t *wait_lists[]);
void nocpe_netrace_run();