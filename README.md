An ISC-licensed C library for skiplists. It currently requires C99 (for
__VA_ARGS__ and stack-allocated arrays), but could be converted to
C89 without a whole lot of work, if necessary.

For more information on skiplists, see William Pugh's paper,
"Skip Lists: A Probabilistic Alternative to Balanced Trees".

The `skiplist.h` file describes the interface.

`skiplist_config.h` contains a couple compile-time configuration options.

For further usage examples, see the test suite.
