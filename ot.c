#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ot.h"
#include "hex.h"

static void ot_free_fmtbound(ot_comp_fmtbound* fmtbound) {
    ot_fmt* start_data = fmtbound->start.data;
    for (size_t i = 0; i < fmtbound->start.len; ++i) {
        free(start_data[i].name);
        free(start_data[i].value);
    }
    array_free(&fmtbound->start);
    
    ot_fmt* end_data = fmtbound->end.data;
    for (size_t i = 0; i < fmtbound->end.len; ++i) {
        free(end_data[i].name);
        free(end_data[i].value);
    }
    array_free(&fmtbound->end);
}

static void ot_free_comp(ot_comp* comp) {
    switch (comp->type) {
        case OT_INSERT:
            free(comp->value.insert.text);
            break;
        case OT_OPEN_ELEMENT:
            free(comp->value.open_element.elem);
            break;
        case OT_FORMATTING_BOUNDARY:
            ot_free_fmtbound(&comp->value.fmtbound);
        default:
            break;
    }
}

ot_op* ot_new_op(int64_t client_id, char parent[64]) {
	ot_op* op = (ot_op*) malloc(sizeof(ot_op));
	op->client_id = client_id;
    array_init(&op->comps, sizeof(ot_comp));
    memcpy(op->parent, parent, 64);
    
	return op;
}

void ot_free_op(ot_op* op) {
    ot_comp* comps = op->comps.data;
    for (size_t i = 0; i < op->comps.len; ++i)
	{
        ot_free_comp(comps + i);
    }
    array_free(&op->comps);
    free(op);
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
                
                break;
            case OT_INSERT:
            {
                char* text1 = comp1.value.insert.text;
                char* text2 = comp2.value.insert.text;
                if (strcmp(text1, text2) != 0) {
                    return false;
                }
                
                break;
            }
            case OT_DELETE:
                if (comp1.value.delete.count != comp2.value.delete.count) {
                    return false;
                }
                
                break;
            case OT_OPEN_ELEMENT:
            {
                char* elem1 = comp1.value.open_element.elem;
                char* elem2 = comp2.value.open_element.elem;
                if (strcmp(elem1, elem2) != 0) {
                    return false;
                }
                
                break;
            }
            case OT_CLOSE_ELEMENT:
                break;
            case OT_FORMATTING_BOUNDARY:
                break;
            default:
                return false;
        }
    }
    
    return true;
}

void ot_skip(ot_op* op, int64_t count) {
    ot_comp* comps = op->comps.data;
    ot_comp* last = comps + (op->comps.len - 1);
    if (op->comps.len > 0 && last->type == OT_SKIP) {
        last->value.skip.count += count;
    } else {
        ot_comp* comp = array_append(&op->comps);
        comp->type = OT_SKIP;
        comp->value.skip.count = count;
    }
}

void ot_insert(ot_op* op, const char* text) {
    ot_comp* comps = op->comps.data;
    ot_comp* last = comps + (op->comps.len - 1);
    if (op->comps.len > 0 && last->type == OT_INSERT) {
        size_t len1 = strlen(last->value.insert.text);
        size_t len2 = strlen(text);
        last->value.insert.text = realloc(last->value.insert.text,
                                          len1 + len2 + 1);
        strcat(last->value.insert.text, text);
    } else {
        ot_comp* comp = array_append(&op->comps);
        comp->type = OT_INSERT;
        size_t size = sizeof(char) * (strlen(text) + 1);
        comp->value.insert.text = malloc(size);
        memcpy(comp->value.insert.text, text, size);
    }
}

