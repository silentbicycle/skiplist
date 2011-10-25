#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "skiplist.h"

typedef struct skiplist skiplist;

#define TIME(name) \
    clock_t timer_##name = clock();             \
    assert(timer_##name != -1);

#define CMP_TIME(label, n1, n2)                         \
    {                                                   \
    clock_t delta = timer_##n2 - timer_##n1;            \
    fprintf(stdout, "%-40s %lu ticks (%.3f secs)\n",    \
        label, (unsigned long) delta, delta / (1.0 * CLOCKS_PER_SEC));   \
    }

#define TDIFF() CMP_TIME(__FUNCTION__, pre, post)

static const int largeish_prime = 7919;

static int intptr_cmp(void *v1, void *v2) {
    intptr_t a = (intptr_t) v1, b = (intptr_t) v2;
    return a < b ? -1 : a > b ? 1 : 0;
}

/* Measure getting existing values (successful lookup). */
static void ins_and_get() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % lim;
        intptr_t v = (intptr_t) skiplist_get(sl, (void *) k);
        if (0) printf("%lu %lu\n", k, v);
        assert(v == k);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

/* Measure getting _nonexistent_ values (lookup failure). */
static void ins_and_get_nonexistent() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) + lim;
        intptr_t v = (intptr_t) skiplist_get(sl, (void *) k);
        assert(v == 0);
        if (0) printf("%lu %lu\n", k, v);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_count() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        assert(skiplist_count(sl) == i);
        skiplist_add(sl, (void *) i, (void *) i);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void set_and_get() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = i % (lim / 2);
        skiplist_set(sl, (void *) k, (void *) k, NULL);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % (lim / 2);
        intptr_t v = (intptr_t) skiplist_get(sl, (void *) k);
        if (0) printf("%lu %lu\n", k, v);
        assert(v == k);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_delete() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % lim;
        intptr_t v = (intptr_t) skiplist_delete(sl, (void *) k);
        if (0) printf("%lu %lu\n", k, v);
        assert(v == k);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_delete_nonexistent() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) + lim;
        intptr_t v = (intptr_t) skiplist_delete(sl, (void *) k);
        if (0) printf("%lu %lu\n", k, v);
        assert(v == 0);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_pop_first() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = 0, v = 0;
        int res = skiplist_pop_first(sl, (void *) &k, (void *) &v);
        assert(res >= 0);
        assert(v == k);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_pop_last() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = 0, v = 0;
        int res = skiplist_pop_last(sl, (void *) &k, (void *) &v);
        assert(res >= 0);
        assert(v == k);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_member() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=1; i < lim; i++) {
        int mem = skiplist_member(sl, (void *) i) == 1;
        /* fprintf(stderr, "%lu: %d\n", i, mem); */
        assert(mem);
    }

    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_clear() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    skiplist_clear(sl, NULL, NULL);
    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static int sum_cb(void *k, void *v, void *ud) {
    intptr_t *p = (intptr_t *) ud;
    *p += (intptr_t) k;
    return 0;
}

static void ins_and_sum() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const int lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    intptr_t total = 0;
    skiplist_iter(sl, &total, sum_cb);
    if (0) fprintf(stderr, "sum: %lu\n", total);
    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

static void ins_and_sum_partway() {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp);
    const intptr_t lim = 100000;

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    intptr_t total = 0;
    skiplist_iter_from(sl, (void *) (lim / 2), &total, sum_cb);
    if (0) fprintf(stderr, "sum: %lu\n", total);
    skiplist_free(sl, NULL, NULL);
    TIME(post);
    TDIFF();
}

int main(int argc, char **argv) {
    TIME(pre);
    ins_and_get();
    ins_and_get_nonexistent();
    ins_and_count();
    set_and_get();
    ins_and_delete();
    ins_and_delete_nonexistent();
    ins_and_pop_first();
    ins_and_pop_last();
    ins_and_member();
    ins_and_clear();
    ins_and_sum();
    ins_and_sum_partway();

    TIME(post);
    fprintf(stdout, "----\n");
    CMP_TIME("total", pre, post);

    return 0;
}
