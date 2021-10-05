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
