#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minunit.h"
#include "../ot.h"
#include "../array.h"
#include "../hex.h"
#include "../otdecode.h"
#include "../otencode.h"

MU_TEST(test_start_fmt_appends_correct_comp_type) {
    ot_comp_type expected_type = OT_FORMATTING_BOUNDARY;
    char* expected_name = "any name";
    char* expected_value = "any value";
    
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_start_fmt(op, expected_name, expected_value);
    
    ot_comp* comps = op->comps.data;
    ot_comp_type actual_type = comps[0].type;
    int equal = expected_type == actual_type;
    
    ot_free_op(op);
    
    mu_assert(equal, "Starting a format did not append a component of type OT_FORMATTING_BOUNDARY.");
}

MU_TEST(test_start_fmt_appends_correct_name_and_value) {
    char* expected_name = "any name";
    char* expected_value = "any value";
    
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_start_fmt(op, expected_name, expected_value);
    
    ot_comp* comps = op->comps.data;
    ot_fmt* data = comps[0].value.fmtbound.start.data;
    char* actual_name = data[0].name;
    char* actual_value = data[0].value;
    
    int cmp_name = strcmp(expected_name, actual_name);
    int cmp_value = strcmp(expected_value, actual_value);
    
    ot_free_op(op);
    
    mu_assert(cmp_name == 0 && cmp_value == 0, "Appended format did not have the correct name and value.");
}

MU_TEST(test_start_fmt_does_not_append_another_fmtbound_when_last_component_is_fmtbound) {
    const size_t expected_comp_count = 1;
    char* any_name = "any name";
    char* any_value = "any value";
    
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_start_fmt(op, any_name, any_value);
    ot_start_fmt(op, any_name, any_value);
    
    size_t actual_comp_count = op->comps.len;
    
    ot_free_op(op);
    
    mu_assert(expected_comp_count == actual_comp_count, "Appended format did not have the correct name and value.");
}

MU_TEST(test_end_fmt_appends_correct_name_and_value) {
    char* expected_name = "any name";
    char* expected_value = "any value";
    
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_end_fmt(op, expected_name, expected_value);
    
    ot_comp* comps = op->comps.data;
    ot_fmt* data = comps[0].value.fmtbound.end.data;
    char* actual_name = data[0].name;
    char* actual_value = data[0].value;
    
    int cmp_name = strcmp(expected_name, actual_name);
    int cmp_value = strcmp(expected_value, actual_value);
    
    ot_free_op(op);
    
    mu_assert(cmp_name == 0 && cmp_value == 0, "Appended format did not have the correct name and value.");
}

MU_TEST(test_end_fmt_does_not_append_another_fmtbound_when_last_component_is_fmtbound) {
    const size_t expected_comp_count = 1;
    char* any_name = "any name";
    char* any_value = "any value";
    
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_end_fmt(op, any_name, any_value);
    ot_end_fmt(op, any_name, any_value);
    
    size_t actual_comp_count = op->comps.len;
    
    ot_free_op(op);
    
    mu_assert(expected_comp_count == actual_comp_count, "Appended format did not have the correct name and value.");
}

MU_TEST(iter_next_on_empty_op) {
    char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_iter iter;
    
    ot_iter_init(&iter, op);
    bool actual = ot_iter_next(&iter);
    
    mu_assert(actual == false, "Expected number of iterations did not equal actual number of iterations");
}

MU_TEST(iter_next_iterates_once_over_skip_with_count_one) {
    char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 1);
    ot_iter iter;
    
    ot_iter_init(&iter, op);
    bool first = ot_iter_next(&iter);
    bool second = ot_iter_next(&iter);
    
    mu_assert(first && !second, "Expected number of iterations did not equal actual number of iterations");
}

