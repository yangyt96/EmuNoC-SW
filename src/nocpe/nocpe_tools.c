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

#include "nocpe_tools.h"

const uintptr_t _nocpe_max__nocpe_tx_buff_ptr__ = TX_BUFF_HIGH - TX_BUFF_BASE;
inline volatile uintptr_t _nocpe_tx_buff_ptr_ = 0;

int nocpe_inject(NocPe_Cyc_t inj_cyc, List_t *inj_list)
{
    uint32_t tx_num_bd = 1;
    void *tx_virt_addr[tx_num_bd];
    void *tx_phys_addr[tx_num_bd];
    uint32_t tx_dbytes[tx_num_bd];

    if (inj_list == NULL)
        tx_dbytes[0] = sizeof(uint32_t);
    else
        tx_dbytes[0] = sizeof(uint32_t) * (1 + inj_list->size);

    // reuse old if larger, otherwise continue use old starting address
    if ((_nocpe_tx_buff_ptr_ + tx_dbytes[0]) > _nocpe_max__nocpe_tx_buff_ptr__)
        _nocpe_tx_buff_ptr_ = 0;

    tx_phys_addr[0] = (void *)(TX_BUFF_BASE + _nocpe_tx_buff_ptr_);
    tx_virt_addr[0] = (void *)(virt_tx_buff + _nocpe_tx_buff_ptr_);

    uint32_t *virt_buff = (uint32_t *)tx_virt_addr[0];

    virt_buff[0] = inj_cyc;
    virt_buff++;

    if (inj_list != NULL)
        while (inj_list->size > 0)
        {
            *virt_buff = *(uint32_t *)list_front(inj_list);

            printf("[INJECT] @%i ", inj_cyc);
            nocpe_print_packet(*(NocPe_Pkt_t *)virt_buff);

            list_pop_front(inj_list);
            virt_buff++;
        }

    Sg_Packets_t sg_pkt;

    sg_pkt.virt_addr = tx_virt_addr;
    sg_pkt.phys_addr = tx_phys_addr;
    sg_pkt.dbytes = tx_dbytes;
    sg_pkt.num_bd = tx_num_bd;

    _nocpe_tx_buff_ptr_ += tx_dbytes[0];

    int status = sg_tran_packets(&sg_pkt);

    return status;
}

int nocpe_eject(int rx_num_bd, List_t *inj_buff[])
{
    if (rx_num_bd == 0)
    {
        printf("Error: Invalid rx_num_bd=%i \n", rx_num_bd);
        return EXIT_FAILURE;
    }

    void *rx_virt_addr[rx_num_bd];
    void *rx_phys_addr[rx_num_bd];
    uint32_t rx_dbytes[rx_num_bd];

    Sg_Packets_t sg_pkts;
    sg_pkts.num_bd = rx_num_bd;
    sg_pkts.virt_addr = rx_virt_addr;
    sg_pkts.phys_addr = rx_phys_addr;
    sg_pkts.dbytes = rx_dbytes;

    int status = sg_recv_packets(&sg_pkts);
    if (status != EXIT_SUCCESS)
    {
        printf("Error: nocpe eject sg_recv_packets \n");
        return status;
    }

    for (int i = 0; i < sg_pkts.num_bd; i++)
    {

        uint32_t *rx_buffer = (uint32_t *)(sg_pkts.virt_addr[i]);

        NocPe_Cyc_t cyc = rx_buffer[0];
        for (int j = 1; j < sg_pkts.dbytes[i] / sizeof(uint32_t); j++)
        {

            if (rx_buffer[j] == 0) // tlast indicator
                continue;

            NocPe_Pkt_t *pkt = (NocPe_Pkt_t *)(&rx_buffer[j]);
            int pos = list_find_data_range(inj_buff[pkt->src], pkt, 0, sizeof(NocPe_Pkt_t));
            if (pos != -1)
            {
                NocPe_PktCyc_t *pkt_cyc = (NocPe_PktCyc_t *)list_at(inj_buff[pkt->src], pos);

                printf("[EJECT] @%i ", cyc);
                // printf("icyc:%i ecyc:%i ", pkt_cyc->cyc, cyc);
                nocpe_print_packet(*pkt);

                list_erase(inj_buff[pkt->src], pos);
            }
            else
            {
                printf("Error: num_bd=%i pkt=%i \n", i, j);
                printf("ecyc:%u ", cyc);
                nocpe_print_packet(*pkt);
            }
        }
    }

    return EXIT_SUCCESS;
}

void nocpe_empty()
{
    sg_start();
    for (int i = 0; i < 10000; i += 100)
    {
        nocpe_inject(i, NULL);
        sg_sync_tx();
    }

    sg_stop();
}