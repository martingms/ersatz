#include <stdio.h>
#include <unistd.h>

#include "types.h"
#include "solver.h"
#include "dimacs.h"

#ifndef VERSION
#define VERSION "0000"
#endif

int main(int argc, char* argv[]) {
    int c, rv;
    size_t i;
    FILE* in = NULL; FILE* vin = NULL;
    struct formula f;
    struct solution s;

    /* To make cleaning up easier. */
    f.data = NULL;
    s.model = NULL;

    while ((c = getopt(argc, argv, "V:vh")) != -1) {
        switch (c) {
        case 'V':
            if (*optarg == '-' && *++optarg == '\0') {
                vin = stdin;
                break;
            }

            if (!(vin = fopen(optarg, "r"))) {
                perror(optarg);
                goto error;
            }

            break;

        case 'v':
            fprintf(stderr, "ersatz %s\n", VERSION);
            rv = EXIT_SUCCESS;
            goto cleanup;

        case 'h': default:
            goto usage;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) goto usage;

    if (!(in = fopen(*argv, "r"))) {
        perror(*argv);
        goto error;
    }

    /* Whether we're solving or verifying, we need to parse the CNF formula. */
    if (parse_dimacs_input(in, &f)) {
        fprintf(stderr, "Unable to parse DIMACS cnf file\n");
        goto error;
    }

    /* Called with -V, we're just going to verify the provided model. */
    if (vin) {
        if (parse_dimacs_output(vin, &s)) {
            fprintf(stderr, "Unable to parse DIMACS output file\n");
            goto error;
        }

        if (!s.satisfiable) {
            fprintf(stderr, "We're not able to verify UNSAT proofs yet.\n");
            goto error;
        }

        if (verify(&f, &s)) {
            fprintf(stderr, "Verification failed!\n");
            goto error;
        }

        rv = EXIT_SUCCESS;

    /* Called with just a CNF file, attempting to solve. */
    } else {
        rv = solve(&f, &s) ? EXIT_FAILURE : EXIT_SUCCESS;
        compose_dimacs_output(stdout, &s);
    }

    goto cleanup;

usage:
    fprintf(stderr, "usage: ersatz [-v] [-V solutionfile] cnffile\n");

error:
    rv = EXIT_FAILURE;

cleanup:
    if (in) fclose(in);
    if (vin) fclose(vin);

    /* Free all clauses. */
    for (i = 0; i < f.length; i++)
        free(VGET(&f, i).literals);

    VFREE(&f);
    free(s.model);

    return rv;
}
