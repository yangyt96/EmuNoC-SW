#include "nocpe.h"
#include "cctrlib/list.h"
#include "xaxidma/xaxidma_sg.h"
#pragma once

typedef struct
{
    // General
    uint32_t max_cyc;
    uint32_t seed;

    // full
    uint32_t time_step;
    uint32_t pkt_len;

    // random
    uint32_t min_time_step;
    uint32_t max_time_step;

    // netrace
    char *trace_file;
    int start_region;
    int ignore_dependencies;
    int reader_throttling;

    // uniform
    uint32_t num_interval;
    float inj_rate;

} NocPe_Resource_t;

inline NocPe_Resource_t NocPe_Resource;

int nocpe_inject(NocPe_Cyc_t inj_cyc, List_t *inj_list);
int nocpe_eject(int rx_num_bd, List_t *inj_buff[]);