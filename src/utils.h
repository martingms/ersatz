#pragma once

#include <stdio.h>
#include <stdlib.h>

/* FIXME: This is just lazy, and using them is a blocker to using the whole
 * thing as a library. Drop! */
void* xmalloc(size_t);
void* xrealloc(void*, size_t);

/* Convenience macros for handling any struct containing size_t fields "length"
 * and "capacity", as well as a pointer field "data", as a vector. */
#define VMEMBERS(t) size_t length, capacity; t* data
#define VEC(t) struct { VMEMBERS(t); }

#define VINIT(s, c) do {                         \
    (s)->length = 0;                             \
    (s)->capacity = c;                           \
    (s)->data = xmalloc(sizeof(*(s)->data) * c); \
} while (0)

#define VFREE(s) do {  \
    (s)->capacity = 0; \
    (s)->length = 0;   \
    free((s)->data);   \
} while (0)

#define VPUSH(s, v)                                                 \
do {                                                                \
    if ((s)->length == (s)->capacity) {                             \
        (s)->capacity = (s)->capacity > 0 ? (s)->capacity * 2 : 8;  \
        (s)->data = xrealloc((s)->data, sizeof(v) * (s)->capacity); \
    }                                                               \
    (s)->data[(s)->length++] = v;                                   \
} while (0)

#define VFIT(s) do {                                                     \
    (s)->capacity = (s)->length;                                         \
    (s)->data = xrealloc((s)->data, sizeof(*(s)->data) * (s)->capacity); \
} while (0)

#define VPOP(s) (s)->data[--(s)->length]
#define VPEEK(s) (s)->data[(s)->length - 1]
#define VGET(s, i) (s)->data[i]
