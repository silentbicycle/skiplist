#ifndef SKIPLIST_MACROS_INTERNAL_H
#define SKIPLIST_MACROS_INTERNAL_H

/* Abbreviations and other macros used internally. */

#define A assert
#define T skiplist

#define TODO()                                                          \
        { fprintf(stderr, "TODO: %s at %u\n", "foo", __LINE__);         \
                assert(0);                                              \
        }

#define LOG(lvl, ...)                                                   \
        do {                                                            \
                if (SKIPLIST_LOG_LEVEL >= lvl)                          \
                        fprintf(stderr, __VA_ARGS__);                   \
        } while(0)

#define LOG1(...) LOG(1, __VA_ARGS__)
#define LOG2(...) LOG(2, __VA_ARGS__)

#define CALL()                                                          \
        do { fprintf(stderr, " @ %s:%d\n", __FUNCTION__, __LINE__); } while (0) 

#define DO(count, block)                                \
        { for(int i=0; i<count; i++) { block; } }

/* Macros to cut down on ugly conditional-compilation-heavy code. */
#if SKIPLIST_USE_LOCK

#define SKIPLIST_LOCK_INIT(lock)                                \
        if (pthread_mutex_init(lock, NULL) != 0) goto lockfail;
#define SKIPLIST_LOCK_FREE(lock)                                \
        if (pthread_mutex_destroy(lock) != 0) goto lockfail;
#define SKIPLIST_LOCK(lock)                                     \
        if (pthread_mutex_lock(lock) != 0) goto lockfail;
#define SKIPLIST_UNLOCK(lock)                                   \
        if (pthread_mutex_unlock(lock) != 0) goto lockfail;

#else
/* No-Ops */
#define SKIPLIST_LOCK_INIT(lock)
#define SKIPLIST_LOCK_FREE(lock)
#define SKIPLIST_LOCK(lock)
#define SKIPLIST_UNLOCK(lock)

#endif

#ifdef SKIPLIST_CMP_CB
#define SKIPLIST_CMP(a, b) SKIPLIST_CMP_CB(a, b)
#define SKIPLIST_CMP_INIT() /* No-op */
#define SKIPLIST_NEW_ARGS /* none */
#else
#define SKIPLIST_CMP(a, b) sl->cmp(a, b)
#define SKIPLIST_CMP_INIT() { assert(cmp); sl->cmp = cmp; }
#define SKIPLIST_NEW_ARGS skiplist_cmp_cb *cmp
#endif

#endif
