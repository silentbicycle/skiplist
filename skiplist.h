/*
 * Copyright (c) 2011-16 Scott Vokes <vokes.s@gmail.com>
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

#ifdef __cplusplus
extern "C" {
#endif

/* 0.9.0 */
#define SKIPLIST_VERSION_MAJOR 0
#define SKIPLIST_VERSION_MINOR 9
#define SKIPLIST_VERSION_PATCH 0

/* Opaque skiplist type. */
struct skiplist;

/* Memory allocation callback.
 *
 * When called as (NULL, 0, SIZE, udata), should be equivalent to
 * `malloc(SIZE)`. When called as (P, OLD_SIZE, 0, udata), should be
 * equivalent to `free(P)`, and return NULL. realloc-like behavior is
 * not currently used. */
typedef void *skiplist_alloc_cb(void *p,
    size_t osize, size_t nsize, void *udata);

/* Comparison callback, should return <0, 0, or >0 for less-than,
 * equal, and greater-than, respectively. */
typedef int skiplist_cmp_cb(void *key_a, void *key_b);

/* Create a new skiplist, returns NULL on error.
 * A comparison callback is required.
 * A memory management callback is optional - if NULL,
 * malloc & free will be used internally. */
struct skiplist *skiplist_new(skiplist_cmp_cb *cmp,
    skiplist_alloc_cb *alloc, void *alloc_udata);

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
uint8_t SKIPLIST_GEN_HEIGHT(void);

/* Add a key/value pair to the skiplist. Equal keys will be kept (bag
 * functionality). KEY and/or VALUE are allowed to be NULL, provided the
 * cmp callback can handle it. If you add multiple values under the same
 * key, they will not necessarily be stored in any particular order.
 *
 * Returns whether the value was successfully added. */
bool skiplist_add(struct skiplist *sl, void *key, void *value);

/* Set a key/value pair in the skiplist, replacing an existing
 * value if present. If OLD is non-NULL, then *old will be set
 * to the previous value, or NULL if it was not present.
 * Otherwise behaves the same as skiplist_add. */
bool skiplist_set(struct skiplist *sl,
    void *key, void *value, void **old);

/* Get the value associated with KEY. If the key is found and VALUE is
 * non-NULL, it will be written into *VALUE.
 * Returns whether the key was found. */
bool skiplist_get(struct skiplist *sl, void *key, void **value);

/* Does the skiplist contain KEY? */
bool skiplist_member(struct skiplist *sl, void *key);

/* Delete an association for KEY in the skiplist.
 * If found and VALUE is non-NULL, the old value will be
 * written to *VALUE.
 * Returns whether the key was found.
 *
 * Note: If there are multiple values for the key, only one
 * (not necessarily the first) is deleted; see skiplist_delete_all. */
bool skiplist_delete(struct skiplist *sl, void *key, void **value);

/* Callback when freeing keys and/or values contained by the skiplist. */
typedef void skiplist_free_cb(void *key, void *value, void *udata);

/* Delete all associations for KEY in the skiplist. The callback is
 * called for each key/value pair, and cannot be NULL. */
void skiplist_delete_all(struct skiplist *sl, void *key,
    skiplist_free_cb *cb, void *udata);

/* Get the first or last pair from the skiplist.
 * If key or value are non-NULL, the pair is returned in them.
 * Passing in a NULL key is legal, it will be ignored.
 * Passing in a NULL value is legal, but useless.
 * Returns whether a pair was found (the skiplist is non-empty). */
bool skiplist_first(struct skiplist *sl, void **key, void **value);
bool skiplist_last(struct skiplist *sl, void **key, void **value);

/* Pop the key/value pair off the skiplist with the first/last key. Same
 * return behavior as skiplist_first/last, but also deletes the pair. */
bool skiplist_pop_first(struct skiplist *sl, void **key, void **value);
bool skiplist_pop_last(struct skiplist *sl, void **key, void **value);

/* How many pairs are in the skiplist?
 * Returns 0 on error. */
size_t skiplist_count(struct skiplist *sl);

/* Is the skiplist empty? */
bool skiplist_empty(struct skiplist *sl);

/* Callback when iterating over the contents of the skiplist.
 * The return value determines whether to keep iterating.
 * UDATA is an extra void * for the callback's closure/enironment. */
enum skiplist_iter_res {
    SKIPLIST_ITER_HALT,
    SKIPLIST_ITER_CONTINUE,
};
typedef enum skiplist_iter_res
skiplist_iter_cb(void *key, void *value, void *udata);

/* Iterate over the skiplist. See the typedef comment for
 * skiplist_iter_cb for more information. */
void skiplist_iter(struct skiplist *sl,
    skiplist_iter_cb *cb, void *udata);

/* Iterate over the skiplist, beginning at KEY. */
void skiplist_iter_from(struct skiplist *sl, void *key,
    skiplist_iter_cb *cb, void *udata);

/* Clear the skiplist. Returns the number of pairs removed,
 * or 0 on error. */
size_t skiplist_clear(struct skiplist *sl,
    skiplist_free_cb *cb, void *udata);

/* Clear and free the skiplist. Returns the number of pairs removed,
 * or 0 on error. */
size_t skiplist_free(struct skiplist *sl,
    skiplist_free_cb *cb, void *udata);

#if SKIPLIST_DEBUG
#include <stdio.h>

/* Callback to print KEY and/or VALUE inside skiplist_debug
 * to file F, if F is non-NULL. */
typedef void skiplist_fprintf_kv_cb(FILE *f, void *key,
    void *value,void *udata);

/* Do an internal consistency check.
 * Prints debugging info to F (if non-NULL). */
void skiplist_debug(struct skiplist *sl, FILE *f,
    skiplist_fprintf_kv_cb *cb, void *udata);
#endif

#ifdef __cplusplus
}
#endif

#endif
