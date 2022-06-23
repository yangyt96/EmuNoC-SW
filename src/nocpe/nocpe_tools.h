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

#include <stdbool.h>
#include <libgen.h>

#include "nocpe.h"
#include "cctrlib/list.h"
#include "xaxidma/xaxidma_sg.h"
#pragma once

typedef struct
{
    // General
    uint32_t max_cyc;
    uint32_t seed;
    char *output;
    char *mode;

    // full
    uint32_t time_step;
    uint32_t pkt_len; // full/uniform/neuro

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

    // neuromorphic
    char *neuro_file;
    double sparsity;

    // internal
    uint32_t hw_buff_count;
    NocPe_Cyc_t id[NOCPE_PE_NUM];
    FILE *log_file;
    int32_t verbose;
    bool disable; // disable the output file

} NocPe_Resource_t;

inline NocPe_Resource_t NocPe_Resource;

int nocpe_inject(NocPe_Cyc_t inj_cyc, List_t *inj_list);
int nocpe_eject(int rx_num_bd, List_t *hw_buff[]);
void nocpe_sync_eject(List_t *hw_buffers[]);
void nocpe_empty();

void nocpe_csv_wopen();
void nocpe_csv_write(NocPe_Cyc_t icyc, NocPe_Cyc_t ecyc, NocPe_Pkt_t pkt);
void nocpe_csv_wclose();

int nocpe_cyc_cmp(NocPe_PktCyc_t *a, NocPe_PktCyc_t *b);