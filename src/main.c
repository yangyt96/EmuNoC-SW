#include <argp.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "nocpe/nocpe_full.h"
#include "nocpe/nocpe_random.h"
#include "nocpe/nocpe_netrace.h"
#include "nocpe/nocpe_uniform.h"

const char *argp_program_version = "NoC Emultation 1.0";
const char *argp_program_bug_address = "<yee.yang.tan@rwth-aachen.de>";
static char doc[] = "";
static char args_doc[] = "";
static struct argp_option options[] = {

    // mode
    {"full", 'F', 0, 0, "Full mode - All PEs send to all PEs.", 0},
    {"random", 'R', 0, 0, "Random mode - Random PEs send to random PEs.", 0},
    {"netrace", 'N', 0, 0, "Netrace mode - Read the *.tra file and inject according to the provided data.", 0},
    {"uniform", 'U', 0, 0, "Uniform mode - Inject the packet according the the injection rate with randomize usage.", 0},

    // General
    {"max-cyc", 'C', "1000", 0, "Max running cycle.", 1},
    {"seed", 'S', "0", 0, "Seed value for random.", 1},
    {"outdir", 'O', "./", 0, "Output directory.", 1},

    // full
    {"time-step", 't', "50", 0, "Mode Full - time step.", 2},
    {"pkt-len", 'p', "31", 0, "Mode Full - packet length.", 2},

    // random
    {"min-time-step", 'l', "10", 0, "Mode Random - minimum time step.", 3},
    {"max-time-step", 'u', "30", 0, "Mode Random - maximum time step.", 3},

    // netrace
    {"trace-file", 'f', "./multiregion.tra", 0, "Mode Netrace - trace file *.tra path.", 4},
    {"start-region", 's', "0", 0, "Mode Netrace - starting region.", 4},
    {"ignore-dependencies", 'i', "1", 0, "Mode Netrace - ignore the dependencies between packets.", 4},
    {"reader-throttling", 'd', "0", 0, "Mode Netrace - reader throttling.", 4},

    // uniform
    {"num-interval", 'n', "100", 0, "Mode Uniform - to separate the max cycle into multiple region for better injection distribution.", 5},
    {"inj-rate", 'r', "0.01", 0, "Mode Uniform - the injection rate.", 5},

    {0}};

typedef NocPe_Resource_t Arguments_t;

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    printf("%i %c %s \n", key, key, arg);

    switch (key)
    {
    // mode
    case 'F':
        NocPe_Resource.mode = "full";
        break;
    case 'R':
        NocPe_Resource.mode = "random";
        break;
    case 'N':
        NocPe_Resource.mode = "netrace";
        break;
    case 'U':
        NocPe_Resource.mode = "uniform";
        break;

    // general
    case 'C':
        NocPe_Resource.max_cyc = atoi(arg);
        break;
    case 'S':
        NocPe_Resource.seed = atoi(arg);
        break;
    case 'O':
        NocPe_Resource.outdir = arg;
        break;

    // full
    case 't':
        NocPe_Resource.time_step = atoi(arg);
        break;
    case 'p':
        NocPe_Resource.pkt_len = atoi(arg);
        break;

    // random
    case 'l':
        NocPe_Resource.min_time_step = atoi(arg);
        break;
    case 'u':
        NocPe_Resource.max_time_step = atoi(arg);
        break;

    // netrace
    case 'f':
        NocPe_Resource.trace_file = arg;
        break;
    case 's':
        NocPe_Resource.start_region = atoi(arg);
        break;
    case 'i':
        NocPe_Resource.ignore_dependencies = atoi(arg);
        break;
    case 'd':
        NocPe_Resource.reader_throttling = atoi(arg);
        break;

    // uniform
    case 'n':
        NocPe_Resource.num_interval = atoi(arg);
        break;
    case 'r':
        NocPe_Resource.inj_rate = atof(arg);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

int main(int argc, char *argv[])
{
    // general
    NocPe_Resource.max_cyc = 1000;
    NocPe_Resource.seed = time(0);
    NocPe_Resource.outdir = "./";
    NocPe_Resource.mode = "full";

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

    // uniform
    NocPe_Resource.inj_rate = 0.1;
    NocPe_Resource.num_interval = 20;

    // parse arguments
    argp_parse(&argp, argc, argv, 0, 0, &NocPe_Resource);
    srand(NocPe_Resource.seed);

    if (!strcmp(NocPe_Resource.mode, "full"))
        nocpe_full_run();
    else if (!strcmp(NocPe_Resource.mode, "random"))
        nocpe_random_run();
    else if (!strcmp(NocPe_Resource.mode, "netrace"))
        nocpe_netrace_run();
    else if (!strcmp(NocPe_Resource.mode, "uniform"))
        nocpe_uniform_run();

    return EXIT_SUCCESS;
}