MU_TEST(iter_next_iterates_correct_number_of_times_over_skip_with_count_greater_than_one) {
    char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 2);
    ot_iter iter;
    
    ot_iter_init(&iter, op);
    bool first = ot_iter_next(&iter);
    bool second = ot_iter_next(&iter);
    bool third = ot_iter_next(&iter);
    
    mu_assert(first && second && !third, "Expected number of iterations did not equal actual number of iterations");
}

MU_TEST(iter_next_iterates_correctly_over_single_insert_component) {
    char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "012");
    ot_iter iter;
    
    ot_iter_init(&iter, op);
    ot_iter_next(&iter);
    mu_assert(iter.pos == 0, "Iterator position was not 0.");
    mu_assert(iter.offset == 0, "Iterator offset was not 0.");
    ot_iter_next(&iter);
    mu_assert(iter.pos == 0, "Iterator position was not 0.");
    mu_assert(iter.offset == 1, "Iterator offset was not 1.");
    ot_iter_next(&iter);
    mu_assert(iter.pos == 0, "Iterator position was not 1.");
    mu_assert(iter.offset == 2, "Iterator offset was not 2.");
}

MU_TEST_SUITE(ot_test_suite) {
    MU_RUN_TEST(test_start_fmt_appends_correct_comp_type);
    MU_RUN_TEST(test_start_fmt_appends_correct_name_and_value);
    MU_RUN_TEST(test_start_fmt_does_not_append_another_fmtbound_when_last_component_is_fmtbound);
    MU_RUN_TEST(test_end_fmt_appends_correct_name_and_value);
    MU_RUN_TEST(test_end_fmt_does_not_append_another_fmtbound_when_last_component_is_fmtbound);
    MU_RUN_TEST(iter_next_on_empty_op);
    MU_RUN_TEST(iter_next_iterates_once_over_skip_with_count_one);
    MU_RUN_TEST(iter_next_iterates_correct_number_of_times_over_skip_with_count_greater_than_one);
    MU_RUN_TEST(iter_next_iterates_correctly_over_single_insert_component);
}

/* otdecode tests */

MU_TEST(decode_skip) {
    const int64_t expected_client_id = 1234;
    char* expected_json = "{ \"clientId\": 1234, \"parent\": \"6162636465666768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }";
    
    char p[64];
    ot_op* op = ot_new_op(0, p);
	ot_decode_err err = ot_decode(op, expected_json);
    mu_check(err == OT_ERR_NONE);
    int64_t actual_client_id = op->client_id;
    
    ot_free_op(op);
    
    mu_assert(expected_client_id == actual_client_id, "Parsed op did not have the correct clientId.");
}

MU_TEST(decode_client_id) {
    const int64_t expected_client_id = 1234;
    char* expected_json = "{ \"clientId\": 1234, \"parent\": \"0\", \"components\": [ ] }";
    
	char p[64];
    ot_op* op = ot_new_op(0, p);
	ot_decode_err err = ot_decode(op, expected_json);
    mu_check(err == OT_ERR_NONE);
    int64_t actual_client_id = op->client_id;
    
    ot_free_op(op);
    
    mu_assert(expected_client_id == actual_client_id, "Parsed op did not have the correct clientId.");
}

MU_TEST(decode_parent) {
    uint8_t expected_parent[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijkl";
    char* expected_json = "{ \"clientId\": 1234, \"parent\": \"6162636465666768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c\", \"components\": [ ] }";
    
	char p[64];
    ot_op* op = ot_new_op(0, p);
	ot_decode_err err = ot_decode(op, expected_json);
    mu_check(err == OT_ERR_NONE);
    char* actual_parent = op->parent;
    int cmp = memcmp(expected_parent, actual_parent, 64);
    
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Parsed op did not have the correct parent.");
}

MU_TEST(decode_fails_if_client_id_is_missing) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    const char* json = "{ \"parent\": \"6162636465666768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c\", \"components\": [ ] }";
    ot_decode_err err = ot_decode(op, json);
    
    mu_assert(err == OT_ERR_CLIENT_ID_MISSING, "Decode did not return the correct error for clientId missing.");
}

