#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minunit.h"
#include "../ot.h"

int tests_run = 0;

static char* test_serialize_empty_op() {
    const char* expected = "[]";
	int64_t parent[8] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    
    uint8_t* actual = ot_serialize(op);
    int cmp = strcmp(expected, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert("Serializing empty op did not create expected string.", cmp == 0);
    
	return 0;
}

static char* test_serialize_single_insert() {
    const char* expected = "[ { \"type\": \"insert\", \"text\": \"any string\" } ]";
	int64_t parent[8] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_insert(op, (uint8_t*) "any string");
    
    uint8_t* actual = ot_serialize(op);
    int cmp = strcmp(expected, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert("Serializing a single insert did not create expected string.", cmp == 0);
    
	return 0;
}

static char* test_serialize_two_inserts() {
    const char* expected = "[ { \"type\": \"insert\", \"text\": \"any string\" }, { \"type\": \"insert\", \"text\": \"any other string\" } ]";
	int64_t parent[8] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_insert(op, (uint8_t*) "any string");
    ot_insert(op, (uint8_t*) "any other string");
    
    uint8_t* actual = ot_serialize(op);
    int cmp = strcmp(expected, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert("Serializing two inserts did not create expected string.", cmp == 0);
    
	return 0;
}

static char* test_serialize_skip() {
    const char* expected = "[ { \"type\": \"skip\", \"count\": 1 } ]";
	int64_t parent[8] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 1);
    
    uint8_t* actual = ot_serialize(op);
    int cmp = strcmp(expected, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert("Serializing a single skip did not create expected string.", cmp == 0);
    
	return 0;
}

static char* all_tests() {
    mu_run_test(test_serialize_empty_op);
    mu_run_test(test_serialize_single_insert);
    mu_run_test(test_serialize_two_inserts);
    mu_run_test(test_serialize_skip);

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
