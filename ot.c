#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ot.h"
#include "hex.h"
#include "utf8.h"

static bool can_merge_comp(array* comps, ot_comp_type t, const array* attrs) {
    size_t len = comps->len;
    if (len == 0) {
        return false;
    }

    ot_comp* data = comps->data;
    ot_comp* last_comp = data + (len - 1);
    if (last_comp->type != t) {
        return false;
    }

    array* last_attrs = NULL;
    switch (t) {
    case OT_SKIP:
        last_attrs = &(last_comp->value.skip.attrs);
        break;
    case OT_INSERT:
        last_attrs = &(last_comp->value.insert.attrs);
        break;
    default:
        break;
    }

    return ot_attrs_equal(last_attrs, attrs);
}

ot_op* ot_new_op() {
    ot_op* op = (ot_op*)malloc(sizeof(ot_op));
    op->client_id = 0;
    array_init(&op->comps, sizeof(ot_comp));
    memset(op->parent, 0, 20);
    memset(op->hash, 0, 20);

    return op;
}

void ot_free_op(ot_op* op) {
    ot_comp* comps = op->comps.data;
    for (size_t i = 0; i < op->comps.len; ++i) {
        ot_free_comp(comps + i);
    }
    array_free(&op->comps);
    free(op);
}

void ot_free_comp(ot_comp* comp) {
    switch (comp->type) {
    case OT_SKIP:
        array_free(&(comp->value.skip.attrs));
        break;
    case OT_INSERT:
        free(comp->value.insert.text);
        array_free(&(comp->value.insert.attrs));
        break;
    default:
        break;
    }
}

// TODO: Make this more efficient by copying memory instead of recreating the op
//       using the various OT functions.
ot_op* ot_dup_op(const ot_op* op) {
    ot_op* dup = malloc(sizeof(ot_op));
    memcpy(dup, op, sizeof(ot_op));
    array_init(&dup->comps, sizeof(ot_comp));

    ot_comp* comps = op->comps.data;
    for (size_t i = 0; i < op->comps.len; ++i) {
        ot_comp* comp = comps + i;
        switch (comp->type) {
        case OT_SKIP:
            ot_skip(dup, comp->value.skip.count);
            break;
        case OT_INSERT:
            ot_insert(dup, comp->value.insert.text);
            break;
        case OT_DELETE:
            ot_delete(dup, comp->value.delete.count);
            break;
        }
    }

    return dup;
}

// TODO: Implement equality for formatting boundaries.
bool ot_equal(const ot_op* op1, const ot_op* op2) {
    if (op1 == NULL || op2 == NULL) {
        return op1 == op2;
    }

    if (op1->client_id != op2->client_id) {
        return false;
    }

    if (memcmp(op1->parent, op2->parent, sizeof(op1->parent)) != 0) {
        return false;
    }

    if (op1->comps.len != op2->comps.len) {
        return false;
    }

    for (size_t i = 0; i < op1->comps.len; ++i) {
        ot_comp comp1 = ((ot_comp*)op1->comps.data)[i];
        ot_comp comp2 = ((ot_comp*)op2->comps.data)[i];

        if (comp1.type != comp2.type) {
            return false;
        }

        switch (comp1.type) {
        case OT_SKIP:
            if (comp1.value.skip.count != comp2.value.skip.count) {
                return false;
            }

            if (!ot_attrs_equal(&comp1.value.skip.attrs,
                                &comp2.value.skip.attrs)) {
                return false;
            }

            break;
        case OT_INSERT: {
            char* text1 = comp1.value.insert.text;
            char* text2 = comp2.value.insert.text;
            if (strcmp(text1, text2) != 0) {
                return false;
            }

            if (!ot_attrs_equal(&comp1.value.insert.attrs,
                                &comp2.value.insert.attrs)) {
                return false;
            }

            break;
        }
        case OT_DELETE:
            if (comp1.value.delete.count != comp2.value.delete.count) {
                return false;
            }

            break;
        default:
            return false;
        }
    }

    return true;
}

void ot_skip(ot_op* op, uint32_t count) { ot_skip_attr(op, count, NULL); }

void ot_skip_attr(ot_op* op, uint32_t count, const array* attrs) {
    if (count == 0) {
        return;
    }

    if (can_merge_comp(&(op->comps), OT_SKIP, attrs)) {
        ot_comp* comps = op->comps.data;
        ot_comp* last = comps + (op->comps.len - 1);
        last->value.skip.count += count;
        return;
    }

    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_SKIP;
    comp->value.skip.count = count;

    if (attrs == NULL) {
        ot_init_attrs(&(comp->value.skip.attrs));
    } else {
        array_copy(&(comp->value.skip.attrs), attrs);
    }
}

