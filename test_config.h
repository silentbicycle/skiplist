#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include "test_alloc.h"

#define SKIPLIST_MAX_HEIGHT 25

#define SKIPLIST_MALLOC(sz) test_malloc(sz)
#define SKIPLIST_REALLOC(p, sz) test_realloc(p, sz)
#define SKIPLIST_FREE(p, sz) test_free(p, sz)

#endif
