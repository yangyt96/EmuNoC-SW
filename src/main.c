#include <argp.h>
#include <time.h>

#include "nocpe/nocpe_full.h"
#include "nocpe/nocpe_random.h"
#include "nocpe/nocpe_netrace.h"
#include "nocpe/nocpe_uniform.h"

int main(int argc, char *argv[])
{

    // general
    NocPe_Resource.max_cyc = 1000;
    NocPe_Resource.seed = time(0);
    srand(NocPe_Resource.seed);

    // full
    NocPe_Resource.pkt_len = NOCPE_PKT_MAX_LEN;
    NocPe_Resource.time_step = 50;

    // random
    NocPe_Resource.min_time_step = 10;
    NocPe_Resource.max_time_step = 30;

    // netrace
    NocPe_Resource.trace_file = "multiregion.tra";
    NocPe_Resource.start_region = 0;
    NocPe_Resource.ignore_dependencies = 1;
    NocPe_Resource.reader_throttling = 0;

    if (argc == 1)
        nocpe_full_run();
    else if (!strcmp(argv[1], "full"))
        nocpe_full_run();
    else if (!strcmp(argv[1], "random"))
        nocpe_random_run();
    else if (!strcmp(argv[1], "netrace"))
        nocpe_netrace_run();

    return EXIT_SUCCESS;
}