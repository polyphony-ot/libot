#include <stdbool.h>
#include <stdio.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define RUN_SCENARIO(f)                                    \
    bool f(char**);                                        \
    ++total;                                               \
    msg = NULL;                                            \
    printf("Running %s... ", #f);                          \
    if (f(&msg)) {                                         \
        puts(ANSI_COLOR_GREEN "passed." ANSI_COLOR_RESET); \
        ++passed;                                          \
    } else {                                               \
        puts(ANSI_COLOR_RED "failed." ANSI_COLOR_RESET);   \
        ++failed;                                          \
    }                                                      \
    if (msg) {                                             \
        printf("\t%s\n", msg);                             \
    }

char* msg = NULL;
int passed = 0;
int failed = 0;
int total = 0;

int main() {
    RUN_SCENARIO(scenario1);

    char* pass_color = "";
    char* fail_color = "";
    if (failed == 0) {
        pass_color = ANSI_COLOR_GREEN;
    } else {
        fail_color = ANSI_COLOR_RED;
    }

    printf("\n%s%d tests passed." ANSI_COLOR_RESET "\n"
           "%s%d tests failed." ANSI_COLOR_RESET "\n"
           "%d tests total.\n",
           pass_color, passed, fail_color, failed, total);

    if (failed > 0) {
        return 1;
    }
}
