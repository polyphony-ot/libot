#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minunit.h"
#include "../ot.h"
#include "../compose.h"
#include "../xform.h"
#include "../array.h"
#include "../hex.h"
#include "../otdecode.h"
#include "../otencode.h"
#include "../sha1.h"
#include "../client.h"

MU_TEST(test_start_fmt_appends_correct_comp_type) {
    ot_comp_type expected_type = OT_FORMATTING_BOUNDARY;
    char* expected_name = "any name";
    char* expected_value = "any value";

    char parent[20] = { 0 };
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

    char parent[20] = { 0 };
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

    char parent[20] = { 0 };
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

    char parent[20] = { 0 };
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

    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_end_fmt(op, any_name, any_value);
    ot_end_fmt(op, any_name, any_value);

    size_t actual_comp_count = op->comps.len;

    ot_free_op(op);

    mu_assert(expected_comp_count == actual_comp_count, "Appended format did not have the correct name and value.");
}

MU_TEST(iter_next_on_empty_op) {
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_iter iter;

    ot_iter_init(&iter, op);
    bool actual = ot_iter_next(&iter);

    mu_assert(actual == false, "Expected number of iterations did not equal actual number of iterations");
    ot_free_op(op);
}

MU_TEST(iter_next_iterates_once_over_skip_with_count_one) {
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 1);
    ot_iter iter;

    ot_iter_init(&iter, op);
    bool first = ot_iter_next(&iter);
    bool second = ot_iter_next(&iter);

    mu_assert(first && !second, "Expected number of iterations did not equal actual number of iterations");
    ot_free_op(op);
}

MU_TEST(iter_next_iterates_correct_number_of_times_over_skip_with_count_greater_than_one) {
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 2);
    ot_iter iter;

    ot_iter_init(&iter, op);
    bool first = ot_iter_next(&iter);
    bool second = ot_iter_next(&iter);
    bool third = ot_iter_next(&iter);

    mu_assert(first && second && !third, "Expected number of iterations did not equal actual number of iterations");
    ot_free_op(op);
}

MU_TEST(iter_next_iterates_correctly_over_single_insert_component) {
    char parent[20] = { 0 };
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

    ot_free_op(op);
}

typedef struct ot_equals_test {
    char* op1;
    char* op2;
    bool equal;
} ot_equals_test;

ot_equals_test ot_equals_tests[] = {
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        true
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 0 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 0 } ] }",
        true
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 0 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }",
        false
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }",
        true
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }",
        "{ \"clientId\": 1, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }",
        false
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"cafebabe\", \"hash\": \"00\", \"components\": [ ] }",
        "{ \"clientId\": 0, \"parent\": \"cafebabe\", \"hash\": \"00\", \"components\": [ ] }",
        true
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 0 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 0 } ] }",
        true
    },
    (ot_equals_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 0 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        false
    },
};

MU_TEST(test_ot_equal) {
    size_t num_tests = sizeof(ot_equals_tests) / sizeof(ot_equals_test);
    for (size_t i = 0; i < num_tests; ++i) {
        ot_equals_test t = ot_equals_tests[i];

        char p[20] = { 0 };
        ot_op* op1 = ot_new_op(0, p);
        ot_err err = ot_decode(op1, t.op1);
        mu_assert(err == OT_ERR_NONE, "Error decoding test JSON.");

        ot_op* op2 = ot_new_op(0, p);
        err = ot_decode(op2, t.op2);
        mu_assert(err == OT_ERR_NONE, "Error decoding test JSON.");

        bool actual = ot_equal(op1, op2);
        mu_check(t.equal == actual);

        ot_free_op(op1);
        ot_free_op(op2);
    }
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
    MU_RUN_TEST(test_ot_equal);
}

/* compose tests */

typedef struct ot_compose_test {
    char* op1;
    char* op2;
    char* expected;
} ot_compose_test;

ot_compose_test ot_compose_tests[] = {
    /* skip, skip */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"skip\", \"count\": 2 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"skip\", \"count\": 2 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"skip\", \"count\": 2 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }"
    },
    /* skip, insert */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"abc\" } ] }"
    },
    /* skip, delete */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 2 }, { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 2 }, { \"type\": \"skip\", \"count\": 1 } ] }"
    },
    /* insert, skip */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"a\" }, { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"insert\", \"text\": \"b\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"a\" }, { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"insert\", \"text\": \"b\" } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdef\" } ] }"
    },
    /* insert, insert */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdef\" } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" }, { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 6 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdef\" }, { \"type\": \"skip\", \"count\": 3 } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"ghi\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdefghi\" } ] }"
    },
    /* insert, delete */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdef\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }"
    },
    /* delete, skip/insert/delete */
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }"
    },
    (ot_compose_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"abc\" } ] }"
    }
};