MU_TEST(decode_fails_if_parent_is_missing) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    const char* json = "{ \"clientId\": 1234, \"components\": [ ] }";
    ot_decode_err err = ot_decode(op, json);
    
    mu_assert(err == OT_ERR_PARENT_MISSING, "Decode did not return the correct error for parent missing.");
}

MU_TEST(decode_fails_if_components_is_missing) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    const char* json = "{ \"clientId\": 1234, \"parent\": \"0\" }";
    ot_decode_err err = ot_decode(op, json);
    
    mu_assert(err == OT_ERR_COMPONENTS_MISSING, "Decode did not return the correct error for components missing.");
}

MU_TEST_SUITE(otdecode_test_suite) {
    MU_RUN_TEST(decode_skip);
    MU_RUN_TEST(decode_client_id);
    MU_RUN_TEST(decode_parent);
    MU_RUN_TEST(decode_fails_if_client_id_is_missing);
    MU_RUN_TEST(decode_fails_if_parent_is_missing);
    MU_RUN_TEST(decode_fails_if_components_is_missing);
}

/* otencode tests */

MU_TEST(test_serialize_empty_op) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing empty op did not create expected string.");
}

MU_TEST(test_serialize_single_insert) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"any string\" } ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "any string");
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing a single insert did not create expected string.");
}

MU_TEST(test_serialize_two_inserts) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"any string any other string\" } ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "any string");
    ot_insert(op, " any other string");
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing two inserts did not create expected string.");
}

MU_TEST(test_serialize_single_skip) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 1);
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing a single skip did not create expected string.");
}

MU_TEST(test_serialize_single_delete) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 1 } ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_delete(op, 1);
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing a single delete did not create expected string.");
}

MU_TEST(test_serialize_single_open_element) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ { \"type\": \"openElement\", \"element\": \"any string\" } ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_open_element(op, "any string");
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing a single openElement did not create expected string.");
}

MU_TEST(test_serialize_single_close_element) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"components\": [ { \"type\": \"closeElement\" } ] }";
	char parent[64] = { 0 };
	ot_op* op = ot_new_op(0, parent);
    ot_close_element(op);
    
    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*) actual);
    
    free(actual);
    ot_free_op(op);
    
    mu_assert(cmp == 0, "Serializing a single closeElement did not create expected string.");
}

MU_TEST_SUITE(otencode_test_suite) {
    MU_RUN_TEST(test_serialize_empty_op);
    MU_RUN_TEST(test_serialize_single_insert);
    MU_RUN_TEST(test_serialize_two_inserts);
    MU_RUN_TEST(test_serialize_single_skip);
    MU_RUN_TEST(test_serialize_single_delete);
    MU_RUN_TEST(test_serialize_single_open_element);
    MU_RUN_TEST(test_serialize_single_close_element);
}

/* array tests */

MU_TEST(ensure_size_on_empty_array_increases_capacity_to_one) {
    const size_t EXPECTED = 1;
    array a;
    array_init(&a, 0);
    
    array_ensure_size(&a);
    const size_t ACTUAL = a.cap;
    array_free(&a);
    
    mu_assert(EXPECTED == ACTUAL, "Ensuring the size of an empty array did not increase its capacity to 1.");
}

MU_TEST(ensure_size_on_non_empty_array_doubles_capacity) {
    const size_t EXPECTED = 4;
    array a;
    array_init(&a, 0);
    // Set length and capacity to 2 to simulate a non-empty, full array. Use 2
    // so we test that the capacity is actually doubled and not just
    // incremented.
    a.len = 2;
    a.cap = 2;
    
    array_ensure_size(&a);
    const size_t ACTUAL = a.cap;
    array_free(&a);
    
    mu_assert(EXPECTED == ACTUAL, "Ensuring the size of a non-empty array did not double its capacity.");
}

