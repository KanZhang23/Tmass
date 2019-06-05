#include <stdio.h>
#include <stdlib.h>

void abend_(void);

void abend_(void)
{
    fprintf(stderr, "Fatal error in CERNLIB. Exiting.\n");
    fflush(stderr);
    exit(1);
}
