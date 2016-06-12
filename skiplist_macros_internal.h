#ifndef SKIPLIST_MACROS_INTERNAL_H
#define SKIPLIST_MACROS_INTERNAL_H

/* Abbreviations and other macros used internally. */

#if SKIPLIST_LOG_LEVEL > 0
#include <stdio.h>
#endif

#define LOG(lvl, ...)                                                   \
        do {                                                            \
                if (SKIPLIST_LOG_LEVEL >= lvl)                          \
                        fprintf(stderr, __VA_ARGS__);                   \
        } while(0)

#define LOG1(...) LOG(1, __VA_ARGS__)
#define LOG2(...) LOG(2, __VA_ARGS__)

#define DO(count, block)                                \
        { for(int i=0; i<count; i++) { block; } }

#endif
