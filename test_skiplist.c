#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <assert.h>

#include "skiplist.h"
#include "minunit.h"
#include "test_alloc.h"
#include "test_config.h"

#define TC(name) static char *name()
#define A(t) mu_assert(#t, t)
#define R return

int tests_run = 0;
static int verbose = 1;
static long global_seed = 23;

typedef struct skiplist SL;

static int sl_strcmp(void *a, void *b) {
    return strcmp((char *) a, (char *) b);
}

static int sl_longcmp(void *la, void *lb) {
    long a = (long) la;
    long b = (long) lb;
    int res = a < b ? -1 : a > b ? 1 : 0;
    if (0) fprintf(stderr, "comparing %ld and %ld: %d\n", a, b, res);
    return res;
}


/*************/
/* Test data */
/*************/

const char *wordlist[] = {
#include "test_words.h"
};


/*********/
/* Tests */
/*********/

#define BEGIN_TEST() test_reset()
#define END_TEST() A(test_check_for_leaks()); R NULL

#define INIT() SL *sl = skiplist_new(sl_strcmp); A(sl)
#define FREE_NN() skiplist_free(sl, NULL, NULL)

TC(init) {
    BEGIN_TEST();
    INIT();
    A(sl);
    FREE_NN();
    END_TEST();
}

/* Adding one word should succeed. */
TC(add_one_word) {
    BEGIN_TEST();
    INIT();
    A(skiplist_add(sl, "foo", "bar") >= 0);
    FREE_NN();
    END_TEST();
}

/* This is a quick and dirty test that (for severaral random seeds)
 * the expectation for the distribution of levels generally holds.
 * This could be replaced by a more formal statistical analysis, and
 * should also be modified if the user supplies their own function. */
TC(level_statistical_distribution) {
    BEGIN_TEST();
    int counts[SKIPLIST_MAX_HEIGHT];
    for(int i=0; i<SKIPLIST_MAX_HEIGHT; i++) counts[i] = 0;
    for (long lseed=1; lseed < 1000; lseed++) {
        int counted=0, in_bounds = 0;
        for (long trials=0; trials < 100000; trials++)
            counts[SKIPLIST_GEN_HEIGHT()]++;

        for(int i=1; i<SKIPLIST_MAX_HEIGHT; i++) {
            if (counts[i - 1] != 0 && counts[i] != 0) {
                counted++;
                double ratio = counts[i] / (1.0 * counts[i - 1]);
                if (ratio >= 0.3 && ratio <= 0.75)
                    in_bounds++;
                else
                    in_bounds--;
            }
        }
        if (0) printf("counted is %d, in_bounds is %d\n", counted, in_bounds);
        A(in_bounds > counted/4);
    }

    END_TEST();
    srandom(global_seed);
}

static void print_key_cb(FILE *f, void *k, void *v, void *ud) {
    /* fprintf(f, "\n\n k is %p\n", k); */
    if (k == NULL) k = "(NULL)";
    fprintf(f, "'%s'", (char *) k);
}

static void print_intptr_cb(FILE *f, void *k, void *v, void *ud) {
    fprintf(f, "'%ld'", (intptr_t) k);
}

/* Add a bunch of words to a skiplist and check that the count
 * remains correct. */
TC(fill_with_words) {
    BEGIN_TEST();
    INIT();
    char **w = NULL;
    long ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (verbose > 1) printf("adding word '%s'\n", *w);
        A(skiplist_add(sl, *w, *w) >= 0);
        ct++;
        A(skiplist_count(sl) == ct);
    }

    if (verbose > 1)
        skiplist_debug(sl, stderr, NULL, print_key_cb);
    
    FREE_NN();
    END_TEST();
}

typedef struct cb_udata {
    int *count;
    const char *prev;
    int ok;
} cb_udata;

static int sl_count_and_check_sorted_cb(void *k, void *v, void *udata) {
    cb_udata *ud = (cb_udata *) udata;
    char *word = (char *) k;
    if (0) fprintf(stderr, "ct=%d, prev='%s', cur='%s'\n",
        *ud->count, ud->prev, word);
    
    /* If set, check that prev is <= current word. */
    if (ud->prev && strcmp(ud->prev, word) > 0) { ud->ok = 0; R 1; }
    
    ud->prev = (char *) k;
    (*ud->count)++;
    R 0;
}

