#include "nocpe_neuromorphic.h"
//#include <float.h>

struct Table
{
    uint32_t src;
    uint32_t dst;
    double rate;
};

uint32_t get_noc_csv_num_row(char *fname)
{

    FILE *fp;
    uint32_t count = 0; // Line counter (result)
    char c;             // To store a character read from file

    // Open the file
    fp = fopen(fname, "r");

    // Check if file exists
    if (fp == NULL)
    {
        printf("Could not open file %s", fname);
        return 0;
    }

    // Extract characters from file and store in character c
    for (c = getc(fp); feof(fp) != true; c = getc(fp))
    {
        printf("%c", c);
        if (c == '\n') // Increment count if this character is newline
            count++;
    }

    // Close the file
    fclose(fp);

    return count;
}

uint32_t get_noc_csv_data(char *fname, struct Table *table)
{

    FILE *fp;
    char row[1000];
    char *token;
    uint32_t table_ptr = 0;

    fp = fopen(fname, "r");
    fgets(row, 1000, fp); // skip 0th column name
    fgets(row, 1000, fp); // read 1st row data
    for (uint32_t j = 0; feof(fp) != true; j++, fgets(row, 1000, fp))
    {
        token = strtok(row, ",");

        uint32_t src;
        uint32_t dst;
        double weight;
        char *ptr;
        char tmp[50];
        for (uint32_t i = 0; token != NULL; i++)
        {
            switch (i)
            {
            case 0:
                src = atoi(token);
                break;
            case 3:
                dst = atoi(token);
                break;
            case 6:
                weight = atof(token);
                break;
            }
            token = strtok(NULL, ",");
        }

        if (src == dst || weight == 0)
            continue;

        table[table_ptr].src = src;
        table[table_ptr].dst = dst;
        table[table_ptr].rate = weight * 30 / pow(10, 9) * (1 - NocPe_Resource.sparsity); // convert weight to injection rate = weight * 30fps = weight * 30 / (10e9 ns)
        table_ptr++;
    }
    fclose(fp);

    return table_ptr;
}

uint32_t calculate_all_num_pkt(uint32_t max_cyc, struct Table table[], uint32_t num_row)
{
    uint32_t tot = 0;
    for (uint32_t i = 0; i < num_row; i++)
        tot += (uint32_t)((double)max_cyc * table[i].rate);
    return tot;
}

void nocpe_neuromorphic_create(uint32_t max_cyc, struct Table table[], uint32_t num_row, NocPe_PktCyc_t *pkt_cycs)
{
    uint32_t tot_num_inj[num_row];
    for (uint32_t i = 0; i < num_row; i++)
        tot_num_inj[i] = max_cyc * table[i].rate;

    uint32_t cnt = 0;
    for (uint32_t i = 0; i < num_row; i++)
    {
        uint32_t tot_num_inj = max_cyc * table[i].rate;

        for (uint32_t j = 0; j < tot_num_inj; j++)
        {
            NocPe_PktCyc_t pkt_cyc;
            pkt_cyc.pkt = nocpe_create_packet(NocPe_Resource.id[table[i].src]++, table[i].src, table[i].dst, NocPe_Resource.pkt_len);
            pkt_cyc.cyc = random_int(0, max_cyc);
            pkt_cycs[cnt++] = pkt_cyc;
        }
    }

    qsort(pkt_cycs, cnt, sizeof(NocPe_PktCyc_t), nocpe_cyc_cmp);
}

void nocpe_neuromorphic_run()
{
    sg_start();

    // neuromorphic settings
    char *fname = NocPe_Resource.neuro_file;

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

    // create neuromorphic data
    struct Table table[get_noc_csv_num_row(fname)];

    uint32_t num_row = get_noc_csv_data(fname, table);

    for (uint32_t i = 0; i < num_row; i++)
        printf("%i src:%i dst:%i rate:%e \n", i, table[i].src, table[i].dst, table[i].rate);

    uint32_t num_pkt = calculate_all_num_pkt(max_cyc, table, num_row);
    NocPe_PktCyc_t *pkt_cycs = malloc(sizeof(NocPe_PktCyc_t) * num_pkt);
    nocpe_neuromorphic_create(max_cyc, table, num_row, pkt_cycs);

    printf("Done create data: %i \n", num_pkt);

    uint64_t pkt_cyc_ptr = 0;

    do
    {
        if (cyc < max_cyc)
            while (pkt_cyc_ptr < num_pkt)
            {
                NocPe_PktCyc_t *pkt_cyc = &pkt_cycs[pkt_cyc_ptr];
                if (pkt_cyc->cyc != cyc)
                    break;

                list_push_back(inj_lists[pkt_cyc->pkt.src], &(pkt_cyc->pkt));

                pkt_cyc_ptr++;
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

        if (cyc < max_cyc && pkt_cyc_ptr < num_pkt)
            cyc = pkt_cycs[pkt_cyc_ptr].cyc;
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
    free(pkt_cycs);

    sg_stop();
}