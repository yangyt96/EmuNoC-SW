#include "math.h"
#include "nocpe_tools.h"
#include "nocpe_random.h"
#pragma once

void nocpe_uniform_create(uint32_t max_cyc, uint32_t num_interval, float inj_rate, List_t *nocpe_pkt_cyc_list);
void nocpe_uniform_run();