/* Add words and check that they are automatically sorted during insertion. */
TC(fill_with_words_check_sorted) {
    BEGIN_TEST();
    INIT();
    char **w = NULL;
    long ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (verbose > 1) printf("word is '%s'\n", *w);
        A(skiplist_add(sl, *w, *w) >= 0);
        ct++;
        A(skiplist_count(sl) == ct);
    }
    
    cb_udata udata;
    int count = 0;
    udata.count = &count;
    udata.prev = NULL;
    udata.ok = 1;
    
    /* iter and verify they're ordered alphabetically, ascending */
    A(skiplist_iter(sl, &udata, sl_count_and_check_sorted_cb) == 0);
    A(udata.ok);
    A(count == ct);
    
    FREE_NN();
    END_TEST();
}

static int sl_check_and_terminate_early_cb(void *k, void *v, void *udata) {
    cb_udata *ud = (cb_udata *) udata;
    char *word = (char *) k;
    if (0) fprintf(stderr, "ct=%d, prev='%s', cur='%s'\n",
        *ud->count, ud->prev, word);
    
    /* If set, check that prev is <= current word. */
    if (ud->prev && strcmp(ud->prev, word) > 0) { ud->ok = 0; R 1; }
    
    ud->prev = (char *) k;
    (*ud->count)++;
    R (0 == strcmp("onion", word));
}

/* Same as previous test, but scan from the beginning to "onion". */
TC(fill_with_words_terminate_early) {
    BEGIN_TEST();
    INIT();
    char **w = NULL;
    long ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (verbose > 1) printf("word is '%s'\n", *w);
        A(skiplist_add(sl, *w, *w) >= 0);
        ct++;
        A(skiplist_count(sl) == ct);
    }
    
    cb_udata udata;
    int count = 0;
    udata.count = &count;
    udata.prev = NULL;
    udata.ok = 1;
    
    /* iter and verify they're ordered alphabetically, quitting
     * at the word "onion". */
    A(skiplist_iter(sl, &udata, sl_check_and_terminate_early_cb) == 1);
    A(udata.ok);
    A(count == 105);
    
    FREE_NN();
    END_TEST();
}

/* Add a bunch of words, and count the number of words starting from the
 * first occurrence of "onion". Tests that it counts from the _first_;
 * "onion" appears multiple times as a key. */
TC(fill_with_words_count_from_onion) {
    BEGIN_TEST();
    INIT();
    char **w = NULL;
    long ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (verbose > 1) printf("word is '%s'\n", *w);
        A(skiplist_add(sl, *w, *w) >= 0);
        ct++;
        A(skiplist_count(sl) == ct);
    }
    
    cb_udata udata;
    int count = 0;
    udata.count = &count;
    udata.prev = NULL;
    udata.ok = 1;
    
    /* iter and verify they're ordered alphabetically, starting
     * at the word "onion". */
    A(skiplist_iter_from(sl, "onion", &udata,
            sl_count_and_check_sorted_cb) == 0);
    A(udata.ok);
    if (verbose > 1) {
        printf("count is %d\n", count);
        skiplist_debug(sl, stdout, NULL, print_key_cb);
    }
    A(count == 62);
    
    FREE_NN();
    END_TEST();
}

/* Test membership for newly added words. */
TC(add_and_member) {
    BEGIN_TEST();
    INIT();
    A(skiplist_empty(sl) == 1);
    A(skiplist_count(sl) == 0);
    A(skiplist_member(sl, "foo") == 0);

    A(skiplist_add(sl, "foo", "foo") >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);
    A(skiplist_empty(sl) == 0);

    FREE_NN();
    END_TEST();
}

/* Add word, replace word, get old value. */
TC(add_and_set) {
    BEGIN_TEST();
    INIT();
    A(skiplist_count(sl) == 0);
    A(skiplist_member(sl, "foo") == 0);

    /* Add "foo" -> "foo" */
    A(skiplist_add(sl, "foo", "foo") >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);
    char *v = skiplist_get(sl, "foo");
    A(v);
    A(0 == strcmp(v, "foo"));
    
    /* Set "foo" -> "bar", check old value */
    char *old = NULL;
    A(skiplist_set(sl, "foo", "bar", (void **) &old) >= 0);
    A(old);
    A(0 == strcmp(old, "foo"));
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);
    v = skiplist_get(sl, "foo");
    A(v);
    A(0 == strcmp(v, "bar"));

    FREE_NN();
    END_TEST();
}

