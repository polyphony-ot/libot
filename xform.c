#include <assert.h>
#include "xform.h"

typedef struct delta_pair {
    size_t delta1;
    size_t delta2;
} delta_pair;

static size_t min(size_t s1, size_t s2) {
    if (s1 < s2) {
        return s1;
    } else {
        return s2;
    }
}

static delta_pair ot_xform_skip_skip(ot_comp_skip skip1, size_t offset1,
                                     ot_comp_skip skip2, size_t offset2,
                                     ot_xform_pair xform) {

    size_t skip1_count = (size_t)skip1.count - offset1;
    size_t skip2_count = (size_t)skip2.count - offset2;
    size_t min_len = min(skip1_count, skip2_count);

    ot_skip(xform.op1_prime, (int64_t)min_len);
    ot_skip(xform.op2_prime, (int64_t)min_len);

    return (delta_pair) { min_len, min_len };
}

static delta_pair ot_xform_skip_insert(ot_comp_insert ins, size_t ins_offset,
                                       ot_xform_pair xform) {

    size_t ins_len = strlen(ins.text) - ins_offset;

    ot_skip(xform.op1_prime, (int64_t)ins_len);

    char* substr = malloc(sizeof(char) * ins_len + 1);
    memcpy(substr, ins.text + ins_offset, ins_len);
    substr[ins_len] = '\0';
    ot_insert(xform.op2_prime, substr);
    free(substr);

    return (delta_pair) { 0, ins_len };
}

static delta_pair ot_xform_insert_skip(ot_comp_insert ins, size_t ins_offset,
                                       ot_xform_pair xform) {

    ot_xform_pair flip = (ot_xform_pair) { xform.op2_prime, xform.op1_prime };
    delta_pair p = ot_xform_skip_insert(ins, ins_offset, flip);
    return (delta_pair) { p.delta2, p.delta1 };
}

static delta_pair ot_xform_insert_insert(ot_comp_insert op1_insert,
                                         size_t op1_offset,
                                         ot_xform_pair xform) {

    size_t len = strlen(op1_insert.text) - op1_offset;

    ot_skip(xform.op2_prime, (int64_t)len);

    char* substr = malloc(sizeof(char) * len + 1);
    memcpy(substr, op1_insert.text + op1_offset, len);
    substr[len] = '\0';
    ot_insert(xform.op1_prime, substr);
    free(substr);

    return (delta_pair) { len, 0 };
}

ot_xform_pair ot_xform(ot_op* op1, ot_op* op2) {
    ot_op* op1_prime = ot_new_op(0, op2->parent);
    ot_op* op2_prime = ot_new_op(0, op1->parent);
    ot_xform_pair xform = (ot_xform_pair) { op1_prime, op2_prime };

    ot_comp* op1_comps = op1->comps.data;
    ot_comp* op2_comps = op2->comps.data;

    ot_iter op1_iter;
    ot_iter_init(&op1_iter, op1);

    ot_iter op2_iter;
    ot_iter_init(&op2_iter, op2);

    bool op1_next = ot_iter_next(&op1_iter);
    bool op2_next = ot_iter_next(&op2_iter);
    while (op1_next || op2_next) {
        ot_comp* op1_comp;
        if (op1_next) {
            op1_comp = op1_comps + op1_iter.pos;
        } else {
            op1_comp = NULL;
        }

        ot_comp* op2_comp;
        if (op2_next) {
            op2_comp = op2_comps + op2_iter.pos;
        } else {
            op2_comp = NULL;
        }

        if (op1_comp == NULL) {
            if (op2_comp == NULL) {
                assert(!"Both op components should never be NULL.");
            } else if (op2_comp->type == OT_INSERT) {
                // If we've reached the end of the first operation but not the
                // second, then just act like the first operation is skipping
                // whatever the second operation did.
                ot_comp_insert op2_insert = op2_comp->value.insert;
                delta_pair p =
                    ot_xform_skip_insert(op2_insert, op2_iter.offset, xform);

                op2_next = ot_iter_skip(&op2_iter, p.delta2);
            } else {
                ot_free_op(op1_prime);
                ot_free_op(op2_prime);
                return (ot_xform_pair) { NULL, NULL };
            }
        } else if (op2_comp == NULL) {
            if (op1_comp == NULL) {
                assert(!"Both op components should never be NULL.");
            } else {
                ot_free_op(op1_prime);
                ot_free_op(op2_prime);
                return (ot_xform_pair) { NULL, NULL };
            }
        } else if (op1_comp->type == OT_SKIP) {
            ot_comp_skip op1_skip = op1_comp->value.skip;

            if (op2_comp->type == OT_SKIP) {
                ot_comp_skip op2_skip = op2_comp->value.skip;

                delta_pair p =
                    ot_xform_skip_skip(op1_skip, op1_iter.offset, op2_skip,
                                       op2_iter.offset, xform);

                op1_next = ot_iter_skip(&op1_iter, p.delta1);
                op2_next = ot_iter_skip(&op2_iter, p.delta2);
            } else if (op2_comp->type == OT_INSERT) {
                ot_comp_insert op2_insert = op2_comp->value.insert;

                delta_pair p =
                    ot_xform_skip_insert(op2_insert, op2_iter.offset, xform);

                op1_next = ot_iter_skip(&op1_iter, p.delta1);
                op2_next = ot_iter_skip(&op2_iter, p.delta2);
            }
        } else if (op1_comp->type == OT_INSERT) {
            ot_comp_insert op1_insert = op1_comp->value.insert;

            if (op2_comp->type == OT_SKIP) {
                delta_pair p =
                    ot_xform_insert_skip(op1_insert, op1_iter.offset, xform);

                op1_next = ot_iter_skip(&op1_iter, p.delta1);
                op2_next = ot_iter_skip(&op2_iter, p.delta2);
            } else if (op2_comp->type == OT_INSERT) {
                delta_pair p =
                    ot_xform_insert_insert(op1_insert, op1_iter.offset, xform);

                op1_next = ot_iter_skip(&op1_iter, p.delta1);
                op2_next = ot_iter_skip(&op2_iter, p.delta2);
            }
        }
    }

    return xform;
}
