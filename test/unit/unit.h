/*
    This header contains a minimal unit testing framework.
*/

#include <stdio.h>
#include <stdbool.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

// Debugging macros for creating a string containing the current line number.
// These are used by assertions so they can output where a test fails.
#define S(x) #x
#define S_(x) S(x)
#define STRLINE S_(__LINE__)

static char* msg = NULL;
static int passed = 0;
static int failed = 0;

typedef struct results {
    int passed;
    int failed;
} results;

#define RUN_TEST(f)                                                            \
    bool f(char**);                                                            \
    msg = NULL;                                                                \
    printf("Running %s... ", #f);                                              \
    if (f(&msg)) {                                                             \
        puts("passed.");                                                       \
        ++passed;                                                              \
    } else {                                                                   \
        puts("failed.");                                                       \
        ++failed;                                                              \
    }                                                                          \
    if (msg) {                                                                 \
        printf("\t%s\n", msg);                                                 \
    }

#define RUN_SUITE(f)                                                           \
    results res = f();                                                         \
    passed += res.passed;                                                      \
    failed += res.failed;

#define ASSERT_INT_EQUAL(expected, actual, detail, msg)                        \
    if (!assert_int_equal(expected, actual, detail, "Line #" STRLINE, msg)) {  \
        return false;                                                          \
    }

static bool assert_int_equal(int expected, int actual, char* detail, char* loc,
                             char** msg) {

    if (expected == actual) {
        return true;
    }

    asprintf(msg, "%s: %s\n"
                  "\tActual: %d\n"
                  "\tExpected: %d",
             loc, detail, actual, expected);
    return false;
}

#pragma clang diagnostic pop