/* Set a word in an empty skiplist, check invariants. */
TC(set) {
    BEGIN_TEST();
    INIT();
    A(skiplist_count(sl) == 0);
    A(skiplist_member(sl, "foo") == 0);

    /* Set "foo" -> "bar", check old value is NULL & count changes. */
    char *old = NULL;
    A(skiplist_set(sl, "foo", "bar", (void **) &old) >= 0);
    A(old == NULL);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);
    char *v = skiplist_get(sl, "foo");
    A(v);
    A(0 == strcmp(v, "bar"));

    FREE_NN();
    END_TEST();
}

/* Set and delete in an empty skiplist, check invariants. */
TC(trivial_delete) {
    BEGIN_TEST();
    INIT();
    A(skiplist_count(sl) == 0);
    A(skiplist_member(sl, "foo") == 0);

    A(skiplist_set(sl, "foo", "bar", NULL) >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);

    char *old = skiplist_delete(sl, "foo");
    A(old);
    A(0 == strcmp(old, "bar"));
    A(skiplist_member(sl, "foo") == 0);
    A(skiplist_count(sl) == 0);

    FREE_NN();
    END_TEST();    
}

/* Check that deletion of a nonexistent key does not affect
 * the skiplist. */
TC(trivial_delete_not_present) {
    BEGIN_TEST();
    INIT();
    A(skiplist_count(sl) == 0);
    A(skiplist_member(sl, "foo") == 0);

    A(skiplist_set(sl, "foo", "bar", NULL) >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);

    char *old = skiplist_delete(sl, "baz");
    A(old == NULL);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);

    FREE_NN();
    END_TEST();    
}

static void inc_cb(void *key, void *value, void *udata)
{
    int *count = (int *)udata;
    assert(count);
    (*count)++;
}

/* Add several numeric values, delete some of them, check that
 * everything stays correct. */
TC(delete_many_individually) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    A(sl);
    const int limit = 100000;
    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        A(cp_buf);
        strncpy(cp_buf, buf, 20);
        A(skiplist_add(sl, (void *) i, cp_buf) >= 0);
    }
    A(skiplist_count(sl) == limit);

    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *old = skiplist_delete(sl, (void *) i);
        A(old);
        A(0 == strcmp(old, buf));
        test_free(old, 20);
        A(skiplist_count(sl) == limit - i - 1);
    }
    A(skiplist_count(sl) == 0);
    A(skiplist_empty(sl));
    skiplist_free(sl, NULL, NULL);
    END_TEST();
}

/* Add keys, deleet them all. */
TC(trivial_delete_all) {
    BEGIN_TEST();
    INIT();
    A(skiplist_add(sl, "foo", "bar") >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);

    A(skiplist_add(sl, "foo", "baz") >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 2);

    if (verbose > 1) {
        puts("\n**** before");
        skiplist_debug(sl, stdout, NULL, print_key_cb);
    }
    int count = 0;
    skiplist_delete_all(sl, "foo", &count, inc_cb);
    A(count == 2);
    A(skiplist_count(sl) == 0);

    if (verbose > 1) {
        puts("\n**** after");
        skiplist_debug(sl, stdout, NULL, NULL /*print_key_cb*/);
    }
    A(skiplist_member(sl, "foo") == 0);

    FREE_NN();
    END_TEST();    
}

/* Add a bunch of numeric keys (with duplicates), delete_all of
 * the duplicated key, which is at the head of the skiplist. */
TC(delete_all_first) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    const intptr_t limit = 100;
    intptr_t key = 0;

    for (intptr_t i=0; i<limit; i++) {
        A(skiplist_add(sl, (void *) i, (void *) 1) >= 0);
    }

    for (intptr_t i=0; i<30; i++) {
        A(skiplist_add(sl, (void *) key, (void *) 1) >= 0);
    }

    A(skiplist_member(sl, (void *) key) == 1);
    A(skiplist_count(sl) == limit + 30);

    if (verbose > 1) {
        puts("**** before");
        skiplist_debug(sl, stdout, NULL, print_intptr_cb);
    }
    int count = 0;
    skiplist_delete_all(sl, (void *) key, &count, inc_cb);
    A(count == 31);
    A(skiplist_count(sl) == limit - 1);

    if (verbose > 1) {
        puts("**** after");
        skiplist_debug(sl, stdout, NULL, print_intptr_cb);
    }

    A(skiplist_member(sl, (void *) key) == 0);

    FREE_NN();
    END_TEST();    
}

