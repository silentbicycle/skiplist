#ifndef SKIPLIST_CONFIG_H
#define SKIPLIST_CONFIG_H

/* Special compile-time configuration for the skiplist library. */

/* If defined, this file will alse be included. For example,
 * compile with -DSKIPLIST_LOCAL_INCLUDE=\"extra_config.h\" . */
#ifdef SKIPLIST_LOCAL_INCLUDE
#include SKIPLIST_LOCAL_INCLUDE
#endif

/* Maximum node height. */
#ifndef SKIPLIST_MAX_HEIGHT
#define SKIPLIST_MAX_HEIGHT 28
#endif

/* Should the skiplist nodes include a pthread_mutex_t?
 * Skiplists have reasonably good performance in a multithreaded
 * system, since most writes tend to be very localized.
 * 
 * NOTE: Thread-safe behavior is not yet implemented. */
#ifndef SKIPLIST_USE_LOCK
#define SKIPLIST_USE_LOCK 0
#endif

/* Type for skiplist count, can be made smaller to save space
 * (depending on platforms' alignment restrictions).
 * Must be signed; negative counts are used to indicate errors. */
typedef long skiplist_count_t;

/* Level for debugging logs.
 * 0 = no logging, 1 = debug, 2 = the firehose. */
#ifndef SKIPLIST_LOG_LEVEL
#define SKIPLIST_LOG_LEVEL 0
#endif

/* Allow swapping out the memory allocation functions. */
#ifndef SKIPLIST_MALLOC
#define SKIPLIST_MALLOC(sz) malloc(sz)
#define SKIPLIST_REALLOC(p, sz) realloc(p, sz)
/* Note - has correct size when freeing, though free does not use it. */
#define SKIPLIST_FREE(p, sz) free(p)
#endif

/* Allow the comparison callback to be hardcoded at compile time. */
/* #define SKIPLIST_CMP_CB skiplist_cmp_function */

/* Allow the free callback to be hardcoded at compile time. NYI. */
/* #define SKIPLIST_FREE_CB skiplist_free_function */

/* Define a custom random-height-calculation function.
 * 
 * To keep expected skiplist behavior, the probability of a
 * new node having a level >= N should be:
 *     probability(1)   :- 1.
 *     probability(N+1) :- P * probability(N).
 * By default, P is 0.5.
 */
/* #define SKIPLIST_GEN_HEIGHT skiplist_gen_height_function */

#endif
