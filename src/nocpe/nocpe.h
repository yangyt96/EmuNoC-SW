#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#pragma once

// Hardware settings
#define NOCPE_PE_NUM (4) // MAX_DIM_X * MAX_DIM_Y * MAX_DIM_Z
#define NOCPE_INJ_BUFF_DEPTH (4)

// Bit Width
#define NOCPE_PKT_SIZE (sizeof(uint32_t) * 8) // 32-bit
#define NOCPE_PKT_LEN_WIDTH (5)               // 5-bit
#define NOCPE_PKT_ADDR_WIDTH (2)              // (uint32_t)(floor(log2(NOCPE_PE_NUM - 1)) + 1)
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

// Global variable
inline static uint32_t nocpe_pkt_id = 0;

// Functions
NocPe_Pkt_t nocpe_create_packet(uint32_t src, uint32_t dst, uint32_t len);
void nocpe_print_packet(NocPe_Pkt_t pkt);
