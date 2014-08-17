#include "../../ot.h"
#include "unit.h"

bool start_fmt_appends_correct_comp_type(char** msg) {
    ot_comp_type expected_type = OT_FORMATTING_BOUNDARY;
    char* expected_name = "any name";
    char* expected_value = "any value";

    ot_op* op = ot_new_op();
    ot_start_fmt(op, expected_name, expected_value);

    ot_comp* comps = op->comps.data;
    ot_comp_type actual_type = comps[0].type;

    ASSERT_INT_EQUAL(expected_type, actual_type, "Starting a format did not "
                                                 "append a component of type "
                                                 "OT_FORMATTING_BOUNDARY.",
                     msg);

    ot_free_op(op);
    return true;
}

results ot_tests() {
    RUN_TEST(start_fmt_appends_correct_comp_type);

    return (results) { passed, failed };
}