MU_TEST(compose_tests) {
    size_t max = sizeof(ot_compose_tests) / sizeof(ot_compose_test);
    for (size_t i = 0; i < max; ++i) {
        ot_compose_test t = ot_compose_tests[i];
        char errmsg[128];

        char p[20] = { 0 };
        ot_op* op1 = ot_new_op(0, p);
        ot_err err = ot_decode(op1, t.op1);
        mu_assert(err == OT_ERR_NONE, "Error decoding first test op.");

        ot_op* op2 = ot_new_op(0, p);
        err = ot_decode(op2, t.op2);
        mu_assert(err == OT_ERR_NONE, "Error decoding second test op.");

        ot_op* expected = ot_new_op(0, p);
        err = ot_decode(expected, t.expected);
        mu_assert(err == OT_ERR_NONE, "Error decoding expected test op.");

        ot_op* actual = ot_compose(op1, op2);
        sprintf(errmsg, "[%zu] Composed op wasn't correct.", i);
        mu_assert(ot_equal(expected, actual), errmsg);

        ot_free_op(op1);
        ot_free_op(op2);
        ot_free_op(expected);
        ot_free_op(actual);
    }
}

MU_TEST_SUITE(compose_test_suite) {
    MU_RUN_TEST(compose_tests);
}

/* xform tests */

typedef struct ot_xform_test {
    char* initial;
    char* op1;
    char* op2;
    char* expected;
} ot_xform_test;

ot_xform_test ot_xform_tests[] = {
    /* skip, skip */
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"skip\", \"count\": 2 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 2 }, { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }"
    },
    /* skip, insert */
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdef\" } ] }"
    },
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"ghi\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 6 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 6 }, { \"type\": \"insert\", \"text\": \"jkl\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"ghijkl\" } ] }"
    },
    /* skip, delete */
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }"
    },
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }"
    },
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 2 }, { \"type\": \"insert\", \"text\": \"b\" }, { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 2 }, { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"bc\" } ] }"
    },
    /* insert, insert */
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"ghi\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcdefghi\" } ] }"
    },
    /* insert, delete */
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }"
    },
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"skip\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }"
    },
    /* delete, delete */
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }"
    },
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 2 }, { \"type\": \"skip\", \"count\": 1 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 1 }, { \"type\": \"delete\", \"count\": 2 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }"
    },
    (ot_xform_test) {
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"def\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" }, { \"type\": \"delete\", \"count\": 3 } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 2 }, { \"type\": \"skip\", \"count\": 1 }, { \"type\": \"insert\", \"text\": \"gh\" } ] }",
        "{ \"clientId\": 0, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abcgh\" } ] }"
    },
};

MU_TEST(xform_tests) {
    char errmsg[128];
    size_t max = sizeof(ot_xform_tests) / sizeof(ot_xform_test);
    for (size_t i = 0; i < max; ++i) {
        ot_xform_test t = ot_xform_tests[i];

        char p[20] = { 0 };
        ot_op* initial = ot_new_op(0, p);
        ot_err err = ot_decode(initial, t.initial);
        mu_assert(err == OT_ERR_NONE, "Error decoding initial test op.");

        ot_op* op1 = ot_new_op(0, p);
        err = ot_decode(op1, t.op1);
        mu_assert(err == OT_ERR_NONE, "Error decoding first test op.");

        ot_op* op2 = ot_new_op(0, p);
        err = ot_decode(op2, t.op2);
        mu_assert(err == OT_ERR_NONE, "Error decoding second test op.");

        ot_op* expected = ot_new_op(0, p);
        err = ot_decode(expected, t.expected);
        mu_assert(err == OT_ERR_NONE, "Error decoding expected test op.");

        ot_xform_pair xform = ot_xform(op1, op2);

        ot_op* composed1 = ot_compose(initial, op1);
        ot_op* actual1 = ot_compose(composed1, xform.op2_prime);
        sprintf(errmsg, "[%zu] op2' wasn't correct.", i);
        mu_assert(ot_equal(expected, actual1), errmsg);

        ot_op* composed2 = ot_compose(initial, op2);
        ot_op* actual2 = ot_compose(composed2, xform.op1_prime);
        sprintf(errmsg, "[%zu] op1' wasn't correct.", i);
        mu_assert(ot_equal(expected, actual2), errmsg);

        ot_free_op(initial);
        ot_free_op(op1);
        ot_free_op(op2);
        ot_free_op(expected);
        ot_free_op(xform.op1_prime);
        ot_free_op(xform.op2_prime);
        ot_free_op(actual1);
        ot_free_op(actual2);
        ot_free_op(composed1);
        ot_free_op(composed2);
    }
}

