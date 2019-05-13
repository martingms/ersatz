#include <stdlib.h>

#include "utils.h"

void die(const char* msg) {
    fprintf(stderr, "ersatz: %s\n", msg);
    exit(EXIT_FAILURE);
}

void* xmalloc(size_t sz) {
    void* ptr;
    if (!(ptr = malloc(sz)))
        die("unable to malloc, out of memory?");

    return ptr;
}

void* xrealloc(void* ptr, size_t sz) {
    if (!(ptr = realloc(ptr, sz)))
        die("unable to realloc, out of memory?");

    return ptr;
}
