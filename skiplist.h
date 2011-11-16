/* 
 * Copyright (c) 2011 Scott Vokes <vokes.s@gmail.com>
 *  
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * For more information on skiplists, see William Pugh's paper,
 * "Skip Lists: A Probabilistic Alternative to Balanced Trees".
 */

#ifndef SKIPLIST_H
#define SKIPLIST_H

/* Opaque skiplist type. */
struct skiplist;

#define T struct skiplist

/* Comparison function, should return <0, 0, or >0 for less-than,
 * equal, and greater-than, respectively. */
typedef int (skiplist_cmp_cb)(void *keyA, void *keyB);

/* Callback when iterating over the contents of the skiplist.
 * If it returns nonzero, iteration will terminate immediately
 * and return that result.
 * UDATA is an extra void * for the callback's closure/enironment. */
typedef int (skiplist_iter_cb)(void *key, void *value, void *udata);

/* Callback when freeing keys and/or values contained by the skiplist. */
typedef void (skiplist_free_cb)(void *key, void *value, void *udata);

/* Callback to print KEY and/or VALUE inside skiplist_debug
 * to file F, if F is non-NULL. */
typedef void (skiplist_fprintf_kv_cb)(FILE *f, void *key,
                                      void *value,void *udata);

#ifdef SKIPLIST_CMP_CB
#define SKIPLIST_NEW_ARGS /* none */
#else
#define SKIPLIST_NEW_ARGS skiplist_cmp_cb *cmp
#endif

/* Create a new skiplist, returns NULL on error.
 * May or may not take skiplist_cmp_cb, depending on whether
 * SKIPLIST_CMP_CB is defined. */
T *skiplist_new(SKIPLIST_NEW_ARGS);

/* Set the random seed used when randomly constructing skiplists. */
void skiplist_set_seed(unsigned seed);

/* Randomly generate the height for the next level.
 * Should return between 1 and SKIPLIST_MAX_HEIGHT, inclusive.
 * Returning an illegal height is a checked error.
 *
 * For the skiplist's invariants to hold, the propability of
 * >= level N should be a constant proportion of the probability
 * of the level beneath it, e.g. prob(>=1) -> 1, prob(>=2) -> 1/2,
 * prob(>=3) -> 1/4, prob(>=4) -> 1/8, etc.
 *
 * SKIPLIST_GEN_HEIGHT can be replaced at compile-time, but
 * defaults to a probability of 0.5 per each additional level.
 */
unsigned int SKIPLIST_GEN_HEIGHT();

/* Add a key/value pair to the skiplist. Equal keys will be kept
 * (bag functionality). KEY and/or VALUE are allowed to be NULL,
 * provided the cmp callback can handle it. Note that a NULL
 * value will not be recognized by skiplist_member.
 * If you add multiple values under the same key, they will not
 * necessarily be stored in any particular order.
 * Returns <0 on error, 0 on success. */
int skiplist_add(T *sl, void *key, void *value);

/* Set a key/value pair in the skiplist, replacing an existing
 * value if present. If OLD is non-NULL, then *old will be set
 * to the previous value, or NULL if it was not present.
 * Otherwise behaves the same as skiplist_add. */
int skiplist_set(T *sl, void *key, void *value, void **old);

/* Get the value associated with KEY, or NULL if not present. */
void *skiplist_get(T *sl, void *key);

/* Does the skiplist contain KEY?
 * Note - will fail if the value is NULL. */
int skiplist_member(T *sl, void *key);

/* Delete an association for KEY in the skiplist.
 * Returns the value, or NULL if not present.
 *
 * Note: If there are multiple values for the key, only one
 * (not necessarily the first) is deleted; see skiplist_delete_all. */
void *skiplist_delete(T *sl, void *key);

/* Delete all associations for KEY in the skiplist. The callback is
 * called for each key/value pair, and cannot be NULL. */
void skiplist_delete_all(T *sl, void *key,
    void *udata, skiplist_free_cb *cb);

/* Get the first or last pair from the skiplist.
 * If key or value are non-NULL, the pair is returned in them.
 * Passing in a NULL key is legal, it will be ignored.
 * Passing in a NULL value is legal, but useless.
 * Returns 0 on success, or <0 on error, e.g. an empty skiplist. */
int skiplist_first(T *sl, void **key, void **value);
int skiplist_last(T *sl, void **key, void **value);

/* Pop the key/value pair off the skiplist with the first/last key.
 * Same return behavior as skiplist_first/last, but also deletes the pair. */
int skiplist_pop_first(T *sl, void **key, void **value);
int skiplist_pop_last(T *sl, void **key, void **value);

/* How many pairs are in the skiplist?
 * Returns <0 on error. */
long skiplist_count(T *sl);

/* Is the skiplist empty? */
int skiplist_empty(T *sl);

/* Iterate over the skiplist. See the typedef comment for
 * skiplist_iter_cb for more information. */
int skiplist_iter(T *sl, void *udata, skiplist_iter_cb *cb);

/* Iterate over the skiplist, beginning at KEY.
 * Returns <0 if KEY is not present. */
int skiplist_iter_from(T *sl, void *key,
    void *udata, skiplist_iter_cb *cb);

/* Clear the skiplist. Returns the number of pairs removed,
 * or <0 on error. */
long skiplist_clear(T *sl, void *udata, skiplist_free_cb *cb);

/* Clear and free the skiplist. Returns the number of pairs removed,
 * or <0 on error. */
long skiplist_free(T *sl, void *udata, skiplist_free_cb *cb);

/* Do an internal consistency check.
 * Prints debugging info to F (if non-NULL). */
void skiplist_debug(T *sl, FILE *f,
    void *udata, skiplist_fprintf_kv_cb *cb);

#undef T
#endif