MU_TEST_SUITE(xform_test_suite) {
    MU_RUN_TEST(xform_tests);
}

/* otdecode tests */

MU_TEST(decode_skip) {
    const uint32_t expected_client_id = 1234;
    char* expected_json = "{ \"clientId\": 1234, \"parent\": \"6162636465666768696a6b6c6d6e6f7071727374\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }";

    char p[20];
    ot_op* op = ot_new_op(0, p);
    ot_err err = ot_decode(op, expected_json);
    mu_check(err == OT_ERR_NONE);
    uint32_t actual_client_id = op->client_id;

    ot_free_op(op);

    mu_assert(expected_client_id == actual_client_id, "Parsed op did not have the correct clientId.");
}

MU_TEST(decode_client_id) {
    const uint32_t expected_client_id = 1234;
    char* expected_json = "{ \"clientId\": 1234, \"parent\": \"0\", \"hash\": \"00\", \"components\": [ ] }";

    char p[20];
    ot_op* op = ot_new_op(0, p);
    ot_err err = ot_decode(op, expected_json);
    mu_check(err == OT_ERR_NONE);
    uint32_t actual_client_id = op->client_id;

    ot_free_op(op);

    mu_assert(expected_client_id == actual_client_id, "Parsed op did not have the correct clientId.");
}

MU_TEST(decode_parent) {
    uint8_t expected_parent[] = "abcdefghijklmnopqrst";
    char* expected_json = "{ \"clientId\": 1234, \"parent\": \"6162636465666768696a6b6c6d6e6f7071727374\", \"hash\": \"00\", \"components\": [ ] }";

    char p[20];
    ot_op* op = ot_new_op(0, p);
    ot_err err = ot_decode(op, expected_json);
    mu_check(err == OT_ERR_NONE);
    char* actual_parent = op->parent;
    int cmp = memcmp(expected_parent, actual_parent, 20);

    ot_free_op(op);

    mu_assert(cmp == 0, "Parsed op did not have the correct parent.");
}

MU_TEST(decode_fails_if_client_id_is_missing) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    const char* json = "{ \"parent\": \"6162636465666768696a6b6c6d6e6f7071727374\", \"components\": [ ] }";
    ot_err err = ot_decode(op, json);

    mu_assert(err == OT_ERR_CLIENT_ID_MISSING, "Decode did not return the correct error for clientId missing.");
    ot_free_op(op);
}

MU_TEST(decode_fails_if_parent_is_missing) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    const char* json = "{ \"clientId\": 1234, \"components\": [ ] }";
    ot_err err = ot_decode(op, json);

    mu_assert(err == OT_ERR_PARENT_MISSING, "Decode did not return the correct error for parent missing.");
    ot_free_op(op);
}

MU_TEST(decode_fails_if_components_is_missing) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    const char* json = "{ \"clientId\": 1234, \"parent\": \"0\", \"hash\": \"00\" }";
    ot_err err = ot_decode(op, json);

    mu_assert(err == OT_ERR_COMPONENTS_MISSING, "Decode did not return the correct error for components missing.");
    ot_free_op(op);
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
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    char msg[1024];
    sprintf(msg, "Serializing empty op did not create expected string. Expected = \"%s\". Actual = \"%s\".", EXPECTED, actual);
    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, msg);
}

MU_TEST(test_serialize_single_insert) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"any string\" } ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "any string");

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    char msg[1024];
    sprintf(msg, "Serializing a single insert did not create expected string. Expected = \"%s\". Actual = \"%s\".", EXPECTED, actual);
    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, msg);
}

MU_TEST(test_serialize_two_inserts) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"any string any other string\" } ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "any string");
    ot_insert(op, " any other string");

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing two inserts did not create expected string.");
}

MU_TEST(test_serialize_single_skip) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 1 } ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_skip(op, 1);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single skip did not create expected string.");
}

MU_TEST(test_serialize_single_delete) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"delete\", \"count\": 1 } ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_delete(op, 1);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single delete did not create expected string.");
}