MU_TEST(append_increases_array_length_by_one) {
    const size_t EXPECTED = 1;
    array a;
    array_init(&a, 0);
    
    array_append(&a);
    const size_t ACTUAL = a.len;
    array_free(&a);
    
    mu_assert(EXPECTED == ACTUAL, "Appending to an array did not increase its length by 1.");
}

MU_TEST_SUITE(array_test_suite) {
    MU_RUN_TEST(ensure_size_on_empty_array_increases_capacity_to_one);
    MU_RUN_TEST(ensure_size_on_non_empty_array_doubles_capacity);
    MU_RUN_TEST(append_increases_array_length_by_one);
}

/* hex tests */

MU_TEST(hextoa_does_not_write_to_array_when_hex_is_empty) {
    const char EXPECTED = 1;
    char a[] = { EXPECTED };
    
    hextoa(a, "", 0);
    const char ACTUAL = a[0];
    
    mu_assert(EXPECTED == ACTUAL, "Converting an empty hex string to an array should not write anything to the array.");
}

MU_TEST(hextoa_decodes_single_byte) {
    const char* const EXPECTED = "a";
    
    char actual[1];
    hextoa(actual, "61", 2);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Decoding a single byte gave an incorrect result.");
}

MU_TEST(hextoa_decodes_multiple_bytes) {
    const char* const EXPECTED = "abc";
    
    char actual[3];
    hextoa(actual, "616263", 6);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Decoding multiple bytes gave an incorrect result.");
}

MU_TEST(hextoa_decodes_mixed_case_letters) {
    const char* const EXPECTED = "JZ";
    
    char actual[2];
    hextoa(actual, "4A5a", 4);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Decoding mixed-case letters gave an incorrect result.");
}

MU_TEST(hextoa_decodes_lowercase_letters) {
    const char* const EXPECTED = "JZ";
    
    char actual[2];
    hextoa(actual, "4a5a", 4);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Decoding lowercase letters gave an incorrect result.");
}

MU_TEST(hextoa_decodes_uppercase_letters) {
    const char* const EXPECTED = "JZ";
    
    char actual[2];
    hextoa(actual, "4A5A", 4);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Decoding uppercase letters gave an incorrect result.");
}

MU_TEST(atohex_encodes_single_byte) {
    const char* const EXPECTED = "01";
    const char ARRAY[] = { 0x01 };
    
    char actual[2];
    atohex(actual, ARRAY, sizeof(ARRAY));
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Encoding a single byte gave an incorrect result.");
}

MU_TEST(atohex_encodes_multiple_bytes) {
    const char* const EXPECTED = "01ff1a";
    const char ARRAY[] = { 0x01, 0xFF, 0x1A };
    
    char actual[6];
    atohex(actual, ARRAY, sizeof(ARRAY));
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));
    
    mu_assert(cmp == 0, "Encoding multiple bytes gave an incorrect result.");
}

MU_TEST_SUITE(hex_test_suite) {
    MU_RUN_TEST(hextoa_does_not_write_to_array_when_hex_is_empty);
    MU_RUN_TEST(hextoa_decodes_single_byte);
    MU_RUN_TEST(hextoa_decodes_multiple_bytes);
    MU_RUN_TEST(hextoa_decodes_mixed_case_letters);
    MU_RUN_TEST(hextoa_decodes_lowercase_letters);
    MU_RUN_TEST(hextoa_decodes_uppercase_letters);
    MU_RUN_TEST(atohex_encodes_single_byte);
    MU_RUN_TEST(atohex_encodes_multiple_bytes);
}

int main() {
	MU_RUN_SUITE(ot_test_suite);
    MU_RUN_SUITE(otdecode_test_suite);
    MU_RUN_SUITE(otencode_test_suite);
    MU_RUN_SUITE(array_test_suite);
    MU_RUN_SUITE(hex_test_suite);
	MU_REPORT();
	return 0;
}
