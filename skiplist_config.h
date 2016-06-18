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

/* Level for debugging logs.
 * 0 = no logging, 1 = debug, 2 = the firehose. */
#ifndef SKIPLIST_LOG_LEVEL
#define SKIPLIST_LOG_LEVEL 0
#endif

/* Include skiplist debugging function? */
#ifndef SKIPLIST_DEBUG
#define SKIPLIST_DEBUG 0
#endif

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