MU_TEST(test_serialize_single_open_element) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"openElement\", \"element\": \"any string\" } ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_open_element(op, "any string");

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single openElement did not create expected string.");
}

MU_TEST(test_serialize_single_close_element) {
    const char* const EXPECTED = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"closeElement\" } ] }";
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_close_element(op);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

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

    char actual[3] = { 0 };
    atohex(actual, ARRAY, sizeof(ARRAY));
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    char msg[1024];
    sprintf(msg, "Encoding a single byte gave an incorrect result. Expected = \"%s\". Actual = \"%.2s\".", EXPECTED, actual);
    mu_assert(cmp == 0, msg);
}

MU_TEST(atohex_encodes_multiple_bytes) {
    const char* const EXPECTED = "01ff1a";
    const char ARRAY[] = { 0x01, 0xFF, 0x1A };

    char actual[7] = { 0 };
    atohex(actual, ARRAY, sizeof(ARRAY));
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    char msg[1024];
    sprintf(msg, "Encoding multiple bytes gave an incorrect result. Expected = \"%s\". Actual = \"%.6s\".", EXPECTED, actual);
    mu_assert(cmp == 0, msg);
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

/* sha1 tests */

typedef struct sha1_test {
    char* in;
    char* hash;
} sha1_test;

sha1_test sha1_tests[] = {
    (sha1_test) { "", (char[20]) { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 } },
    (sha1_test) { "hello world", (char[20]) { 0x2a, 0xae, 0x6c, 0x35, 0xc9, 0x4f, 0xcf, 0xb4, 0x15, 0xdb, 0xe9, 0x5f, 0x40, 0x8b, 0x9c, 0xe9, 0x1e, 0xe8, 0x46, 0xed } },
    (sha1_test) { "lorem ipsum dolor sit amet consetetur sadipscing elitr sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam", (char[20]) { 0x78, 0x34, 0x63, 0x8a, 0xa8, 0xeb, 0x18, 0xbb, 0x43, 0x95, 0x2d, 0x91, 0x9e, 0xfb, 0x49, 0x1e, 0xe4, 0x82, 0x56, 0x45 } }
};

MU_TEST(sha1) {
    size_t max = sizeof(sha1_tests) / sizeof(sha1_test);
    for (size_t i = 0; i < max; ++i) {
        sha1_test t = sha1_tests[i];
        hash_state hs;

        int result = sha1_init(&hs);
        mu_assert(result == CRYPT_OK, "An error occurred initializing the hash state.");

        result = sha1_process(&hs, t.in, (uint32_t)strlen(t.in));
        mu_assert(result == CRYPT_OK, "An error occurred processing the hash input.");

        char hash[20];
        result = sha1_done(&hs, hash);
        mu_assert(result == CRYPT_OK, "An error occurred getting the hash.");

        for (size_t j = 0; j < 20; ++j) {
            if (hash[j] != t.hash[j]) {
                mu_fail("Computed hash was incorrect.");
            }
        }
    }
}

MU_TEST_SUITE(sha1_test_suite) {
    MU_RUN_TEST(sha1);
}

/* client tests */

char* sent_op = "";
int send_ret = 0;

static int send_stub(const char* op) {
    sent_op = strdup(op);
    return send_ret;
}

ot_event_type event_type = 0;
ot_op* event_op = NULL;
int event_ret = 0;

static int event_stub(ot_event_type t, ot_op* op) {
    event_type = (const ot_event_type)t;
    event_op = op;
    return event_ret;
}

MU_TEST(client_has_correct_state_and_event_when_receiving_op_for_empty_doc) {
    const uint32_t NONZERO = 1;
    const char* const EXPECTED = "any string";
    ot_client* client = ot_new_client(send_stub, event_stub, NONZERO);
    char* enc_op = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"any string\" } ] }";

    ot_client_receive(client, enc_op);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the document state to be \"%s\", but instead got \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    if (event_type != OT_OP_APPLIED) {
        char* msg;
        asprintf(&msg, "Expected an event of type OT_OP_APPLIED (%d), but instead got %d.", OT_OP_APPLIED, event_type);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
}

MU_TEST(client_has_correct_state_and_event_when_receiving_op_for_non_empty_doc) {
    const uint32_t NONZERO = 1;
    const char* const EXPECTED = "abcdef";
    const char* const ENC_OP1 = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }";
    const char* const ENC_OP2 = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"def\" } ] }";
    ot_client* client = ot_new_client(send_stub, event_stub, NONZERO);

    ot_client_receive(client, ENC_OP1);
    ot_client_receive(client, ENC_OP2);

    char* actual = ot_snapshot(client->doc->composed);
    const int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the document state to be \"%s\", but instead got \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    if (event_type != OT_OP_APPLIED) {
        char* msg;
        asprintf(&msg, "Expected an event of type OT_OP_APPLIED (%d), but instead got %d.", OT_OP_APPLIED, event_type);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
}

