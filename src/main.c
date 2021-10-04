#include <argp.h>

#include "nocpe_full.h"
#include "nocpe_random.h"
#include "nocpe_netrace.h"

int main(int argc, char *argv[])
{

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