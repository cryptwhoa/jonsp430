#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "define.h"
#include "util.h"
#include "state.h"
#include "cli.h"

const char USAGE[] = "usage: %s <file>\n";

int main (int argc, char *argv[]) {
        if (argc < 2) {
                fprintf (stderr, USAGE, argv[0]);
                return -1;
        }

        char asm_filename[MAX_FILENAME_LENGTH] = "";
        int seed = 0;

        int opt;
        while ((opt = getopt(argc, argv, "s:")) != -1) {
                switch(opt) {
                case 's': {
                        seed = atoi(optarg);
                        break; }
                default:
                        fprintf(stderr, USAGE, argv[0]);
                        return -1;
                }
        }

        if (optind == argc) {
                fprintf(stderr, USAGE, argv[0]);
                return -1;
        }

        if (strlen(argv[argc - 1]) >= MAX_FILENAME_LENGTH) {
                printf ("filename %s is too long\n", argv[argc - 1]);
                return -1;
        }

        strcpy(asm_filename, argv[argc - 1]);
        if (!seed) {
                seed = time(NULL);
        }

        struct state *context = init_state(asm_filename, seed);
        if (context == NULL) {
                fprintf(stderr, "error constructing initial state");
                return -1;
        }

        cli_start(context);

        free (context);
        return 0;
}
