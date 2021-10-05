#include "nocpe_full.h"

void nocpe_full_create(int len, List_t *inj_lists[])
{
    for (int src = 0; src < NOCPE_PE_NUM; src++)
    {
        for (int dst = 0; dst < NOCPE_PE_NUM; dst++)
        {
            if (src == dst)
                continue;

            NocPe_Pkt_t pkt = nocpe_create_packet(src, dst, len);
            list_push_back(inj_lists[src], &pkt);
        }
    }
}

void nocpe_full_run()
{
    sg_start();

    // full settings
    NocPe_Cyc_t time_step = 50;
    uint32_t pkt_len = NOCPE_PKT_MAX_LEN;

    // nocpe vars
    NocPe_Cyc_t MAX_CYC = 1000;
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

    nocpe_full_create(pkt_len, inj_lists);

    do
    {
        // put to hw_buffers whenever its empty and and hw_list for hw injection
        if (cyc < MAX_CYC)
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

        cyc += time_step;

        if (cyc > 2 * MAX_CYC)
            break;
    } while (cyc < MAX_CYC || tot_hw_buffers > 0);

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