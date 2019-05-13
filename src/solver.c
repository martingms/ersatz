#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>

#include "solver.h"

#define DECISION_SENTINEL ((ssize_t) 0)

struct solver {
    struct formula* formula;
    struct solution* solution;

    size_t decision_level;
    struct {
        VMEMBERS(ssize_t);
        size_t head;
    } propq;
};

enum value eval_literal(struct solution* s, ssize_t literal) {
    assert(VAL(literal) != UNDEFINED);
    assert(VAR(literal) <= s->nvars);

    if (s->model[VAR(literal)] == UNDEFINED) return UNDEFINED;

    /* XXX: Is this well defined? */
    return s->model[VAR(literal)] == VAL(literal);
}

static void enqueue(struct solver* s, ssize_t literal) {
    /* We should only be enqueueing unassigned literals. */
    assert(eval_literal(s->solution, literal) == UNDEFINED);
    /* But assigning undefined makes no sense. */
    assert(VAL(literal) != UNDEFINED);

    /* Assign this variable in the model (might be backtracked) */
    s->solution->model[VAR(literal)] = VAL(literal);

    VPUSH(&s->propq, literal);
}

static int propagate(struct solver* s) {
    ssize_t literal, last_undef /*, to_prop */;
    size_t i, j, nundefined;
    int any_true;
    enum value val;
    struct clause clause;

    for (; s->propq.head < s->propq.length; s->propq.head++) {
        /* XXX: Not needed until watchers are added. */
        /* to_prop = vec_get(&f->propq, f->prop_idx); */

        /* XXX: Add watchers! */
        for (i = 0; i < s->formula->length; i++) {
            clause = VGET(s->formula, i);
            any_true = 0;
            nundefined = 0;

            for (j = 0; !any_true && j < clause.length; j++) {
                literal = clause.literals[j];

                val = eval_literal(s->solution, literal);

                if (val == TRUE) {
                    any_true = 1;
                } else if (val == UNDEFINED) {
                    nundefined++;
                    last_undef = literal;
                }
            }

            if (any_true) continue;

            /* Conflict! */
            if (nundefined == 0) return -1;

            /* Unit clause found */
            if (nundefined == 1) enqueue(s, last_undef);
        }
    }

    return 0;
}

static void backtrack(struct solver* s) {
    ssize_t last_decision = 0;

    while (VPEEK(&s->propq) != DECISION_SENTINEL) {
        last_decision = VPOP(&s->propq);
        s->solution->model[VAR(last_decision)] = UNDEFINED;
    }

    /* Pop the sentinel value. */
    (void) VPOP(&s->propq);
    s->decision_level--;
    s->propq.head = s->propq.length;

    /* Let's try again, reversing the polarity this time. */
    assert(last_decision != DECISION_SENTINEL);
    enqueue(s, NEG(last_decision));
}

/* XXX: This is the most naive way to pick possible, add some heuristics! */
static ssize_t pick_branch_literal(struct solver* s) {
    ssize_t out = 0;
    size_t i;
    for (i = 0; i < s->solution->nvars; i++)
        if (s->solution->model[i] == UNDEFINED)
            out = NEG((ssize_t) i);

    /* We might return 0 here, which means undefined, which means we're SAT. */
    return out;
}

int solve(struct formula* f, struct solution* s) {
    int rv;
    size_t i;
    ssize_t next;
    struct solver solver;
    solver.formula = f;
    solver.solution = s;
    /* Literals are 1-indexed, so for simplicity we simply
     * over-allocate instead of shifting everything. */
    s->nvars = f->nvars;
    s->model = xmalloc(sizeof(enum value) * (s->nvars + 1));
    /* All variables initially undefined */
    for (i = 0; i <= s->nvars; i++) s->model[i] = UNDEFINED;

    VINIT(&solver.propq, s->nvars);
    solver.propq.head = 0;
    solver.decision_level = 0;

    /* Handle all trivial clauses (those with 0 or 1 length). */
    for (i = 0; i < solver.formula->length; i++) {
        struct clause c = VGET(solver.formula, i);

        /* By definition. */
        if (c.length == 0) {
            s->satisfiable = 0; rv = -1;
            goto cleanup;
        }

        if (c.length == 1) {
            ssize_t literal = c.literals[0];

            switch (eval_literal(s, literal)) {
            case UNDEFINED:
                /* If propagating this literal leads to conflicts, we have to be
                 * UNSAT. */
                enqueue(&solver, literal);
                if (propagate(&solver)) {
                    s->satisfiable = 0; rv = -1;
                    goto cleanup;
                }

                /* Could remove the clause at this point. */
                break;

            /* This occurs if there are contradictory unit clauses in the input,
             * such as x && !x. In that case the formula is obviously UNSAT. */
            case FALSE:
                s->satisfiable = 0; rv = -1;
                goto cleanup;

            /* An identical unit clause has already been propagated, so it's
             * strictly a superfluous clause. */
            case TRUE: break;
            }
        }
    }

    /* Main loop of the algorithm. */
    for (;;) {
        /* While propagation leads to conflict, backtrack. */
        while (propagate(&solver)) {
            if (solver.decision_level == 0) {
                s->satisfiable = 0; rv = -1;
                goto cleanup;
            }

            backtrack(&solver);
        }

        /* Figure out what assignment to try next. */
        if ((next = pick_branch_literal(&solver)) == 0) {
            s->satisfiable = 1; rv = 0;
            goto cleanup;
        }

        /* Start a new decision level. */
        VPUSH(&solver.propq, DECISION_SENTINEL);
        solver.propq.head++; solver.decision_level++;
        enqueue(&solver, next);
    }

cleanup:
    VFREE(&solver.propq);

    return rv;
}

int verify(struct formula* f, struct solution* s) {
    size_t i, j;
    int any_true;
    struct clause clause;

    for (i = 0; i < f->length; i++) {
        clause = VGET(f, i);
        any_true = 0;

        for (j = 0; j < clause.length; j++) {
            if (eval_literal(s, clause.literals[j]) == TRUE) {
                any_true = 1; break;
            }
        }

        if (!any_true)
            return -1;
    }

    return 0;
}
