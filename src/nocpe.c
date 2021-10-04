#include "nocpe.h"

NocPe_Pkt_t nocpe_create_packet(uint32_t src, uint32_t dst, uint32_t len)
{
    NocPe_Pkt_t ret;
    ret.id = nocpe_pkt_id++;
    ret.src = src;
    ret.dst = dst;
    ret.len = len;
    return ret;
}

void nocpe_print_packet(NocPe_Pkt_t pkt)
{
    printf("(0x%08x) id:%i src:%i dst:%i len:%i \n", pkt, pkt.id, pkt.src, pkt.dst, pkt.len);
}
