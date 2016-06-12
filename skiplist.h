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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 0.8.0 */
#define SKIPLIST_VERSION_MAJOR 0
#define SKIPLIST_VERSION_MINOR 8
#define SKIPLIST_VERSION_PATCH 0

/* Opaque skiplist type. */
struct skiplist;

/* Comparison function, should return <0, 0, or >0 for less-than,
 * equal, and greater-than, respectively. */
typedef int skiplist_cmp_cb(void *key_a, void *key_b);

/* Callback when iterating over the contents of the skiplist.
 * If it returns nonzero, iteration will terminate immediately
 * and return that result.
 * UDATA is an extra void * for the callback's closure/enironment. */
typedef int skiplist_iter_cb(void *key, void *value, void *udata);

/* Callback when freeing keys and/or values contained by the skiplist. */
typedef void skiplist_free_cb(void *key, void *value, void *udata);

/* Callback to print KEY and/or VALUE inside skiplist_debug
 * to file F, if F is non-NULL. */
typedef void skiplist_fprintf_kv_cb(FILE *f, void *key,
    void *value,void *udata);

#ifdef SKIPLIST_CMP_CB
#define SKIPLIST_NEW_ARGS void
#else
#define SKIPLIST_NEW_ARGS skiplist_cmp_cb *cmp
#endif

/* Create a new skiplist, returns NULL on error.
 * May or may not take skiplist_cmp_cb, depending on whether
 * SKIPLIST_CMP_CB is defined. */
struct skiplist *skiplist_new(SKIPLIST_NEW_ARGS);

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
unsigned int SKIPLIST_GEN_HEIGHT(void);

/* Add a key/value pair to the skiplist. Equal keys will be kept
 * (bag functionality). KEY and/or VALUE are allowed to be NULL,
 * provided the cmp callback can handle it. Note that a NULL
 * value will not be recognized by skiplist_member.
 * If you add multiple values under the same key, they will not
 * necessarily be stored in any particular order.
 * Returns whether the value was successfully added. */
bool skiplist_add(struct skiplist *sl, void *key, void *value);

/* Set a key/value pair in the skiplist, replacing an existing
 * value if present. If OLD is non-NULL, then *old will be set
 * to the previous value, or NULL if it was not present.
 * Otherwise behaves the same as skiplist_add. */
bool skiplist_set(struct skiplist *sl, void *key, void *value, void **old);

/* Get the value associated with KEY. If the key is found and VALUE is
 * non-NULL, it will be written into *VALUE.
 * Returns whether the key was found. */
bool skiplist_get(struct skiplist *sl, void *key, void **value);

/* Does the skiplist contain KEY? */
bool skiplist_member(struct skiplist *sl, void *key);

/* Delete an association for KEY in the skiplist.
 * Returns the value, or NULL if not present.
 *
 * Note: If there are multiple values for the key, only one
 * (not necessarily the first) is deleted; see skiplist_delete_all. */
void *skiplist_delete(struct skiplist *sl, void *key);

/* Delete all associations for KEY in the skiplist. The callback is
 * called for each key/value pair, and cannot be NULL. */
void skiplist_delete_all(struct skiplist *sl, void *key,
    void *udata, skiplist_free_cb *cb);

/* Get the first or last pair from the skiplist.
 * If key or value are non-NULL, the pair is returned in them.
 * Passing in a NULL key is legal, it will be ignored.
 * Passing in a NULL value is legal, but useless.
 * Returns 0 on success, or <0 on error, e.g. an empty skiplist. */
bool skiplist_first(struct skiplist *sl, void **key, void **value);
bool skiplist_last(struct skiplist *sl, void **key, void **value);

/* Pop the key/value pair off the skiplist with the first/last key.
 * Same return behavior as skiplist_first/last, but also deletes the pair. */
bool skiplist_pop_first(struct skiplist *sl, void **key, void **value);
bool skiplist_pop_last(struct skiplist *sl, void **key, void **value);

/* How many pairs are in the skiplist?
 * Returns <0 on error. */
long skiplist_count(struct skiplist *sl);

/* Is the skiplist empty? */
bool skiplist_empty(struct skiplist *sl);

/* Iterate over the skiplist. See the typedef comment for
 * skiplist_iter_cb for more information. */
int skiplist_iter(struct skiplist *sl, void *udata, skiplist_iter_cb *cb);

/* Iterate over the skiplist, beginning at KEY.
 * Returns <0 if KEY is not present. */
int skiplist_iter_from(struct skiplist *sl, void *key,
    void *udata, skiplist_iter_cb *cb);

/* Clear the skiplist. Returns the number of pairs removed,
 * or <0 on error. */
long skiplist_clear(struct skiplist *sl, void *udata, skiplist_free_cb *cb);

/* Clear and free the skiplist. Returns the number of pairs removed,
 * or <0 on error. */
long skiplist_free(struct skiplist *sl, void *udata, skiplist_free_cb *cb);

/* Do an internal consistency check.
 * Prints debugging info to F (if non-NULL). */
void skiplist_debug(struct skiplist *sl, FILE *f,
    void *udata, skiplist_fprintf_kv_cb *cb);

#ifdef __cplusplus
}
#endif

#endif
