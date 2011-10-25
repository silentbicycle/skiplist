#include <stdlib.h>
#include <stdio.h>

long allocated = 0;

#define TRACE_ALLOC 0

void *test_malloc(size_t sz) {
    if (TRACE_ALLOC) fprintf(stderr, "alloc %zd bytes\n", sz);
    void *p = malloc(sz);
    allocated += sz;
    return p;
}

void test_free(void *p, size_t sz) {
    if (TRACE_ALLOC) fprintf(stderr, "free %zd bytes\n", sz);
    allocated -= sz;
    free(p);
}

void test_reset() { allocated = 0; }

int test_check_for_leaks() {
    if (allocated != 0)
        fprintf(stderr, "leaked %ld bytes\n", allocated);
    else if (TRACE_ALLOC)
        fprintf(stderr, "0 bytes leaked\n");
    return allocated == 0;
}
