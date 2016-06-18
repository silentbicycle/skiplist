#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "test_alloc.h"

long allocated = 0;

#define TRACE_ALLOC 0

void *test_alloc(void *p, size_t osize, size_t nsize, void *udata) {
    (void)udata;
    if (p) {
        assert(nsize == 0);
        if (TRACE_ALLOC) { fprintf(stderr, "free %zd bytes\n", osize); }
        allocated -= osize;
        free(p);
        return NULL;
    } else {
        if (TRACE_ALLOC) { fprintf(stderr, "alloc %zd bytes\n", nsize); }
        assert(osize == 0);
        p = malloc(nsize);
        allocated += nsize;
        return p;
    }
}

void *test_malloc(size_t sz) {
    return test_alloc(NULL, 0, sz, NULL);
}

void test_free(void *p, size_t sz) {
    (void)test_alloc(p, sz, 0, NULL);
}


void test_reset(void) { allocated = 0; }

int test_check_for_leaks(void) {
    if (allocated != 0)
        fprintf(stderr, "leaked %ld bytes\n", allocated);
    else if (TRACE_ALLOC)
        fprintf(stderr, "0 bytes leaked\n");
    return allocated == 0;
}
