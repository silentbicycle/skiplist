#ifndef TEST_ALLOC_H
#define TEST_ALLOC_H

extern long allocated;

void *test_alloc(void *p, size_t osize, size_t nsize, void *udata);

void *test_malloc(size_t sz);
void test_free(void *p, size_t sz);

void test_reset(void);
int test_check_for_leaks(void);

#endif
