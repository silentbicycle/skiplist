#ifndef TEST_ALLOC_H
#define TEST_ALLOC_H

extern long allocated;

void *test_malloc(size_t sz);
void *test_realloc(void *p, size_t sz);
void test_free(void *p, size_t sz);

void test_reset();
int test_check_for_leaks();

#endif
