# skiplist Changes By Release

## v. 0.9.0 - 2016-06-18

### API Changes

Removed ability to define comparison callback at compile-time.
`skiplist_new` always takes a comparison function pointer,
which is required.

Memory allocation callback (and a user data `void *`) are now
arguments to `skiplist_new`. Both are optional, and can be NULL.

Changed the iteration API to use a CONTINUE/HALT enum return value,
since a real return value of a user-chosen type can still go in the
environment passed to the callback.

`first, last, pop_first, pop_last`, and others now return `bool`
rather than an int (which was used like a bool).

Use more appropriate numeric types: `size_t` rather than
`long` for `skiplist_count`'s return value, for example.


### Other Improvements

Added semantic versioning.

Fixed warnings, general code style cleanup.

Switched to using greatest for tests, so a single failure doesn't halt
the whole test runner.

Improved the benchmark formatting.

Deleted tentative references (mostly in the configuration file) to a
optional mutexes -- that has not been implemented. If a future version
adds mutexes (or lockless operations), it will be an interface change
and reflected in the version.


## v. [no version] - 2012-05-28

Some early API changes.


## v. [no version] - 2011-10-24

Initial public version.

