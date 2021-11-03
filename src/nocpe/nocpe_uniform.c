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

#include "nocpe_uniform.h"

void nocpe_uniform_create(uint32_t max_cyc, uint32_t num_interval, float inj_rate, List_t *nocpe_pkt_cyc_list)
{
    uint32_t avg_time_frame = max_cyc / num_interval;
    uint32_t rem_time_frame = max_cyc % num_interval;

    uint32_t tot_num_inj = (uint32_t)((float)max_cyc * inj_rate);
    uint32_t avg_num_inj = tot_num_inj / num_interval;
    uint32_t rem_num_inj = tot_num_inj % num_interval;

    uint32_t calc_num_inj = 0;
    uint32_t calc_time_frame = 0;
    uint32_t cur_cyc = 0;

    for (int i = 0; i < num_interval; i++)
    {
        uint32_t time_frame = avg_time_frame;
        if (i < rem_time_frame)
            time_frame++;

        uint32_t num_inj = avg_num_inj;
        if (i < rem_num_inj)
            num_inj++;

        uint32_t max_time_frame = cur_cyc + time_frame;
        uint32_t sub_cur_cyc = cur_cyc;

        NocPe_PktCyc_t pkt_cycs[num_inj];
        for (int j = 0; j < num_inj; j++)
        {
            uint32_t src = random_int(0, NOCPE_PE_NUM - 1);
            uint32_t dst;
            do
                dst = random_int(0, NOCPE_PE_NUM - 1);
            while (dst == src);

            NocPe_PktCyc_t pkt_cyc;
            pkt_cyc.pkt = nocpe_create_packet(NocPe_Resource.id[src]++, src, dst, NocPe_Resource.pkt_len);

            pkt_cyc.cyc = random_int(sub_cur_cyc, max_time_frame);

            pkt_cycs[j] = pkt_cyc;
        }

        // sort by time
        qsort(pkt_cycs, num_inj, sizeof(NocPe_PktCyc_t), nocpe_cyc_cmp);

        // push back to list from array, checkout list package update (need to do debug -> list_insert_array)
        for (int k = 0; k < num_inj; k++)
            list_push_back(nocpe_pkt_cyc_list, &pkt_cycs[k]);

        cur_cyc += time_frame;
    }
}

void nocpe_uniform_run()
{
    sg_start();

    // uniform settings
    uint32_t num_interval = NocPe_Resource.num_interval;
    float inj_rate = NocPe_Resource.inj_rate;

    // uniform vars
    List_t *uniform_pkt_cyc_list = list_init(sizeof(NocPe_PktCyc_t));

    // nocpe vars
    NocPe_Cyc_t max_cyc = NocPe_Resource.max_cyc;
    NocPe_Cyc_t cyc = 0;

    List_t *inj_lists[NOCPE_PE_NUM];
    List_t *hw_buffers[NOCPE_PE_NUM];
    List_t *hw_list = list_init(sizeof(NocPe_Pkt_t));

    for (int i = 0; i < NOCPE_PE_NUM; i++)
    {
        hw_buffers[i] = list_init(sizeof(NocPe_PktCyc_t));
        inj_lists[i] = list_init(sizeof(NocPe_Pkt_t));
    }

    nocpe_uniform_create(max_cyc, num_interval, inj_rate, uniform_pkt_cyc_list);

    do
    {
        if (cyc < max_cyc)
            while (uniform_pkt_cyc_list->size > 0)
            {
                NocPe_PktCyc_t *pkt_cyc = list_front(uniform_pkt_cyc_list);
                if (pkt_cyc->cyc != cyc)
                    break;

                list_push_back(inj_lists[pkt_cyc->pkt.src], &(pkt_cyc->pkt));

                list_pop_front(uniform_pkt_cyc_list);
            }

        // put to hw_buffers whenever its empty and and hw_list for hw injection
        if (cyc < max_cyc)
            for (int i = 0; i < NOCPE_PE_NUM; i++)
                while (inj_lists[i]->size > 0)
                {
                    if (hw_buffers[i]->size >= NOCPE_INJ_BUFF_DEPTH)
                        break;

                    NocPe_PktCyc_t pkt_cyc;
                    pkt_cyc.cyc = cyc;
                    pkt_cyc.pkt = *(NocPe_Pkt_t *)list_front(inj_lists[i]);
                    list_push_back(hw_buffers[i], &pkt_cyc);

                    NocPe_Pkt_t pkt = pkt_cyc.pkt;
                    list_push_back(hw_list, &pkt);

                    list_pop_front(inj_lists[i]);
                }

        // inject to hw
        if (TxDone == TxBdRing.max_bd_count && TxWrite == TxBdRing.max_bd_count)
            sg_restart_tx();

        nocpe_inject(cyc, hw_list); // single injection

        // sg_sync_tx(); // after all have been ejected then check the injection is done or not
        nocpe_sync_eject(hw_buffers);

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

        if (cyc < max_cyc && uniform_pkt_cyc_list->size > 0)
            cyc = ((NocPe_PktCyc_t *)list_front(uniform_pkt_cyc_list))->cyc;
        else
            cyc += NocPe_Resource.time_step;

        if (cyc > 2 * max_cyc)
            break;
    } while (cyc < max_cyc || NocPe_Resource.hw_buff_count > 0);

    printf("[END] num_lost:%i cyc:%i \n", NocPe_Resource.hw_buff_count, cyc);
    for (int i = 0; i < NOCPE_PE_NUM; i++)
    {
        while (hw_buffers[i]->size > 0)
        {
            NocPe_PktCyc_t *pkt_cyc = (NocPe_PktCyc_t *)list_front(hw_buffers[i]);

            printf("[LOST] @%i ", pkt_cyc->cyc);
            nocpe_print_packet(pkt_cyc->pkt);

            list_pop_front(hw_buffers[i]);
        }
    }

    // Free memory
    list_destroy(hw_list);
    for (int i = 0; i < NOCPE_PE_NUM; i++)
    {
        list_destroy(inj_lists[i]);
        list_destroy(hw_buffers[i]);
    }

    sg_stop();
}