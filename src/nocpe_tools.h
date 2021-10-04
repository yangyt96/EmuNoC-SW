#include "nocpe.h"
#include "cctrlib/list.h"
#include "xaxidma/xaxidma_sg.h"
#pragma once

int nocpe_inject(NocPe_Cyc_t inj_cyc, List_t *inj_list);
int nocpe_eject(int rx_num_bd, List_t *inj_buff[]);