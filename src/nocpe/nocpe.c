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

#include "nocpe.h"

NocPe_Pkt_t nocpe_create_packet(uint32_t id, uint32_t src, uint32_t dst, uint32_t len)
{
    NocPe_Pkt_t ret;
    ret.id = id;
    ret.src = src;
    ret.dst = dst;
    ret.len = len;
    return ret;
}

void nocpe_print_packet(NocPe_Pkt_t pkt)
{
    printf("(0x%08x) id:%i src:%i dst:%i len:%i \n", pkt, pkt.id, pkt.src, pkt.dst, pkt.len);
}
