#include "nocpe_uniform.h"

void nocpe_uniform_create(uint32_t max_cyc, uint32_t num_interval, float inj_rate, List_t *nocpe_pkt_cyc_list)
{
    uint32_t avg_time_frame = max_cyc / num_interval;
    uint32_t rem_time_frame = max_cyc % num_interval;

    uint32_t tot_num_inj = max_cyc * inj_rate;
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
            pkt_cyc.pkt = nocpe_create_packet(src, dst, 1);

            pkt_cyc.cyc = random_int(sub_cur_cyc, max_time_frame);

            pkt_cycs[j] = pkt_cyc;
        }

        // sort by time
        for (int k = 0; k < num_inj; k++)
        {
            for (int l = k; l < num_inj; l++)
            {
                if (pkt_cycs[l].cyc < pkt_cycs[k].cyc)
                {
                    NocPe_PktCyc_t tmp = pkt_cycs[k];
                    pkt_cycs[k] = pkt_cycs[l];
                    pkt_cycs[l] = tmp;
                }
            }
        }

        list_insert_array(nocpe_pkt_cyc_list, -1, pkt_cycs, num_inj);

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
    uint32_t tot_hw_buffers = 0;

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

        // check whether all pkts are ejected
        tot_hw_buffers = 0; // ? optimization
        for (int i = 0; i < NOCPE_PE_NUM; i++)
            tot_hw_buffers += hw_buffers[i]->size;

        if (cyc < max_cyc && uniform_pkt_cyc_list->size > 0)
            cyc = ((NocPe_PktCyc_t *)list_front(uniform_pkt_cyc_list))->cyc;
        else
            cyc += random_int(1, 100);

        if (cyc > 2 * max_cyc)
            break;
    } while (cyc < max_cyc || tot_hw_buffers > 0);

    printf("[END] num_lost:%i cyc:%i \n", tot_hw_buffers, cyc);
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