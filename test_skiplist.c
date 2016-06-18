#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <assert.h>

#include "test_config.h"
#include "skiplist.h"
#include "greatest.h"
#include "test_alloc.h"

static long global_seed = 23;

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

static void setup(void *udata) {
    (void)udata;
    test_reset();
}

static void teardown(void *udata) {
    (void)udata;
    assert(test_check_for_leaks());
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

TEST init(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Adding one word should succeed. */
TEST add_one_word(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_add(sl, "foo", "bar"));
    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* This is a quick and dirty test that (for severaral random seeds)
 * the expectation for the distribution of levels generally holds.
 * This could be replaced by a more formal statistical analysis, and
 * should also be modified if the user supplies their own function. */
TEST level_statistical_distribution(void) {
    int counts[SKIPLIST_MAX_HEIGHT];
    for(int i = 0; i < SKIPLIST_MAX_HEIGHT; i++) counts[i] = 0;
    for (long lseed = 1; lseed < 1000; lseed++) {
        int counted = 0, in_bounds = 0;
        for (long trials = 0; trials < 10000; trials++)
            counts[SKIPLIST_GEN_HEIGHT()]++;

        for(int i = 1; i < SKIPLIST_MAX_HEIGHT; i++) {
            if (counts[i - 1] != 0 && counts[i] != 0) {
                counted++;
                double ratio = counts[i] / (1.0 * counts[i - 1]);
                if (ratio >= 0.3 && ratio <= 0.75)
                    in_bounds++;
                else
                    in_bounds--;
            }
        }
        if (0) {
            printf("counted is %d, in_bounds is %d\n", counted, in_bounds);
        }
        ASSERT(in_bounds > counted/4);
    }

    PASS();
    srandom(global_seed);
}

static void print_key_cb(FILE *f, void *k, void *v, void *ud) {
    (void)v;
    (void)ud;
    /* fprintf(f, "\n\n k is %p\n", k); */
    if (k == NULL) k = "(NULL)";
    fprintf(f, "'%s'", (char *) k);
}

static void print_intptr_cb(FILE *f, void *k, void *v, void *ud) {
    (void)v;
    (void)ud;
    fprintf(f, "'%ld'", (intptr_t) k);
}

/* Add a bunch of words to a skiplist and check that the count
 * remains correct. */
TEST fill_with_words(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    char **w = NULL;
    size_t ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        ASSERT(skiplist_add(sl, *w, *w));
        ct++;
        ASSERT(skiplist_count(sl) == ct);
    }

    if (greatest_get_verbosity() > 1) {
        skiplist_debug(sl, stderr, print_key_cb, NULL);
    }

    skiplist_free(sl, NULL, NULL);
    PASS();
}

typedef struct cb_udata {
    size_t *count;
    const char *prev;
    int ok;
} cb_udata;

static enum skiplist_iter_res
sl_count_and_check_sorted_cb(void *k, void *v, void *udata) {
    (void)v;
    cb_udata *ud = (cb_udata *) udata;
    char *word = (char *) k;
    if (0) fprintf(stderr, "ct=%zd, prev='%s', cur='%s'\n",
        *ud->count, ud->prev, word);

    /* If set, check that prev is <= current word. */
    if (ud->prev && strcmp(ud->prev, word) > 0) {
        ud->ok = 0;
        return SKIPLIST_ITER_HALT;
    }

    ud->prev = (char *) k;
    (*ud->count)++;
    return SKIPLIST_ITER_CONTINUE;
}

/* Add words and check that they are automatically sorted during insertion. */
TEST fill_with_words_check_sorted(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    char **w = NULL;
    size_t ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (greatest_get_verbosity() > 1) printf("word is '%s'\n", *w);
        ASSERT(skiplist_add(sl, *w, *w));
        ct++;
        ASSERT(skiplist_count(sl) == ct);
    }

    cb_udata udata;
    size_t count = 0;
    udata.count = &count;
    udata.prev = NULL;
    udata.ok = 1;

    /* iter and verify they're ordered alphabetically, ascending */
    skiplist_iter(sl, sl_count_and_check_sorted_cb, &udata);
    ASSERT_EQ_FMT(1, udata.ok, "%d");
    ASSERT(udata.ok);
    ASSERT(count == ct);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

static enum skiplist_iter_res
sl_check_and_terminate_early_cb(void *k, void *v, void *udata) {
    (void)v;
    cb_udata *ud = (cb_udata *) udata;
    char *word = (char *) k;
    if (0) fprintf(stderr, "ct=%zd, prev='%s', cur='%s'\n",
        *ud->count, ud->prev, word);

    /* If set, check that prev is <= current word. */
    if (ud->prev && strcmp(ud->prev, word) > 0) {
        ud->ok = 0;
        return SKIPLIST_ITER_HALT;
    }

    ud->prev = (char *) k;
    (*ud->count)++;
    if (0 == strcmp("onion", word)) {
        return SKIPLIST_ITER_HALT;
    } else {
        return SKIPLIST_ITER_CONTINUE;
    }
}

/* Same as previous test, but scan from the beginning to "onion". */
TEST fill_with_words_terminate_early(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    char **w = NULL;
    size_t ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (greatest_get_verbosity() > 1) printf("word is '%s'\n", *w);
        ASSERT(skiplist_add(sl, *w, *w));
        ct++;
        ASSERT(skiplist_count(sl) == ct);
    }

    cb_udata udata;
    size_t count = 0;
    udata.count = &count;
    udata.prev = NULL;
    udata.ok = 1;

    /* iter and verify they're ordered alphabetically, quitting
     * at the word "onion". */
    skiplist_iter(sl, sl_check_and_terminate_early_cb, &udata);
    ASSERT(udata.ok);
    ASSERT(count == 105);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add a bunch of words, and count the number of words starting from the
 * first occurrence of "onion". Tests that it counts from the _first_;
 * "onion" appears multiple times as a key. */
TEST fill_with_words_count_from_onion(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    char **w = NULL;
    size_t ct = 0;
    for (w = (char **)wordlist; *w; w++) {
        if (greatest_get_verbosity() > 1) printf("word is '%s'\n", *w);
        ASSERT(skiplist_add(sl, *w, *w));
        ct++;
        ASSERT(skiplist_count(sl) == ct);
    }

    cb_udata udata;
    size_t count = 0;
    udata.count = &count;
    udata.prev = NULL;
    udata.ok = 1;

    /* iter and verify they're ordered alphabetically, starting
     * at the word "onion". */
    skiplist_iter_from(sl, "onion",
        sl_count_and_check_sorted_cb, &udata);
    ASSERT(udata.ok);
    if (greatest_get_verbosity() > 1) {
        printf("count is %zd\n", count);
        skiplist_debug(sl, stdout, print_key_cb, NULL);
    }
    ASSERT(count == 62);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Test membership for newly added words. */
TEST add_and_member(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_empty(sl) == 1);
    ASSERT(skiplist_count(sl) == 0);
    ASSERT(skiplist_member(sl, "foo") == 0);

    ASSERT(skiplist_add(sl, "foo", "foo"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);
    ASSERT(skiplist_empty(sl) == 0);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add word, replace word, get old value. */
TEST add_and_set(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_count(sl) == 0);
    ASSERT(skiplist_member(sl, "foo") == 0);

    /* Add "foo" -> "foo" */
    ASSERT(skiplist_add(sl, "foo", "foo"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);
    char *v = NULL;
    ASSERT(skiplist_get(sl, "foo", (void **)&v));
    ASSERT(v);
    ASSERT(0 == strcmp(v, "foo"));

    /* Set "foo" -> "bar", check old value */
    char *old = NULL;
    ASSERT(skiplist_set(sl, "foo", "bar", (void **) &old));
    ASSERT(old);
    ASSERT(0 == strcmp(old, "foo"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);
    v = NULL;
    ASSERT(skiplist_get(sl, "foo", (void **)&v));
    ASSERT(v);
    ASSERT(0 == strcmp(v, "bar"));

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Set a word in an empty skiplist, check invariants. */
TEST set(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_count(sl) == 0);
    ASSERT(skiplist_member(sl, "foo") == 0);

    /* Set "foo" -> "bar", check old value is NULL & count changes. */
    char *old = NULL;
    ASSERT(skiplist_set(sl, "foo", "bar", (void **) &old));
    ASSERT(old == NULL);
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);
    char *v = NULL;
    ASSERT(skiplist_get(sl, "foo", (void **)&v));
    ASSERT(v);
    ASSERT(0 == strcmp(v, "bar"));

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Set and delete in an empty skiplist, check invariants. */
TEST trivial_delete(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_count(sl) == 0);
    ASSERT(skiplist_member(sl, "foo") == 0);

    ASSERT(skiplist_set(sl, "foo", "bar", NULL));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);

    char *old = NULL;
    ASSERT(skiplist_delete(sl, "foo", (void **)&old));
    ASSERT(old);
    ASSERT(0 == strcmp(old, "bar"));
    ASSERT(skiplist_member(sl, "foo") == 0);
    ASSERT(skiplist_count(sl) == 0);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Check that deletion of a nonexistent key does not affect
 * the skiplist. */
TEST trivial_delete_not_present(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_count(sl) == 0);
    ASSERT(skiplist_member(sl, "foo") == 0);

    ASSERT(skiplist_set(sl, "foo", "bar", NULL));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);

    char *old = NULL;
    ASSERT(!skiplist_delete(sl, "baz", (void **)&old));
    ASSERT(old == NULL);
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

static void inc_cb(void *key, void *value, void *udata)
{
    (void)key;
    (void)value;
    int *count = (int *)udata;
    assert(count);
    (*count)++;
}

/* Add several numeric values, delete some of them, check that
 * everything stays correct. */
TEST delete_many_individually(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    ASSERT(sl);
    const size_t limit = 100000;
    for (size_t i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        ASSERT(cp_buf);
        strncpy(cp_buf, buf, 20);
        ASSERT(skiplist_add(sl, (void *) i, cp_buf));
    }
    ASSERT(skiplist_count(sl) == limit);

    for (size_t i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *old = NULL;
        ASSERT(skiplist_delete(sl, (void *) i, (void **)&old));
        ASSERT(old);
        ASSERT(0 == strcmp(old, buf));
        test_free(old, 20);
        ASSERT(skiplist_count(sl) == limit - i - 1);
    }
    ASSERT(skiplist_count(sl) == 0);
    ASSERT(skiplist_empty(sl));
    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add keys, deleet them all. */
TEST trivial_delete_all(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_add(sl, "foo", "bar"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);

    ASSERT(skiplist_add(sl, "foo", "baz"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 2);

    if (greatest_get_verbosity() > 1) {
        puts("\n**** before");
        skiplist_debug(sl, stdout, print_key_cb, NULL);
    }
    size_t count = 0;
    skiplist_delete_all(sl, "foo", inc_cb, &count);
    ASSERT(count == 2);
    ASSERT(skiplist_count(sl) == 0);

    if (greatest_get_verbosity() > 1) {
        puts("\n**** after");
        skiplist_debug(sl, stdout, NULL, NULL /*print_key_cb*/);
    }
    ASSERT(skiplist_member(sl, "foo") == 0);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add a bunch of numeric keys (with duplicates), delete_all of
 * the duplicated key, which is at the head of the skiplist. */
TEST delete_all_first(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    const intptr_t limit = 100;
    intptr_t key = 0;

    for (intptr_t i = 0; i < limit; i++) {
        ASSERT(skiplist_add(sl, (void *) i, (void *) 1));
    }

    for (intptr_t i = 0; i < 30; i++) {
        ASSERT(skiplist_add(sl, (void *) key, (void *) 1));
    }

    ASSERT(skiplist_member(sl, (void *) key) == 1);
    ASSERT(skiplist_count(sl) == limit + 30);

    if (greatest_get_verbosity() > 1) {
        puts("**** before");
        skiplist_debug(sl, stdout, print_intptr_cb, NULL);
    }
    int count = 0;
    skiplist_delete_all(sl, (void *) key, inc_cb, &count);
    ASSERT(count == 31);
    ASSERT(skiplist_count(sl) == limit - 1);

    if (greatest_get_verbosity() > 1) {
        puts("**** after");
        skiplist_debug(sl, stdout, print_intptr_cb, NULL);
    }

    ASSERT(skiplist_member(sl, (void *) key) == 0);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add a bunch of numeric keys (with duplicates), delete_all of
 * the duplicated key, which is at the middle of the skiplist. */
TEST delete_all_middle(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    const intptr_t limit = 100000;
    intptr_t key = (limit / 2);
    for (intptr_t i = 0; i < limit; i++) {
        ASSERT(skiplist_add(sl, (void *) i, (void *) 1));
    }

    for (intptr_t i = 0; i < 30; i++) {
        ASSERT(skiplist_add(sl, (void *) key, (void *) 1));
    }

    ASSERT(skiplist_member(sl, (void *) 0) == 1);
    ASSERT(skiplist_count(sl) == limit + 30);

    if (greatest_get_verbosity() > 1) {
        puts("**** before");
        skiplist_debug(sl, stdout, print_intptr_cb, NULL);
    }
    int count = 0;
    skiplist_delete_all(sl, (void *) key, inc_cb, &count);
    ASSERT(count == 31);
    ASSERT(skiplist_count(sl) == limit - 1);

    if (greatest_get_verbosity() > 1) {
        puts("**** after");
        skiplist_debug(sl, stdout, print_intptr_cb, NULL);
    }
    ASSERT(skiplist_member(sl, (void *) key) == 0);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add a bunch of numeric keys (with duplicates), delete_all of
 * the duplicated key, which is at the end of the skiplist. */
TEST delete_all_end(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    const intptr_t limit = 100000;
    for (intptr_t i = 0; i < limit; i++) {
        ASSERT(skiplist_add(sl, (void *) i, (void *) 1));
    }

    for (intptr_t i = 0; i < 30; i++) {
        ASSERT(skiplist_add(sl, (void *) (limit / 2), (void *) 1));
    }

    ASSERT(skiplist_member(sl, (void *) 0) == 1);
    ASSERT(skiplist_count(sl) == limit + 30);

    if (greatest_get_verbosity() > 1) {
        puts("**** before");
        skiplist_debug(sl, stdout, print_intptr_cb, NULL);
    }
    int count = 0;
    skiplist_delete_all(sl, (void *) (limit / 2), inc_cb, &count);
    ASSERT(count == 31);
    ASSERT(skiplist_count(sl) == limit - 1);

    if (greatest_get_verbosity() > 1) {
        puts("**** after");
        skiplist_debug(sl, stdout, print_intptr_cb, NULL);
    }
    ASSERT(skiplist_member(sl, (void *) (limit / 2)) == 0);

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Get the first value. */
TEST first(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_add(sl, "foo", "bar"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);

    ASSERT(skiplist_add(sl, "bar", "baz"));
    ASSERT(skiplist_member(sl, "bar") == 1);
    ASSERT(skiplist_count(sl) == 2);

    char *k = NULL;
    char *v = NULL;
    ASSERT(skiplist_first(sl, (void **) &k, (void **) &v));
    ASSERT(k);
    ASSERT(v);
    ASSERT(0 == strcmp(k, "bar"));
    ASSERT(0 == strcmp(v, "baz"));

    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Get the last value. */
TEST last(void) {
    struct skiplist *sl = skiplist_new(sl_strcmp, test_alloc, NULL);
    ASSERT(sl);
    ASSERT(skiplist_add(sl, "foo", "bar"));
    ASSERT(skiplist_member(sl, "foo") == 1);
    ASSERT(skiplist_count(sl) == 1);

    ASSERT(skiplist_add(sl, "bar", "baz"));
    ASSERT(skiplist_member(sl, "bar") == 1);
    ASSERT(skiplist_count(sl) == 2);

    char *k = NULL;
    char *v = NULL;
    ASSERT(skiplist_last(sl, (void **) &k, (void **) &v));
    ASSERT(k);
    ASSERT(v);
    ASSERT(0 == strcmp(k, "foo"));
    ASSERT(0 == strcmp(v, "bar"));

    skiplist_free(sl, NULL, NULL);
    PASS();
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
TEST clear(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    ASSERT(sl);
    const int limit = 1000000;
    for (long i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        ASSERT(cp_buf);
        strncpy(cp_buf, buf, 20);
        ASSERT(skiplist_add(sl, (void *) i, cp_buf));
    }

    struct clear_cb_ud ud;
    ud.count = 0;
    ud.ok = 1;
    int ct = skiplist_clear(sl, clear_cb, &ud);
    ASSERT(ud.ok == 1);
    ASSERT(ud.count == limit);
    ASSERT(ct == limit);
    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Clear the skiplist with the free callback. */
TEST free_clear(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    ASSERT(sl);
    const int limit = 10000;
    for (long i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        ASSERT(cp_buf);
        strncpy(cp_buf, buf, 20);
        ASSERT(skiplist_add(sl, (void *) i, cp_buf));
    }

    struct clear_cb_ud ud;
    ud.count = 0;
    ud.ok = 1;
    int ct = skiplist_free(sl, clear_cb, &ud);
    ASSERT(ud.ok == 1);
    ASSERT(ud.count == limit);
    ASSERT(ct == limit);
    PASS();
}

/* Add numeric pairs, then pepeatedly pop the first key/value pair
 * until empty, and check invariants. */
TEST pop_first(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    ASSERT(sl);
    const size_t limit = 1000;
    for (size_t i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        ASSERT(cp_buf);
        strncpy(cp_buf, buf, 20);
        ASSERT(skiplist_add(sl, (void *) i, cp_buf));
    }

    for (size_t i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        size_t key = 0;
        char *value = NULL;
        ASSERT(skiplist_count(sl) == limit - i);
        ASSERT(skiplist_pop_first(sl, (void **) &key, (void **) &value));
        ASSERT(skiplist_count(sl) == limit - i - 1);
        ASSERT(key == i);
        ASSERT(0 == strcmp(value, buf));
        test_free(value, 20);
    }

    ASSERT(!skiplist_pop_first(sl, NULL, NULL));
    ASSERT(skiplist_empty(sl) == 1);
    ASSERT(skiplist_count(sl) == 0);
    skiplist_free(sl, NULL, NULL);
    PASS();
}

/* Add numeric pairs, then pepeatedly pop the last key/value pair
 * until empty, and check invariants. */
TEST pop_last(void) {
    struct skiplist *sl = skiplist_new(sl_longcmp, test_alloc, NULL);
    ASSERT(sl);
    const size_t limit = 1000;
    for (size_t i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", i);
        char *cp_buf = test_malloc(20);
        ASSERT(cp_buf);
        strncpy(cp_buf, buf, 20);
        ASSERT(skiplist_add(sl, (void *) i, cp_buf));
    }

    for (size_t i = 0; i < limit; i++) {
        char buf[20];
        snprintf(buf, 20, "%ld", limit - i - 1);
        size_t key = 0;
        char *value = NULL;
        ASSERT(skiplist_count(sl) == limit - i);
        ASSERT(skiplist_pop_last(sl, (void **) &key, (void **) &value));
        ASSERT(skiplist_count(sl) == limit - i - 1);
        if (greatest_get_verbosity() > 1)
            fprintf(stderr, "i is %ld, got %ld and '%s', expected %ld\n",
                i, key, value, limit - i - 1);
        ASSERT(key == limit - i - 1);
        ASSERT(0 == strcmp(value, buf));
        test_free(value, 20);
    }

    ASSERT(!skiplist_pop_last(sl, NULL, NULL));
    ASSERT(skiplist_empty(sl) == 1);
    ASSERT(skiplist_count(sl) == 0);
    skiplist_free(sl, NULL, NULL);

    PASS();
}


/*********/
/* Suite */
/*********/

GREATEST_MAIN_DEFS();

SUITE(suite) {
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(init);
    RUN_TEST(add_one_word);
    RUN_TEST(level_statistical_distribution);
    RUN_TEST(fill_with_words);
    RUN_TEST(fill_with_words_check_sorted);
    RUN_TEST(fill_with_words_terminate_early);
    RUN_TEST(fill_with_words_count_from_onion);
    RUN_TEST(add_and_member);
    RUN_TEST(add_and_set);
    RUN_TEST(set);
    RUN_TEST(trivial_delete);
    RUN_TEST(trivial_delete_not_present);
    RUN_TEST(delete_many_individually);
    RUN_TEST(trivial_delete_all);
    RUN_TEST(delete_all_first);
    RUN_TEST(delete_all_middle);
    RUN_TEST(delete_all_end);
    RUN_TEST(first);
    RUN_TEST(last);
    RUN_TEST(clear);
    RUN_TEST(free_clear);
    RUN_TEST(pop_first);
    RUN_TEST(pop_last);
}

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* init & parse command-line args */
    srandom(global_seed);
    RUN_SUITE(suite);
    GREATEST_MAIN_END();        /* display results */
}