MU_TEST(client_receive_does_not_send_empty_buffer_after_acknowledgement) {
    ot_client* client = ot_new_client(send_stub, event_stub, 0);
    char* op = "{ \"clientId\": 0, \"parent\": \"0\", \"components\": [ ] }";
    char* nothing = "NOTHING";
    sent_op = nothing;

    ot_client_receive(client, op);

    if (sent_op != nothing) {
        char* msg;
        asprintf(&msg, "Expected the client to not send anything, but it sent \"%s\".", sent_op);
        mu_fail(msg);
    }

    ot_free_client(client);
}

MU_TEST(client_apply_sends_op_if_not_waiting_for_acknowledgement) {
    ot_client* client = ot_new_client(send_stub, event_stub, 0);
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "any string");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    ot_op* dec_sent_op = ot_new_op(0, parent);
    ot_err derr = ot_decode(dec_sent_op, sent_op);

    mu_assert_int_eq(OT_ERR_NONE, derr);
    mu_assert(ot_equal(op, dec_sent_op), "Sent op wasn't equal to the applied op.");

    ot_free_op(dec_sent_op);
    ot_free_client(client);
    free(sent_op);
}

MU_TEST(client_receives_new_op_before_acknowledgement_starting_with_empty_doc) {
    const char* const EXPECTED = "client text server text";
    ot_client* client = ot_new_client(send_stub, event_stub, 0);
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "client text ");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    char* enc_serv_op = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"d82ac619d64a0883de5276f0f3e9a984c3e22620\", \"components\": [ { \"type\": \"insert\", \"text\": \"server text\" } ] }";
    ot_client_receive(client, enc_serv_op);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the client's document to be \"%s\", but instead it's \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
    free(sent_op);
}

MU_TEST(client_receives_multiple_ops_before_acknowledgement_starting_with_empty_doc) {
    const char* const EXPECTED = "client text server text more server text";
    ot_client* client = ot_new_client(send_stub, event_stub, 0);
    char parent[20] = { 0 };
    ot_op* op = ot_new_op(0, parent);
    ot_insert(op, "client text ");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    char* enc_serv_op = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"d82ac619d64a0883de5276f0f3e9a984c3e22620\", \"components\": [ { \"type\": \"insert\", \"text\": \"server text\" } ] }";
    char* enc_serv_op2 = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"66a02881b5b0d0d4e2e40b2bfa3d6e3ca710c85c\", \"components\": [ { \"type\": \"skip\", \"count\": 11 }, { \"type\": \"insert\", \"text\": \" more server text\" } ] }";
    ot_client_receive(client, enc_serv_op);
    ot_client_receive(client, enc_serv_op2);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the client's document to be \"%s\", but instead it's \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
    free(sent_op);
}

MU_TEST_SUITE(client_test_suite) {
    MU_RUN_TEST(client_has_correct_state_and_event_when_receiving_op_for_empty_doc);
    MU_RUN_TEST(client_has_correct_state_and_event_when_receiving_op_for_non_empty_doc);
    MU_RUN_TEST(client_receive_does_not_send_empty_buffer_after_acknowledgement);
    MU_RUN_TEST(client_apply_sends_op_if_not_waiting_for_acknowledgement);
    MU_RUN_TEST(client_receives_new_op_before_acknowledgement_starting_with_empty_doc);
    MU_RUN_TEST(client_receives_multiple_ops_before_acknowledgement_starting_with_empty_doc);
}

int main() {
    // Close stderr to avoid littering test output with log messages. This line
    // can be removed to aid in debugging tests.
    fclose(stderr);

    MU_RUN_SUITE(ot_test_suite);
    MU_RUN_SUITE(compose_test_suite);
    MU_RUN_SUITE(xform_test_suite);
    MU_RUN_SUITE(otdecode_test_suite);
    MU_RUN_SUITE(otencode_test_suite);
    MU_RUN_SUITE(array_test_suite);
    MU_RUN_SUITE(hex_test_suite);
    MU_RUN_SUITE(sha1_test_suite);
    MU_RUN_SUITE(client_test_suite);
    MU_REPORT();
    return 0;
}
