#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include "skiplist.h"

#include <sys/time.h>

typedef struct skiplist skiplist;

static const size_t usec_per_sec = 1000000L;

static size_t get_usec_delta(struct timeval *pre, struct timeval *post) {
    return (usec_per_sec * (post->tv_sec - pre->tv_sec)
        + (post->tv_usec - pre->tv_usec));
}

#define TIME(name)                                                      \
        struct timeval timer_##name = { 0, 0 };                         \
        int timer_res_##name = gettimeofday(&timer_##name, NULL);       \
        (void)timer_res_##name;                                         \
        assert(0 == timer_res_##name);                                  \

#define CMP_TIME(label, n1, n2)                                         \
    do {                                                                \
        size_t usec_delta = get_usec_delta(&timer_##n1, &timer_##n2);   \
        double usec_per = usec_delta / (double)lim;                     \
        double per_second = usec_per_sec / usec_per;                    \
        printf("%-30s limit %zd %9.3f msec, %6.3f usec per, "           \
            "%11.3f K ops/sec\n",                                       \
            label, lim, usec_delta / (double)1000,                      \
            usec_per, per_second / 1000);                               \
    } while(0)                                                          \

#define TDIFF() CMP_TIME(__FUNCTION__, pre, post)

#define DEF_LIM 100000

static const int largeish_prime = 7919;
static long lim = DEF_LIM;

static int intptr_cmp(void *v1, void *v2) {
    intptr_t a = (intptr_t) v1, b = (intptr_t) v2;
    return a < b ? -1 : a > b ? 1 : 0;
}

/* Measure insertions. */
static void ins(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

/* Measure getting existing values (successful lookup). */
static void get(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % lim;
        intptr_t v = 0;
        skiplist_get(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == k);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

/* Measure getting _nonexistent_ values (lookup failure). */
static void get_nonexistent(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) + lim;
        intptr_t v = 0;
        skiplist_get(sl, (void *) k, (void **)&v);
        assert(v == 0);
        if (0) { printf("%lu %lu\n", k, v); }
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

/* Measure getting existing values (successful lookup). */
static void ins_and_get(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % lim;
        intptr_t v = 0;
        skiplist_get(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == k);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

/* Measure getting _nonexistent_ values (lookup failure). */
static void ins_and_get_nonexistent(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) + lim;
        intptr_t v = 0;
        skiplist_get(sl, (void *) k, (void **)&v);
        assert(v == 0);
        if (0) { printf("%lu %lu\n", k, v); }
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_count(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        assert(skiplist_count(sl) == i);
        skiplist_add(sl, (void *) i, (void *) i);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void set(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = i % (lim / 2);
        skiplist_set(sl, (void *) k, (void *) k, NULL);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void set_and_get(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = i % (lim / 2);
        skiplist_set(sl, (void *) k, (void *) k, NULL);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % (lim / 2);
        intptr_t v = (intptr_t)0;
        skiplist_get(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == k);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void delete(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % lim;
        intptr_t v = 0;
        skiplist_delete(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == k);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_delete(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) % lim;
        intptr_t v = 0;
        skiplist_delete(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == k);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void delete_nonexistent(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) + lim;
        intptr_t v = 0;
        skiplist_delete(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == 0);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_delete_nonexistent(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = (i * largeish_prime) + lim;
        intptr_t v = 0;
        skiplist_delete(sl, (void *) k, (void **)&v);
        if (0) { printf("%lu %lu\n", k, v); }
        assert(v == 0);
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_pop_first(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = 0, v = 0;
        int res = skiplist_pop_first(sl, (void *) &k, (void *) &v);
        assert(res >= 0);
        assert(v == k);
        (void) res;
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_pop_last(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = 0, v = 0;
        int res = skiplist_pop_last(sl, (void *) &k, (void *) &v);
        assert(res >= 0);
        assert(v == k);
        (void) res;
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void pop_first(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = 0, v = 0;
        int res = skiplist_pop_first(sl, (void *) &k, (void *) &v);
        assert(res >= 0);
        assert(v == k);
        (void) res;
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void pop_last(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        intptr_t k = 0, v = 0;
        int res = skiplist_pop_last(sl, (void *) &k, (void *) &v);
        assert(res >= 0);
        assert(v == k);
        (void) res;
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_member(void) {
    TIME(pre);
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    for (intptr_t i=1; i < lim; i++) {
        int mem = skiplist_member(sl, (void *) i) == 1;
        /* fprintf(stderr, "%lu: %d\n", i, mem); */
        assert(mem);
        (void) mem;
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void member(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    for (intptr_t i=1; i < lim; i++) {
        int mem = skiplist_member(sl, (void *) i) == 1;
        /* fprintf(stderr, "%lu: %d\n", i, mem); */
        assert(mem);
        (void) mem;
    }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_clear(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    skiplist_clear(sl, NULL, NULL);
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static enum skiplist_iter_res sum_cb(void *k, void *v, void *ud) {
    (void)v;
    intptr_t *p = (intptr_t *) ud;
    *p += (intptr_t) k;
    return SKIPLIST_ITER_CONTINUE;
}

static void sum(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    TIME(pre);
    intptr_t total = 0;
    skiplist_iter(sl, sum_cb, &total);
    if (0) { fprintf(stderr, "sum: %lu\n", total); }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_sum(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    intptr_t total = 0;
    skiplist_iter(sl, sum_cb, &total);
    if (0) { fprintf(stderr, "sum: %lu\n", total); }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

static void ins_and_sum_partway(void) {
    skiplist *sl = skiplist_new(intptr_cmp, NULL, NULL);

    TIME(pre);
    for (intptr_t i=0; i < lim; i++) {
        skiplist_add(sl, (void *) i, (void *) i);
    }

    intptr_t total = 0;
    skiplist_iter_from(sl, (void *) (lim / 2), sum_cb, &total);
    if (0) { fprintf(stderr, "sum: %lu\n", total); }
    TIME(post);

    TDIFF();
    skiplist_free(sl, NULL, NULL);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        lim = atol(argv[1]);
        if (lim <= 1) {
            fprintf(stderr, "Bad limit.\nUsage: bench [LIMIT]\n");
            exit(1);
        }
    } else {
        lim = DEF_LIM;
    }

    TIME(pre);
    ins();
    get();
    get_nonexistent();
    set();
    delete();
    delete_nonexistent();
    ins_and_get();
    ins_and_get_nonexistent();
    ins_and_count();
    set_and_get();
    ins_and_delete();
    ins_and_delete_nonexistent();
    pop_first();
    pop_last();
    ins_and_pop_first();
    ins_and_pop_last();
    member();
    ins_and_member();
    ins_and_clear();
    sum();
    ins_and_sum();
    ins_and_sum_partway();

    TIME(post);
    double usec_total = (double)get_usec_delta(&timer_pre, &timer_post);
    printf("----\n%-30s %.3f sec\n", "total", usec_total / usec_per_sec);

    return 0;
}
