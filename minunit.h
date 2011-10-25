/* License
 *
 * You may use the code in this tech note for any purpose, with the
 * understanding that it comes with NO WARRANTY.
 *
 * Copied from http://www.jera.com/techinfo/jtns/jtn002.html. */

#ifndef MINUNIT_H
#define MINUNIT_H

 /* file: minunit.h */
 #define mu_assert(message, test) do { if (!(test)) return message; } while (0)
 #define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
 extern int tests_run;

#endif
