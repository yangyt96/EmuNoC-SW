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
    {
        NocPe_Resource.hw_buff_count += inj_list->size;
        while (inj_list->size > 0)
        {
            *virt_buff = *(uint32_t *)list_front(inj_list);

            if (NocPe_Resource.verbose == 2)
            {
                printf("[INJECT] @%i ", inj_cyc);
                nocpe_print_packet(*(NocPe_Pkt_t *)virt_buff);
            }

            list_pop_front(inj_list);
            virt_buff++;
        }
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

int nocpe_eject(int rx_num_bd, List_t *hw_buff[])
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
            int pos = list_find_data_range(hw_buff[pkt->src], pkt, 0, sizeof(NocPe_Pkt_t) / 2);
            if (pos != -1)
            {
                NocPe_PktCyc_t *pkt_cyc = (NocPe_PktCyc_t *)list_at(hw_buff[pkt->src], pos);

                if (NocPe_Resource.verbose == 1)
                {
                    printf("icyc:%i ecyc:%i ", pkt_cyc->cyc, cyc);
                    nocpe_print_packet(*pkt);
                }
                else if (NocPe_Resource.verbose == 2)
                {
                    printf("[EJECT] @%i ", cyc);
                    nocpe_print_packet(*pkt);
                }

                // output to file
                if (NocPe_Resource.disable != true)
                    if (NocPe_Resource.log_file != NULL)
                        nocpe_csv_write(pkt_cyc->cyc, cyc, *pkt);

                list_erase(hw_buff[pkt->src], pos);
                NocPe_Resource.hw_buff_count--;
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

void nocpe_sync_eject(List_t *hw_buffers[])
{
    XAxiDma_DmaSr_Reg_t sr;

    // due to the blocking issue (hardware sp-ps), the stream to dma has not sufficient buffer descriptor, so eject the data first to prevent software deadlock
    uint32_t time_out = 0;
    do
    {
        if (time_out == 1000000)
        {
            // eject from hw
            do
            {

                if (RxDone == RxBdRing.max_bd_count && RxRead == RxBdRing.max_bd_count)
                    sg_restart_rx();

                sg_sync_rx();

                int rx_num_bd = RxDone - RxRead;
                if (rx_num_bd > 0)
                    nocpe_eject(rx_num_bd, hw_buffers);

            } while (RxDone - RxRead > 0 || RxDone == RxBdRing.max_bd_count || RxRead == RxBdRing.max_bd_count);

            time_out = 0;
        }

        sr = virt_dma->mm2s_sr;
        time_out++;
    } while (sr.Idle != 1);

    // Check error
    sr = virt_dma->mm2s_sr;
    Error += (sr.DMADecErr + sr.DMAIntErr + sr.DMASlvErr);
    if (Error > 0)
    {
        uintptr_t cur_phys_addr = virt_dma->mm2s_curdesc;
        uint32_t idx = (cur_phys_addr - TX_BD_BASE) / sizeof(XAxiDma_Bd_t);
        XAxiDma_Bd_t bd = TxBdRing.virt_head[idx];
        printf("Error: sg_sync_tx \n");
        xaxidma_print_dma_sr(virt_dma->mm2s_sr);
        xaxidma_print_bd(bd);
        return Error;
    }

    // check the bd is completed
    while (TxBdRing.virt_head[TxDone].stat.Cmplt != 0 && TxDone < TxBdRing.max_bd_count)
        TxDone++;

    if (TxDone != TxWrite)
    {
        printf("Error: sg_sync_tx - TxDone=%i TxWrite=%i \n", TxDone, TxWrite);
        uintptr_t cur_phys_addr = virt_dma->mm2s_curdesc;
        uint32_t idx = (cur_phys_addr - TX_BD_BASE) / sizeof(XAxiDma_Bd_t);
        XAxiDma_Bd_t bd = TxBdRing.virt_head[idx];
        printf("cur_phys_addr: 0x%08x \n", virt_dma->mm2s_curdesc);
        printf("idx: %i \n", idx);
        xaxidma_print_dma_sr(virt_dma->mm2s_sr);
        xaxidma_print_bd(bd);
    }
}

void nocpe_empty()
{
    sg_start();
    for (int i = 0; i < NocPe_Resource.max_cyc; i += NocPe_Resource.time_step)
    {
        if (TxDone == TxBdRing.max_bd_count && TxWrite == TxBdRing.max_bd_count)
            sg_restart_tx();

        nocpe_inject(i, NULL);
        sg_sync_tx();
    }

    sg_stop();
}

void nocpe_csv_wopen()
{
    char fname[100] = {};
    if (NocPe_Resource.output != NULL)
    {
        strcpy(fname, NocPe_Resource.output);
    }
    else
    {
        if (!strcmp(NocPe_Resource.mode, "full"))
            sprintf(fname, "%s_cyc-%u_time-step-%u_pkt-len-%u.csv",
                    NocPe_Resource.mode, NocPe_Resource.max_cyc,
                    NocPe_Resource.time_step, NocPe_Resource.pkt_len);
        else if (!strcmp(NocPe_Resource.mode, "random"))
            sprintf(fname, "%s_cyc-%u_seed-%u_min-time-step-%u_max-time-step-%u.csv",
                    NocPe_Resource.mode, NocPe_Resource.max_cyc, NocPe_Resource.seed,
                    NocPe_Resource.min_time_step, NocPe_Resource.max_time_step);
        else if (!strcmp(NocPe_Resource.mode, "netrace"))
            sprintf(fname, "%s_cyc-%u_%s_start-region-%i.csv",
                    NocPe_Resource.mode, NocPe_Resource.max_cyc,
                    NocPe_Resource.trace_file, NocPe_Resource.start_region);
        else if (!strcmp(NocPe_Resource.mode, "uniform"))
            sprintf(fname, "%s_cyc-%u_seed-%u_pkt-len-%u_num-interval-%u_inj-rate-%f.csv",
                    NocPe_Resource.mode, NocPe_Resource.max_cyc, NocPe_Resource.seed,
                    NocPe_Resource.pkt_len,
                    NocPe_Resource.num_interval, NocPe_Resource.inj_rate);
    }

    if (NocPe_Resource.log_file == NULL)
    {
        NocPe_Resource.log_file = fopen(fname, "w");
        fprintf(NocPe_Resource.log_file, "icyc, ecyc, id, src, dst, len \n");
    }
}

void nocpe_csv_write(NocPe_Cyc_t icyc, NocPe_Cyc_t ecyc, NocPe_Pkt_t pkt)
{
    fprintf(NocPe_Resource.log_file, "%u, %u, %u, %u, %u, %u \n", icyc, ecyc, pkt.id, pkt.src, pkt.dst, pkt.len);
}

void nocpe_csv_wclose()
{
    fclose(NocPe_Resource.log_file);
}

int nocpe_cyc_cmp(NocPe_PktCyc_t *a, NocPe_PktCyc_t *b)
{
    if (a->cyc < b->cyc)
        return -1; // a < b
    else if (a->cyc == b->cyc)
        return 0; // a == n
    else
        return 1; // a > b
}