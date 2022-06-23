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

#include "nocpe_random.h"

int random_int(int min, int max)
{
    if (max < min)
        return 0;

    return min + rand() % (max + 1 - min); // include max num random
}

/**
 * @brief Check the hw_buffers to determine the generation of inj_lists
 *
 * @param hw_buffers
 * @param inj_lists
 */
void nocpe_random_create(List_t *hw_buffers[], List_t *inj_lists[])
{
    int used_hw_buffers = 0;
    for (int i = 0; i < NOCPE_PE_NUM; i++)
        used_hw_buffers += hw_buffers[i]->size;
    int remain_hw_buffers = NOCPE_INJ_BUFF_DEPTH * NOCPE_PE_NUM - used_hw_buffers;

    int inj_num = random_int(0, remain_hw_buffers);

    for (int i = 0; i < inj_num; i++)
    {

        int src;
        int dst;
        int len = random_int(NOCPE_PKT_MIN_LEN, NOCPE_PKT_MAX_LEN);
        do
            src = random_int(0, NOCPE_PE_NUM - 1);
        while ((inj_lists[src]->size + hw_buffers[src]->size) >= NOCPE_INJ_BUFF_DEPTH);
        do
            dst = random_int(0, NOCPE_PE_NUM - 1);
        while (dst == src);

        NocPe_Pkt_t pkt = nocpe_create_packet(NocPe_Resource.id[src]++, src, dst, len);

        list_push_back(inj_lists[src], &pkt);
    }
}

void nocpe_random_run()
{
    sg_start();

    // random settings
    NocPe_Cyc_t min_time_step = NocPe_Resource.min_time_step;
    NocPe_Cyc_t max_time_step = NocPe_Resource.max_time_step;

    // nocpe vars
    NocPe_Cyc_t max_cyc = NocPe_Resource.max_cyc;
    NocPe_Cyc_t cyc = random_int(0, min_time_step);

    List_t *inj_lists[NOCPE_PE_NUM];
    List_t *hw_buffers[NOCPE_PE_NUM];
    List_t *hw_list = list_init(sizeof(NocPe_Pkt_t));

    for (int i = 0; i < NOCPE_PE_NUM; i++)
    {
        hw_buffers[i] = list_init(sizeof(NocPe_PktCyc_t));
        inj_lists[i] = list_init(sizeof(NocPe_Pkt_t));
    }

    do
    {
        // create random
        if (cyc < max_cyc)
            nocpe_random_create(hw_buffers, inj_lists);

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

        sg_sync_tx(); // after all have been ejected then check the injection is done or not

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

        cyc += random_int(min_time_step, max_time_step);

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
