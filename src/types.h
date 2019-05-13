#pragma once

#include "utils.h"

#define VAR(l) ((size_t) ((l) < 0 ? (-l) : (l)))
#define VAL(l) ((l) <= 0 ? ((l) == 0 ? UNDEFINED : FALSE) : TRUE)
#define NEG(l) (-l)

/* A variable might have one of the following values, in different contexts. */
enum value {
    FALSE, TRUE, UNDEFINED
};

/* A clause is a disjunction of literals. */
struct clause {
    size_t length;
    ssize_t* literals;
};

/* A formula is conjunction of clauses. */
struct formula {
    size_t nvars;
    VMEMBERS(struct clause);
};

struct solution {
    int satisfiable;

    size_t nvars;
    enum value* model;
};
