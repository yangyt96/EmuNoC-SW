#include "nocpe_tools.h"
#pragma once

int random_int(int min, int max);

void nocpe_random_create(List_t *inj_buff[], List_t *wait_lists[]);
void nocpe_random_run();