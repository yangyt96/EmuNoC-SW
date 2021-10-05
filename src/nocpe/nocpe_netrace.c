#include "nocpe_netrace.h"

void nocpe_netrace_create(nt_packet_t nt_packet, List_t *wait_lists[])
{
    uint32_t src = nt_packet.src % NOCPE_PE_NUM;
    uint32_t dst = nt_packet.dst % NOCPE_PE_NUM;
    int32_t size = nt_packet_sizes[nt_packet.type];

    if (src == dst)
        dst = (dst + 1) % NOCPE_PE_NUM;

    while (size > 0)
    {
        NocPe_Pkt_t nocpe_pkt;
        if (size > NOCPE_PKT_MAX_LEN)
            nocpe_pkt = nocpe_create_packet(src, dst, NOCPE_PKT_MAX_LEN);
        else
            nocpe_pkt = nocpe_create_packet(src, dst, size);

        list_push_back(wait_lists[src], &nocpe_pkt);

        size -= NOCPE_PKT_MAX_LEN;
    }
}

/**
 * @brief currently only support no dependency netrace
 *
 */
void nocpe_netrace_run()
{
    sg_start();

    // netrace settings
    char *trace_file = "multiregion.tra";
    int start_region = 1;
    int ignore_dependencies = 1; // ? set to 1 first, later if got update then can change value
    int reader_throttling = 0;   // ? 0

    // netrace vars
    nt_packet_t *netrace_packet = NULL;
    nt_header_t *header;
    nt_context_t *ctx = (nt_context_t *)calloc(1, sizeof(nt_context_t));

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

    // netrace works
    nt_open_trfile(ctx, trace_file);

    if (ignore_dependencies)
        nt_disable_dependencies(ctx);

    // nt_print_trheader(ctx);

    header = nt_get_trheader(ctx);
    nt_seek_region(ctx, &header->regions[start_region]);

    for (int i = 0; i < start_region; i++)
        cyc += header->regions[i].num_cycles;

    if (!ignore_dependencies && reader_throttling)
        nt_init_self_throttling(ctx);

    MAX_CYC += cyc;
    do
    {
        if (cyc < MAX_CYC)
            for (netrace_packet = nt_read_packet(ctx); netrace_packet != NULL && cyc == netrace_packet->cycle; netrace_packet = nt_read_packet(ctx))
                nocpe_netrace_create(*netrace_packet, inj_lists);

        if (netrace_packet->cycle < cyc)
        {
            printf("Error: current simulation cycle %i is smaller than next inject packet cycle %i \n", cyc, netrace_packet->cycle);
            break;
        }

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

        if (cyc < MAX_CYC)
            cyc = netrace_packet->cycle;
        else
            cyc += 100;

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

    nt_close_trfile(ctx);

    sg_stop();
}