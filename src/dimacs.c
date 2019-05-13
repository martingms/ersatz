#include <assert.h>
#include <unistd.h>

#include "dimacs.h"
#include "utils.h"

static void skip_until(FILE* in, char c) {
    int d;
    while ((d = getc(in)) != EOF)
        if (c == d) break;
}

/* Format for both input and output is specified at
 * https://www.cs.ubc.ca/~hoos/SATLIB/Benchmarks/SAT/satformat.ps */

int parse_dimacs_input(FILE* in, struct formula* f) {
    int c, n; size_t nclauses; ssize_t literal;
    VEC(ssize_t) clause_builder; struct clause clause;

    /* Header part */
    while ((c = getc(in)) != EOF) {
        if (c == 'p') {
            if (fscanf(in, " cnf %lu %lu", &f->nvars, &nclauses) != 2)
                return -1;

            /* Ignore the rest of the p-line. */
            skip_until(in, '\n');
            break;
        }

        /* We don't care about any line that doesn't start with p. */
        skip_until(in, '\n');
    }

    /* We've only parsed the header, can't be finished yet! */
    if (feof(in) || ferror(in))
        return -1;

    VINIT(f, nclauses);
    VINIT(&clause_builder, 0);

    /* Clause part */
    for (;;) {
        /* Skip any inline comments. */
        if ((c = fgetc(in)) == 'c')
            skip_until(in, '\n');
        else ungetc(c, in);

        /* Scan for literal, throw away any surrounding whitespace. */
        n = fscanf(in, " %ld ", &literal);

        /* Successfully parsed literal, push it to current clause. */
        if (n == 1 && literal != 0) {
            VPUSH(&clause_builder, literal);

        /* Clause finalized, either by marker (0) or EOF/unparseable token. */
        } else if ((n == 1 && literal == 0) ||
                   (n != 1 && clause_builder.length > 0)) {
            VFIT(&clause_builder);

            /* Add the clause. */
            clause.length = clause_builder.length;
            clause.literals = clause_builder.data;
            VPUSH(f, clause);

            /* Re-init builder for next clause. */
            if (n == 1)
                VINIT(&clause_builder, 0);
        }

        /* EOF or unparseable as literal, either way we bail. */
        if (n != 1) break;
    }

    /* Not really needed, just to keep the valgrind leak checker happy. */
    if (clause_builder.length == 0)
        VFREE(&clause_builder);

    /* XXX: We don't really care, but should maybe issue a warning? */
    assert(nclauses == f->length);

    return 0;
}

int parse_dimacs_output(FILE* in, struct solution* s) {
    int c; ssize_t literal; size_t i, max_var = 0;
    VEC(ssize_t) literals;
    VINIT(&literals, 8);

    while ((c = fgetc(in)) != EOF) {
        switch (c) {
        /* Variable line */
        case 'v':
            while (fscanf(in, " %ld ", &literal) == 1 && literal != 0) {
                if (VAR(literal) > max_var) max_var = VAR(literal);

                VPUSH(&literals, literal);
            }

            break;

        /* Solution line */
        case 's':
            if (fgetc(in) != ' ')
                return -1; /* XXX: goto error */

            /* We cheat, and simply look at the first character. We're really
             * looking for "(UN)?SATISFIABLE". */
            switch (fgetc(in)) {
            case 'U': s->satisfiable = 0; break;
            case 'S': s->satisfiable = 1; break;
            default: return -1; /* XXX: goto error */
            }

            /* Fallthrough! We skip the rest of the s-line. */
        default:
            skip_until(in, '\n'); break;
        }
    }

    /* XXX: Move this inline, drop the literals VEC, or support VSET() with
     * resizing!  */
    s->nvars = max_var + 1;
    s->model = xmalloc(sizeof(enum value) * (s->nvars + 1));

    for (i = 0; i <= s->nvars; i++) s->model[i] = UNDEFINED;
    for (i = 0; i < literals.length; i++) {
        literal = VGET(&literals, i);

        s->model[VAR(literal)] = VAL(literal);
    }

    VFREE(&literals);

    return 0;
}

void compose_dimacs_output(FILE* out, struct solution* s) {
    int p;
    size_t i;

    fprintf(out, "s %sSATISFIABLE\n", s->satisfiable ? "" : "UN");

    if (!s->satisfiable)
        return;

    p = fprintf(out, "v ");
    for (i = 0; i <= s->nvars; i++) {
        if (s->model[i] == UNDEFINED) continue;

        /* Avoid printing too long lines. */
        if (p >= 78) p = fprintf(out, "\nv ");

        p += fprintf(out, "%ld ", s->model[i] == TRUE ? i : -i);
    }

    fprintf(out, "0\n");
}
