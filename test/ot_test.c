#include <stdio.h>
#include <stdlib.h>
#include "minunit.h"
#include "../ot.h"

int tests_run = 0;

static char* test_foo() {
	int64_t parent[8] = { 0 };
	ot_op* op = ot_new_op(0, parent);
	ot_insert(op, "Hello world!");
	ot_insert(op, " It's me again, world!");

    uint8_t* snapshot = ot_snapshot(op);
	puts(snapshot);
    free(snapshot);

    int64_t actual = op->comp_count;
    ot_free_op(op);
    
	mu_assert("Count should be 1.", actual == 1);
    
    

	return 0;
}

static char* all_tests() {
	mu_run_test(test_foo);

	return 0;
}

int main() {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
