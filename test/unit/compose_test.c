#include "../../ot.h"
#include "../../compose.h"
#include "../../encode.h"
#include "unit.h"

bool param_compose_test(ot_op* op1, ot_op* op2, ot_op* expected, char** msg) {
    ot_op* actual = ot_compose(op1, op2);
    ot_free_op(op1);
    ot_free_op(op2);

    if (actual == NULL) {
        if (expected == NULL) {
            return true;
        } else {
            FAIL("Operations couldn't be composed.", msg);
        }
    }
    bool equal = ot_equal(expected, actual);
    char* expected_enc = ot_encode(expected);
    char* actual_enc = ot_encode(actual);
    ASSERT_CONDITION(equal, expected_enc, actual_enc,
                     "Composed operation wasn't correct.", msg);

    ot_free_op(expected);
    ot_free_op(actual);
    free(expected_enc);
    free(actual_enc);
    return true;
}

bool compose_skip_skip(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* expected = ot_new_op();
    ot_skip(expected, NONZERO_INT);

    ot_op* op1 = ot_new_op();
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, NONZERO_INT);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_skip_insert(char** msg) {
    const int NONZERO_INT = 1;
    const char* const NONEMPTY_STRING = "abc";

    ot_op* expected = ot_new_op();
    ot_insert(expected, NONEMPTY_STRING);
    ot_skip(expected, NONZERO_INT);

    ot_op* op1 = ot_new_op();
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, NONEMPTY_STRING);
    ot_skip(op2, NONZERO_INT);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_skip_delete(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* expected = ot_new_op();
    ot_delete(expected, NONZERO_INT);

    ot_op* op1 = ot_new_op();
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_delete(op2, NONZERO_INT);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_insert_skip(char** msg) {
    const char* const NONEMPTY_STRING = "abc";
    const int insert_len = strlen(NONEMPTY_STRING);

    ot_op* expected = ot_new_op();
    ot_insert(expected, NONEMPTY_STRING);

    ot_op* op1 = ot_new_op();
    ot_insert(op1, NONEMPTY_STRING);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, insert_len);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_insert_insert(char** msg) {
    const char* const INSERT1 = "def";
    const char* const INSERT2 = "abc";
    const int insert2_len = strlen(INSERT2);
    const char* const EXPECTED_INSERT = "abcdef";

    ot_op* expected = ot_new_op();
    ot_insert(expected, EXPECTED_INSERT);

    ot_op* op1 = ot_new_op();
    ot_insert(op1, INSERT1);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, INSERT2);
    ot_skip(op2, insert2_len);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_insert_delete(char** msg) {
    const char* const NONEMPTY_STRING = "abc";
    const int insert_len = strlen(NONEMPTY_STRING);

    ot_op* expected = ot_new_op();

    ot_op* op1 = ot_new_op();
    ot_insert(op1, NONEMPTY_STRING);

    ot_op* op2 = ot_new_op();
    ot_delete(op2, insert_len);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_delete_skip(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* expected = NULL;

    ot_op* op1 = ot_new_op();
    ot_delete(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, NONZERO_INT);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_delete_insert(char** msg) {
    const char* const NONEMPTY_STRING = "abc";
    const int NONZERO_INT = 1;

    ot_op* expected = ot_new_op();
    ot_delete(expected, NONZERO_INT);
    ot_insert(expected, NONEMPTY_STRING);

    ot_op* op1 = ot_new_op();
    ot_delete(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, NONEMPTY_STRING);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_delete_delete(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* expected = ot_new_op();
    ot_delete(expected, NONZERO_INT * 2);

    ot_op* op1 = ot_new_op();
    ot_delete(op1, NONZERO_INT);
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_delete(op2, NONZERO_INT);

    return param_compose_test(op1, op2, expected, msg);
}

bool compose_returns_op_with_same_client_id_and_parent_as_first_op(char** msg) {
    const int NONZERO_INT = 1;
    const int NONEMPTY_PARENT = 0xFF;

    ot_op* op1 = ot_new_op();
    op1->client_id = NONZERO_INT;
    memset(op1->parent, NONEMPTY_PARENT, 20);

    ot_op* op2 = ot_new_op();

    ot_op* composed = ot_compose(op1, op2);
    if (composed == NULL) {
        FAIL("Operations couldn't be composed.", msg);
    }
    ASSERT_INT_EQUAL(op1->client_id, composed->client_id,
                     "The client ID of the composed operation wasn't equal to "
                     "the client ID of the first operation.",
                     msg);
    bool parents_equal = (memcmp(op1->parent, composed->parent, 20) == 0);
    if (!parents_equal) {
        FAIL("The parent of the composed operation wasn't equal to the parent of the first operation.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    ot_free_op(composed);
    return true;
}

results compose_tests() {
    RUN_TEST(compose_skip_skip);
    RUN_TEST(compose_skip_insert);
    RUN_TEST(compose_skip_delete);
    RUN_TEST(compose_insert_skip);
    RUN_TEST(compose_insert_insert);
    RUN_TEST(compose_insert_delete);
    RUN_TEST(compose_delete_skip);
    RUN_TEST(compose_delete_insert);
    RUN_TEST(compose_delete_delete);
    RUN_TEST(compose_returns_op_with_same_client_id_and_parent_as_first_op);

    return (results) { passed, failed };
}