void ot_insert(ot_op* op, const char* text) { ot_insert_attr(op, text, NULL); }

void ot_insert_attr(ot_op* op, const char* text, const array* attrs) {
    if (text == NULL) {
        return;
    }

    ot_comp* comps = op->comps.data;
    if (can_merge_comp(&(op->comps), OT_INSERT, attrs)) {
        ot_comp* last = comps + (op->comps.len - 1);
        size_t len1 = strlen(last->value.insert.text);
        size_t len2 = strlen(text);
        last->value.insert.text =
            realloc(last->value.insert.text, len1 + len2 + 1);
        strcat(last->value.insert.text, text);
        return;
    }

    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_INSERT;
    size_t size = sizeof(char) * (strlen(text) + 1);
    comp->value.insert.text = malloc(size);
    memcpy(comp->value.insert.text, text, size);

    if (attrs == NULL) {
        ot_init_attrs(&(comp->value.insert.attrs));
    } else {
        array_copy(&(comp->value.insert.attrs), attrs);
    }
}

void ot_delete(ot_op* op, uint32_t count) {
    if (count == 0) {
        return;
    }

    ot_comp* comps = op->comps.data;
    ot_comp* last = comps + (op->comps.len - 1);
    if (op->comps.len > 0 && last->type == OT_DELETE) {
        last->value.delete.count += count;
    } else {
        ot_comp* comp = array_append(&op->comps);
        comp->type = OT_DELETE;
        comp->value.delete.count = count;
    }
}

char* ot_snapshot(ot_op* op) {
    size_t size = sizeof(char);
    size_t written = 0;
    char* snapshot = NULL;
    ot_comp* comps = op->comps.data;

    for (size_t i = 0; i < op->comps.len; ++i) {
        if (comps[i].type == OT_INSERT) {
            size_t oldsize = size;
            char* t = comps[i].value.insert.text;
            size_t comp_len = strlen(t);
            size += sizeof(char) * comp_len;
            snapshot = realloc(snapshot, size);
            memcpy(snapshot + oldsize - 1, t, comp_len);
            written += comp_len;
        }
    }

    if (snapshot != NULL) {
        snapshot[written] = 0;
    }

    return snapshot;
}

uint32_t ot_size(const ot_op* op) {
    uint32_t size = 0;
    ot_comp* comps = op->comps.data;
    for (size_t i = 0; i < op->comps.len; ++i) {
        ot_comp* comp = comps + i;
        switch (comp->type) {
        case OT_INSERT:
            size += strlen(comp->value.insert.text);
            break;
        case OT_DELETE:
            size -= comp->value.delete.count;
            break;
        default:
            break;
        }
    }

    return size;
}

uint32_t ot_comp_size(const ot_comp* comp) {
    switch (comp->type) {
    case OT_SKIP:
        return comp->value.skip.count;
    case OT_INSERT:
        return utf8_length(comp->value.insert.text);
    case OT_DELETE:
        return comp->value.delete.count;
    }

    return -1;
}

void ot_iter_init(ot_iter* iter, const ot_op* op) {
    iter->op = op;
    iter->started = false;
}

static bool ot_iter_adv(ot_iter* iter, size_t n, size_t max) {
    if (iter->offset + n < max) {
        iter->offset += n;
        return true;
    }
    if (iter->pos < iter->op->comps.len - 1) {
        iter->pos++;
        iter->offset = 0;
        return true;
    }

    return false;
}

bool ot_iter_next(ot_iter* iter) { return ot_iter_skip(iter, 1); }

// This can be made more efficient instead of simply calling ot_iter_next()
// "count" times.
bool ot_iter_skip(ot_iter* iter, size_t count) {
    if (iter->op->comps.len == 0) {
        return false;
    }

    if (!iter->started) {
        iter->pos = 0;
        iter->offset = 0;
        iter->started = true;
        return true;
    }

    for (size_t i = 0; i < count;) {
        ot_comp* comp = ((ot_comp*)iter->op->comps.data) + iter->pos;
        size_t max = ot_comp_size(comp);

        if (!ot_iter_adv(iter, count, max)) {
            return false;
        }

        if (iter->offset == 0) {
            i += max;
        } else {
            i += count;
        }
    }

    return true;
}
