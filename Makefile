PROJECT=	skiplist

TARGETS=	lib${PROJECT}.a test_${PROJECT} bench

WARN=		-Wall -Wshadow -Wuninitialized \
		-Wmissing-declarations \
		-Wmissing-prototypes \
		-pedantic -Wextra

CFLAGS+=	${WARN} -std=c99 -D_BSD_SOURCE
CFLAGS+=	-O3
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

benchmark: bench
	@./bench

libskiplist.a: skiplist.o
	${MAKE_LIB} skiplist.o

test_skiplist: skiplist-test.o test_alloc.o test_skiplist.o test_words.h
	${CC} -o test_skiplist ${CFLAGS} ${LDFLAGS} \
	skiplist-test.o test_alloc.o test_skiplist.o

bench: bench.c libskiplist.a
	${CC} -o $@ bench.c ${CFLAGS} ${BENCH_FLAGS} -L. -lskiplist ${LDFLAGS}

*.o: Makefile ${SKIPLIST_HEADERS}

skiplist.o: skiplist.c
	${CC} -c -o $@ skiplist.c ${CFLAGS} 

skiplist-test.o: skiplist.c test_config.h ${SKIPLIST_HEADERS}
	${CC} -c -o $@ -DSKIPLIST_LOCAL_INCLUDE=\"test_config.h\" \
	skiplist.c ${CFLAGS}

test_alloc.o: test_alloc.c

TAGS: skiplist.c ${SKIPLIST_HEADERS}
	etags *.[ch]

clean:
	rm -rf libskiplist*.a test_skiplist bench *.o *.core TAGS *.dSYM

# Installation
PREFIX ?=	/usr/local
INSTALL ?=	install
RM ?=		rm

install: lib${PROJECT}.a ${PROJECT}.h
	${INSTALL} -c lib${PROJECT}.a ${PREFIX}/lib
	${INSTALL} -c ${PROJECT}.h ${PREFIX}/include

uninstall:
	${RM} -f ${PREFIX}/lib/lib${PROJECT}.a
	${RM} -f ${PREFIX}/include/${PROJECT}.h

distclean: clean
