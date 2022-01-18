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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#pragma once

// Hardware settings
#define NOCPE_PE_NUM (64) // MAX_DIM_X * MAX_DIM_Y * MAX_DIM_Z
#define NOCPE_INJ_BUFF_DEPTH (16)

// Bit Width
#define NOCPE_PKT_SIZE (sizeof(uint32_t) * 8) // 32-bit
#define NOCPE_PKT_LEN_WIDTH (5)               // 5-bit
#define NOCPE_PKT_ADDR_WIDTH (6)              // (uint32_t)(floor(log2(NOCPE_PE_NUM - 1)) + 1)
#define NOCPE_PKT_ID_WIDTH (NOCPE_PKT_SIZE - NOCPE_PKT_LEN_WIDTH - 2 * NOCPE_PKT_ADDR_WIDTH)

// Constants
#define NOCPE_PKT_MIN_LEN (1)
#define NOCPE_PKT_MAX_LEN (int)(pow(2, NOCPE_PKT_LEN_WIDTH) - 1)

/**
 * Data structure
 **/

// HW structure
typedef struct __attribute__((__packed__))
{
    uint16_t len : NOCPE_PKT_LEN_WIDTH;
    uint16_t dst : NOCPE_PKT_ADDR_WIDTH;
    uint16_t src : NOCPE_PKT_ADDR_WIDTH;
    uint32_t id : NOCPE_PKT_ID_WIDTH;
} NocPe_Pkt_t;

typedef uint32_t NocPe_Cyc_t;

// SW structure
typedef struct
{
    NocPe_Pkt_t pkt;
    NocPe_Cyc_t cyc;
} NocPe_PktCyc_t;

// Functions
NocPe_Pkt_t nocpe_create_packet(uint32_t id, uint32_t src, uint32_t dst, uint32_t len);
void nocpe_print_packet(NocPe_Pkt_t pkt);
