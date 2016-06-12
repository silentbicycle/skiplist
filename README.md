# skiplist

A skiplist-based ordered collection library.

## Key Features:

- Getting or popping the first or last value in the skiplist
    is very cheap.

- Keys can have multiple values associated with them, if
    `skiplist_add` is used instead of `skiplist_set`.

- The skiplist can be iterated over from the start, or
    beginning at an arbitrary key.

- This library is distributed under the ISC License. You can use it
    freely, even for commercial purposes.

This library currently requires C99 (for `__VA_ARGS__`, stack-allocated
arrays, and numeric types), but could be converted to C89 without a
whole lot of work.

For more information on skiplists, see William Pugh's paper,
"Skip Lists: A Probabilistic Alternative to Balanced Trees".

The `skiplist.h` file describes the interface.

`skiplist_config.h` contains a couple compile-time configuration options.

For further usage examples, see the test suite in `test_skiplist.c` and
the benchmark suite in `bench.c`.