/* Add a bunch of numeric keys (with duplicates), delete_all of
 * the duplicated key, which is at the middle of the skiplist. */
TC(delete_all_middle) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    const intptr_t limit = 100000;
    intptr_t key = (limit / 2);
    for (intptr_t i=0; i<limit; i++) {
        A(skiplist_add(sl, (void *) i, (void *) 1) >= 0);
    }

    for (intptr_t i=0; i<30; i++) {
        A(skiplist_add(sl, (void *) key, (void *) 1) >= 0);
    }

    A(skiplist_member(sl, (void *) 0) == 1);
    A(skiplist_count(sl) == limit + 30);

    if (verbose > 1) {
        puts("**** before");
        skiplist_debug(sl, stdout, NULL, print_intptr_cb);
    }
    int count = 0;
    skiplist_delete_all(sl, (void *) key, &count, inc_cb);
    A(count == 31);
    A(skiplist_count(sl) == limit - 1);

    if (verbose > 1) {
        puts("**** after");
        skiplist_debug(sl, stdout, NULL, print_intptr_cb);
    }
    A(skiplist_member(sl, (void *) key) == 0);

    FREE_NN();
    END_TEST();    
}

/* Add a bunch of numeric keys (with duplicates), delete_all of
 * the duplicated key, which is at the end of the skiplist. */
TC(delete_all_end) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    const intptr_t limit = 100000;
    for (intptr_t i=0; i<limit; i++) {
        A(skiplist_add(sl, (void *) i, (void *) 1) >= 0);
    }

    for (intptr_t i=0; i<30; i++) {
        A(skiplist_add(sl, (void *) (limit / 2), (void *) 1) >= 0);
    }

    A(skiplist_member(sl, (void *) 0) == 1);
    A(skiplist_count(sl) == limit + 30);

    if (verbose > 1) {
        puts("**** before");
        skiplist_debug(sl, stdout, NULL, print_intptr_cb);
    }
    int count = 0;
    skiplist_delete_all(sl, (void *) (limit / 2), &count, inc_cb);
    A(count == 31);
    A(skiplist_count(sl) == limit - 1);

    if (verbose > 1) {
        puts("**** after");
        skiplist_debug(sl, stdout, NULL, print_intptr_cb);
    }
    A(skiplist_member(sl, (void *) (limit / 2)) == 0);

    FREE_NN();
    END_TEST();    
}

/* Get the first value. */
TC(first) {
    BEGIN_TEST();
    INIT();
    A(skiplist_add(sl, "foo", "bar") >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);

    A(skiplist_add(sl, "bar", "baz") >= 0);
    A(skiplist_member(sl, "bar") == 1);
    A(skiplist_count(sl) == 2);

    char *k = NULL;
    char *v = NULL;
    A(skiplist_first(sl, (void **) &k, (void **) &v) >= 0);
    A(k);
    A(v);
    A(0 == strcmp(k, "bar"));
    A(0 == strcmp(v, "baz"));

    FREE_NN();
    END_TEST();    
}

/* Get the last value. */
TC(last) {
    BEGIN_TEST();
    INIT();
    A(skiplist_add(sl, "foo", "bar") >= 0);
    A(skiplist_member(sl, "foo") == 1);
    A(skiplist_count(sl) == 1);

    A(skiplist_add(sl, "bar", "baz") >= 0);
    A(skiplist_member(sl, "bar") == 1);
    A(skiplist_count(sl) == 2);

    char *k = NULL;
    char *v = NULL;
    A(skiplist_last(sl, (void **) &k, (void **) &v) >= 0);
    A(k);
    A(v);
    A(0 == strcmp(k, "foo"));
    A(0 == strcmp(v, "bar"));

    FREE_NN();
    END_TEST();    
}

struct clear_cb_ud {
    int count;
    int ok;
};

static void clear_cb(void *key, void *value, void *udata)
{
    /* fprintf(stderr, "udata is %p\n", udata); */
    struct clear_cb_ud *ud = (struct clear_cb_ud *) udata;
    assert(ud);
    ud->count++;
    long ikey = (long) key;
    char *ivalue = (char *) value;
    char buf[20];
    snprintf(buf, 20, "%ld", ikey);
    ud->ok = ud->ok && (0 == strcmp(buf, ivalue));
    test_free(ivalue, 20);
}