void ot_delete(ot_op* op, int64_t count) {
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

void ot_open_element(ot_op* op, const char* elem) {
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_OPEN_ELEMENT;
    size_t size = sizeof(char) * (strlen(elem) + 1);
    comp->value.open_element.elem = malloc(size);
    memcpy(comp->value.open_element.elem, elem, size);
}

void ot_close_element(ot_op* op) {
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_CLOSE_ELEMENT;
}

void ot_start_fmt(ot_op* op, const char* name, const char* value) {
    ot_comp* cur_comp;
    ot_comp* comps = op->comps.data;
    ot_comp_fmtbound* fmtbound;
    if (op->comps.len == 0) {
        cur_comp = array_append(&op->comps);
        cur_comp->type = OT_FORMATTING_BOUNDARY;
        fmtbound = &cur_comp->value.fmtbound;
        array_init(&cur_comp->value.fmtbound.start, sizeof(ot_fmt));
        array_init(&cur_comp->value.fmtbound.end, sizeof(ot_fmt));
    } else {
        cur_comp = comps + op->comps.len - 1;
        if (cur_comp->type != OT_FORMATTING_BOUNDARY) {
            cur_comp = array_append(&op->comps);
            cur_comp->type = OT_FORMATTING_BOUNDARY;
            fmtbound = &cur_comp->value.fmtbound;
            array_init(&cur_comp->value.fmtbound.start, sizeof(ot_fmt));
            array_init(&cur_comp->value.fmtbound.end, sizeof(ot_fmt));
        } else {
            fmtbound = &cur_comp->value.fmtbound;
        }
    }
    
    ot_fmt* fmt = array_append(&fmtbound->start);
    
    size_t name_size = sizeof(char) * (strlen(name) + 1);
    fmt->name = malloc(name_size);
    memcpy(fmt->name, name, name_size);

    size_t value_size = sizeof(char) * (strlen(value) + 1);
    fmt->value = malloc(value_size);
    memcpy(fmt->value, value, value_size);
}

void ot_end_fmt(ot_op* op, const char* name, const char* value) {
    ot_comp* cur_comp;
    ot_comp* comps = op->comps.data;
    ot_comp_fmtbound* fmtbound;
    if (op->comps.len == 0) {
        cur_comp = array_append(&op->comps);
        cur_comp->type = OT_FORMATTING_BOUNDARY;
        fmtbound = &cur_comp->value.fmtbound;
        array_init(&cur_comp->value.fmtbound.start, sizeof(ot_fmt));
        array_init(&cur_comp->value.fmtbound.end, sizeof(ot_fmt));
    } else {
        cur_comp = comps + op->comps.len - 1;
        if (cur_comp->type != OT_FORMATTING_BOUNDARY) {
            cur_comp = array_append(&op->comps);
            cur_comp->type = OT_FORMATTING_BOUNDARY;
            fmtbound = &cur_comp->value.fmtbound;
            array_init(&cur_comp->value.fmtbound.start, sizeof(ot_fmt));
            array_init(&cur_comp->value.fmtbound.end, sizeof(ot_fmt));
        } else {
            fmtbound = &cur_comp->value.fmtbound;
        }
    }
    
    ot_fmt* fmt = array_append(&fmtbound->end);
    
    size_t name_size = sizeof(char) * (strlen(name) + 1);
    fmt->name = malloc(name_size);
    memcpy(fmt->name, name, name_size);
    
    size_t value_size = sizeof(char) * (strlen(value) + 1);
    fmt->value = malloc(value_size);
    memcpy(fmt->value, value, value_size);
}

char* ot_snapshot(ot_op* op) {
    size_t size = sizeof(char);
    size_t written = 0;
	char* snapshot = NULL;
    ot_comp* comps = op->comps.data;
    
	for (size_t i = 0; i < op->comps.len; ++i)
	{
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

void ot_iter_init(ot_iter* iter, const ot_op* op) {
    iter->op = op;
    iter->started = false;
}

static bool ot_iter_adv(ot_iter* iter, size_t max) {
    if (iter->offset < max) {
        iter->offset++;
        return true;
    }
    if (iter->pos < iter->op->comps.len - 1) {
        iter->pos++;
        iter->offset = 0;
        return true;
    }
    
    return false;
}

bool ot_iter_next(ot_iter* iter) {
    if (iter->op->comps.len == 0) {
        return false;
    }
    
    if (!iter->started) {
        iter->pos = 0;
        iter->offset = 0;
        iter->started = true;
        return true;
    }
    
    ot_comp* comp = ((ot_comp*) iter->op->comps.data) + iter->pos;
    if (comp->type == OT_SKIP) {
        ot_comp_skip skip = comp->value.skip;
        return ot_iter_adv(iter, (size_t) skip.count - 1);
    } else if (comp->type == OT_INSERT) {
        ot_comp_insert insert = comp->value.insert;
        return ot_iter_adv(iter, strlen(insert.text) - 1);
    } else if (comp->type == OT_DELETE) {
        ot_comp_delete delete = comp->value.delete;
        return ot_iter_adv(iter, (size_t) delete.count - 1);
    } else if (comp->type == OT_OPEN_ELEMENT) {
        return ot_iter_adv(iter, 0);
    } else if (comp->type == OT_CLOSE_ELEMENT) {
        return ot_iter_adv(iter, 0);
    } else if (comp->type == OT_FORMATTING_BOUNDARY) {
        return ot_iter_adv(iter, 0);
    }
    
    assert(!"Iterator doesn't know how to handle this component type.");
}

// This can be made more efficient instead of simply calling ot_iter_next()
// "count" times.
bool ot_iter_skip(ot_iter* iter, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!ot_iter_next(iter)) {
            return false;
        }
    }
    
    return true;
}
