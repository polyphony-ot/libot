#include <stdio.h>
#include <stdbool.h>
#include "unit.h"

extern results ot_tests();
extern results compose_tests();

int main() {
    fclose(stderr);

    RUN_SUITE(ot_tests);
    RUN_SUITE(compose_tests);

    printf("\n%d tests passed.\n"
           "%d tests failed.\n"
           "%d tests total.\n",
           passed, failed, passed + failed);

    if (failed > 0) {
        return 1;
    }
}
