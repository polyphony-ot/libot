#include "../../attr.h"
#include "unit.h"

static bool null_attr_sets_are_equal(char** msg) {
    bool equal = ot_attrs_equal(NULL, NULL);

    if (!equal) {
        FAIL("Null attribute sets weren't equal.", msg);
    }

    return true;
}

static bool attr_set_is_equal_to_itself(char** msg) {
    array* attrs = ot_new_attrs();
    bool equal = ot_attrs_equal(attrs, attrs);

    if (!equal) {
        FAIL("Attribute set wasn't equal to itself.", msg);
    }

    array_free(attrs);
    return true;
}

static bool null_attr_set_equals_empty_attr_set(char** msg) {
    array* attrs = ot_new_attrs();

    bool equal = ot_attrs_equal(attrs, NULL);
    if (!equal) {
        FAIL("Empty attribute set wasn't equal to NULL.", msg);
    }

    equal = ot_attrs_equal(NULL, attrs);
    if (!equal) {
        FAIL("NULL wasn't equal to empty attribute set.", msg);
    }

    array_free(attrs);
    return true;
}

static bool attr_set_is_not_equal_to_null(char** msg) {
    array* attrs = ot_new_attrs();
    ot_add_attr(attrs, "name", "value");

    bool equal = ot_attrs_equal(attrs, NULL);
    if (equal) {
        FAIL("Attribute set was equal to NULL.", msg);
    }

    equal = ot_attrs_equal(NULL, attrs);
    if (equal) {
        FAIL("NULL was equal to attribute set.", msg);
    }

    array_free(attrs);
    return true;
}

static bool attr_sets_with_one_attr_are_equal(char** msg) {
    array* attrs1 = ot_new_attrs();
    ot_add_attr(attrs1, "name", "value");

    array* attrs2 = ot_new_attrs();
    ot_add_attr(attrs2, "name", "value");

    bool equal = ot_attrs_equal(attrs1, attrs2);
    if (!equal) {
        FAIL("Attribute sets weren't equal.", msg);
    }

    array_free(attrs1);
    array_free(attrs2);
    return true;
}

static bool attr_sets_with_attrs_in_different_order_are_equal(char** msg) {
    array* attrs1 = ot_new_attrs();
    ot_add_attr(attrs1, "name1", "value1");
    ot_add_attr(attrs1, "name2", "value2");

    array* attrs2 = ot_new_attrs();
    ot_add_attr(attrs2, "name2", "value2");
    ot_add_attr(attrs2, "name1", "value1");

    bool equal = ot_attrs_equal(attrs1, attrs2);
    if (!equal) {
        FAIL("Attribute sets weren't equal.", msg);
    }

    array_free(attrs1);
    array_free(attrs2);
    return true;
}

static bool adding_existing_attr_updates_value(char** msg) {
    const char* const ORIGINAL_VALUE = "original value";
    const char* const NEW_VALUE = "new value";

    array* attrs = ot_new_attrs();
    ot_add_attr(attrs, "name1", ORIGINAL_VALUE);
    ot_add_attr(attrs, "name1", NEW_VALUE);

    ASSERT_INT_EQUAL(1, attrs->len,
        "Attribute set didn't have the correct length.", msg);

    ot_attr* attr = (ot_attr*)attrs->data;
    ASSERT_STR_EQUAL(NEW_VALUE, attr->value,
        "Existing attribute wasn't updated with the new value.", msg);

    array_free(attrs);
    return true;
}

results attr_tests() {
    RUN_TEST(null_attr_sets_are_equal);
    RUN_TEST(attr_set_is_equal_to_itself);
    RUN_TEST(null_attr_set_equals_empty_attr_set);
    RUN_TEST(attr_set_is_not_equal_to_null);
    RUN_TEST(attr_sets_with_one_attr_are_equal);
    RUN_TEST(attr_sets_with_attrs_in_different_order_are_equal);
    RUN_TEST(adding_existing_attr_updates_value);

    return (results){passed, failed};
}
