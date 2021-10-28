/*
Copyright (c) 2021 Yee Yang Tan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdbool.h>

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
    uint32_t pkt_len; // full/uniform

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

    // internal
    NocPe_Cyc_t id[NOCPE_PE_NUM];
    FILE *log_file;
    int32_t verbose;
    bool disable; // disable the output file

} NocPe_Resource_t;

inline NocPe_Resource_t NocPe_Resource;

int nocpe_inject(NocPe_Cyc_t inj_cyc, List_t *inj_list);
int nocpe_eject(int rx_num_bd, List_t *inj_buff[]);
void nocpe_empty();

void nocpe_csv_wopen();
void nocpe_csv_write(NocPe_Cyc_t icyc, NocPe_Cyc_t ecyc, NocPe_Pkt_t pkt);
void nocpe_csv_wclose();

int nocpe_cyc_cmp(NocPe_PktCyc_t *a, NocPe_PktCyc_t *b);