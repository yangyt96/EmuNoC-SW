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

#include "math.h"
#include "nocpe_tools.h"
#include "nocpe_random.h"
#pragma once

void nocpe_uniform_create(uint32_t max_cyc, uint32_t num_interval, float inj_rate, List_t *nocpe_pkt_cyc_list);
void nocpe_uniform_run();