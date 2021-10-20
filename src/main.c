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

#include <argp.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "nocpe/nocpe_full.h"
#include "nocpe/nocpe_random.h"
#include "nocpe/nocpe_netrace.h"
#include "nocpe/nocpe_uniform.h"

const char *argp_program_version = "NoC Emultator 1.0";
const char *argp_program_bug_address = "<yee.yang.tan@rwth-aachen.de>";
static char doc[] = "";
static char args_doc[] = "";
static struct argp_option options[] = {

    // mode
    {"full", 'F', 0, 0, "Full mode - All PEs send to all PEs.", 0},
    {"random", 'R', 0, 0, "Random mode - Random PEs send to random PEs.", 0},
    {"netrace", 'N', 0, 0, "Netrace mode - Read the *.tra file and inject according to the provided data.", 0},
    {"uniform", 'U', 0, 0, "Uniform mode - Inject the packet according the the injection rate with uniform randomize usage.", 0},
    {"empty", 'E', 0, 0, "Empty mode - Reset/Empty the NoC hardware if the error occurs.", 0},

    // General
    {"max-cyc", 'C', "1000", 0, "Max running cycle.", 1},
    {"seed", 'S', "0", 0, "Seed value for random.", 1},
    {"output", 'O', "./{mode}_{settings}.csv", 0, "Output file path, it will automatically generate according to the mode.", 1},
    {"disable", 'D', 0, 0, "Disable the output file, so that the program won't write the output file.", 1},
    {"verbose", 'V', "0", 0, "Print the inject and eject info, 0 for nothing, 1 for csv format, 2 for details.", 1},

    // full
    {"time-step", 't', "50", 0, "Mode Full - time step.", 2},
    {"pkt-len", 'p', "1", 0, "Mode Full/Uniform - packet length.", 2},

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

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
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
    case 'E':
        NocPe_Resource.mode = "empty";
        break;

    // general
    case 'C':
        NocPe_Resource.max_cyc = atoi(arg);
        break;
    case 'S':
        NocPe_Resource.seed = atoi(arg);
        break;
    case 'O':
        NocPe_Resource.output = arg;
        break;
    case 'D':
        NocPe_Resource.disable = true;
        break;
    case 'V':
        NocPe_Resource.verbose = atoi(arg);
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
    memset(&NocPe_Resource, 0, sizeof(NocPe_Resource_t));

    // general
    NocPe_Resource.max_cyc = 1000;
    NocPe_Resource.seed = time(0);
    NocPe_Resource.output = NULL;
    NocPe_Resource.mode = "random";

    // full
    NocPe_Resource.pkt_len = 1;
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
    argp_parse(&argp, argc, argv, 0, 0, NULL);
    srand(NocPe_Resource.seed);

    if (NocPe_Resource.disable != true)
        nocpe_csv_wopen();

    if (!strcmp(NocPe_Resource.mode, "full"))
        nocpe_full_run();
    else if (!strcmp(NocPe_Resource.mode, "random"))
        nocpe_random_run();
    else if (!strcmp(NocPe_Resource.mode, "netrace"))
        nocpe_netrace_run();
    else if (!strcmp(NocPe_Resource.mode, "uniform"))
        nocpe_uniform_run();
    else if (!strcmp(NocPe_Resource.mode, "empty"))
        nocpe_empty();

    if (NocPe_Resource.disable != true)
        nocpe_csv_wclose();

    return EXIT_SUCCESS;
}