/* Clear the skiplist. */
TC(clear) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    A(sl);
    const int limit = 1000000;
    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        A(cp_buf);
        strncpy(cp_buf, buf, 20);
        A(skiplist_add(sl, (void *) i, cp_buf) >= 0);
    }

    struct clear_cb_ud ud;
    ud.count = 0;
    ud.ok = 1;
    int ct = skiplist_clear(sl, &ud, clear_cb);
    A(ud.ok == 1);
    A(ud.count == limit);
    A(ct == limit);
    skiplist_free(sl, NULL, NULL);
    END_TEST();
}

/* Clear the skiplist with the free callback. */
TC(free_clear) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    A(sl);
    const int limit = 10000;
    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        A(cp_buf);
        strncpy(cp_buf, buf, 20);
        A(skiplist_add(sl, (void *) i, cp_buf) >= 0);
    }

    struct clear_cb_ud ud;
    ud.count = 0;
    ud.ok = 1;
    int ct = skiplist_free(sl, &ud, clear_cb);
    A(ud.ok == 1);
    A(ud.count == limit);
    A(ct == limit);
    END_TEST();
}

/* Add numeric pairs, then pepeatedly pop the first key/value pair
 * until empty, and check invariants. */
TC(pop_first) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    A(sl);
    const int limit = 1000;
    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        A(cp_buf);
        strncpy(cp_buf, buf, 20);
        A(skiplist_add(sl, (void *) i, cp_buf) >= 0);
    }

    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        long key = 0;
        char *value = NULL;
        A(skiplist_count(sl) == limit - i);
        A(skiplist_pop_first(sl, (void **) &key, (void **) &value) >= 0);
        A(skiplist_count(sl) == limit - i - 1);
        A(key == i);
        A(0 == strcmp(value, buf));
        test_free(value, 20);
    }

    A(skiplist_pop_first(sl, NULL, NULL) < 0);
    A(skiplist_empty(sl) == 1);
    A(skiplist_count(sl) == 0);
    skiplist_free(sl, NULL, NULL);
    END_TEST();
}

/* Add numeric pairs, then pepeatedly pop the last key/value pair
 * until empty, and check invariants. */
TC(pop_last) {
    BEGIN_TEST();
    SL *sl = skiplist_new(sl_longcmp);
    A(sl);
    const int limit = 1000;
    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        A(cp_buf);
        strncpy(cp_buf, buf, 20);
        A(skiplist_add(sl, (void *) i, cp_buf) >= 0);
    }

    for (long i=0; i<limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", limit - i - 1);
        long key = 0;
        char *value = NULL;
        A(skiplist_count(sl) == limit - i);
        A(skiplist_pop_last(sl, (void **) &key, (void **) &value) >= 0);
        A(skiplist_count(sl) == limit - i - 1);
        if (verbose > 1)
            fprintf(stderr, "i is %ld, got %ld and '%s', expected %ld\n",
                i, key, value, limit - i - 1);
        A(key == limit - i - 1);
        A(0 == strcmp(value, buf));
        test_free(value, 20);
    }

    A(skiplist_pop_last(sl, NULL, NULL) < 0);
    A(skiplist_empty(sl) == 1);
    A(skiplist_count(sl) == 0);
    skiplist_free(sl, NULL, NULL);

    END_TEST();
}


/*********/
/* Suite */
/*********/

static char *run_suite(char *test) {
#define TESTCASE(NAME)                                                  \
    if (!test || 0 == strcmp(test, #NAME)) {                            \
        if (verbose)                                                    \
            fprintf(stderr, "-- Running test '%s'\n", #NAME);           \
        mu_run_test(NAME);                                              \
    }
#include "test_list.h"
    TESTCASE(clear);
    R NULL;
}

int main(int argc, char **argv) {
    char *testname = NULL;
proc_arg:
    if (argc > 1) {
        if (0 == strcmp("-v", argv[1])) {
            verbose++;
            argv++;
            argc--;
            goto proc_arg;
        } else if (argc > 2 && 0 == (strcmp("-s", argv[1]))) {
            global_seed = atol(argv[2]);
            argv += 2;
            argc -= 2;
            goto proc_arg;
        } else {
            testname = argv[1];
        }
    }
    
    srandom(global_seed);

    char *res = run_suite(testname);
    fprintf(stderr, "%d test%s run\n",
        tests_run, tests_run == 1 ? "" : "s");
    if (res) fprintf(stderr, "Error: %s\n", res);
    return (res != NULL);
}
