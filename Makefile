TARGETS=	libskiplist.a test_skiplist bench

#CC=	gcc
#CC=	clang

WARN=	-Wall -Wshadow -Wuninitialized

CFLAGS+=	${WARN} -std=c99
CFLAGS+=	-O2
#CFLAGS+=	-DNDEBUG
CFLAGS+=	-g #-pg

BENCH_FLAGS=	-DNDEBUG

# ----

SKIPLIST_HEADERS=	skiplist.h skiplist_config.h \
			skiplist_macros_internal.h

# Build the static library with ar or libtool?
MAKE_LIB=	ar rcs $@
#MAKE_LIB=	libtool -static -o $@

all: ${TARGETS}

test: test_skiplist
	@./test_skiplist

libskiplist.a: skiplist.o
	${MAKE_LIB} skiplist.o

test_skiplist: skiplist-test.o test_alloc.o test_list.h test_words.h
	${CC} -o test_skiplist ${CFLAGS} ${LDFLAGS} \
	skiplist-test.o test_alloc.o test_skiplist.c

bench: bench.c libskiplist.a
	${CC} -o $@ bench.c ${CFLAGS} ${BENCH_FLAGS} -L. -lskiplist ${LDFLAGS}

skiplist.o: Makefile skiplist.c ${SKIPLIST_HEADERS}
	${CC} -c -o $@ skiplist.c ${CFLAGS} 

skiplist-test.o: Makefile skiplist.c ${SKIPLIST_HEADERS}
	${CC} -c -o $@ -DSKIPLIST_LOCAL_INCLUDE=\"test_config.h\" \
	skiplist.c ${CFLAGS}

test_alloc.o: test_alloc.c

test_list.h: test_skiplist.c
	./gen_tests test_skiplist.c > $@

TAGS: skiplist.c ${SKIPLIST_HEADERS}
	etags *.[ch]

clean:
	rm -rf libskiplist*.a test_skiplist *.o *.core TAGS test_list.h *.dSYM

distclean: clean
