#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "ot.h"
#include "hex.h"

static void ot_free_fmtbound(ot_comp_fmtbound* fmtbound) {
    ot_fmt* start_data = fmtbound->start.data;
    for (int i = 0; i < fmtbound->start.len; ++i) {
        free(start_data[i].name);
        free(start_data[i].value);
    }
    array_free(&fmtbound->start);
    
    ot_fmt* end_data = fmtbound->end.data;
    for (int i = 0; i < fmtbound->end.len; ++i) {
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

static void ot_copy_comp(ot_comp* dest, ot_comp* src) {
    switch (src->type) {
        case OT_SKIP:
            dest->type = OT_SKIP;
            dest->value.skip.count = src->value.skip.count;
            break;
        case OT_INSERT:
            dest->type = OT_INSERT;
            strcpy(dest->value.insert.text, src->value.insert.text);
            break;
        case OT_DELETE:
            dest->type = OT_DELETE;
            dest->value.delete.count = src->value.delete.count;
            break;
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
    for (int i = 0; i < op->comps.len; ++i)
	{
        ot_free_comp(comps + i);
    }
    array_free(&op->comps);
    free(op);
}

void ot_skip(ot_op* op, int64_t count) {
    ot_comp* comp = array_append(&op->comps);
	comp->type = OT_SKIP;
    comp->value.skip.count = count;
}

void ot_insert(ot_op* op, const char* text) {
	ot_comp* comp = array_append(&op->comps);
	comp->type = OT_INSERT;
    size_t size = sizeof(char) * (strlen(text) + 1);
    comp->value.insert.text = malloc(size);
    memcpy(comp->value.insert.text, text, size);
}

void ot_delete(ot_op* op, int64_t count) {
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_DELETE;
    comp->value.delete.count = count;
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

ot_op* ot_compose(ot_op* op1, ot_op* op2) {
    char parent[64];
    memcpy(parent, op1->parent, 64);
    ot_op* composed = ot_new_op(op1->client_id, parent);
    
    size_t op1_cur = 0;
    ot_comp* op1_comps = op1->comps.data;
    size_t op2_cur = 0;
    ot_comp* op2_comps = op2->comps.data;
    while (op1_cur < op1->comps.len || op2_cur < op2->comps.len) {
        ot_comp* op1_comp;
        if (op1_cur < op1->comps.len) {
            op1_comp = op1_comps + op1_cur;
        } else {
            op1_comp = NULL;
        }
        
        ot_comp* op2_comp;
        if (op2_cur < op2->comps.len) {
            op2_comp = op2_comps + op2_cur;
        } else {
            op2_comp = NULL;
        }
        
        if (op2_comp->type == OT_SKIP) {
            if (op1_comp == NULL) {
                // ERROR
            }
            ot_comp* res = array_append(&composed->comps);
            ot_copy_comp(res, op1_comp);
            op1_cur++;
            op2_cur++;
        } else if (op2_comp->type == OT_INSERT) {
            ot_comp* res = array_append(&composed->comps);
            ot_copy_comp(res, op2_comp);
            op2_cur++;
        } else if (op2_comp->type == OT_DELETE) {
            op1_cur++;
            op2_cur++;
        }
    }
    
    return composed;
}

char* ot_snapshot(ot_op* op) {
    size_t size = sizeof(char);
	char* snapshot = NULL;
	int64_t written = 0;
    ot_comp* comps = op->comps.data;
    
	for (int i = 0; i < op->comps.len; ++i)
	{
		if (comps[i].type == OT_INSERT) {
            char* t = comps[i].value.insert.text;
            size += sizeof(char) * strlen(t);
            snapshot = realloc(snapshot, size);
            memcpy(snapshot, t, size);
            written += strlen(t);
            
		}
	}
    
	return snapshot;
}

void ot_iter_init(ot_iter* iter, ot_op* op) {
    iter->op = op;
    iter->pos = -1;
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
    
    if (iter->pos == -1) {
        iter->pos = 0;
        iter->offset = 0;
        return true;
    }
    
    ot_comp* comp = ((ot_comp*) iter->op->comps.data) + iter->pos;
    if (comp->type == OT_SKIP) {
        ot_comp_skip skip = comp->value.skip;
        return ot_iter_adv(iter, skip.count - 1);
    } else if (comp->type == OT_INSERT) {
        ot_comp_insert insert = comp->value.insert;
        return ot_iter_adv(iter, strlen(insert.text) - 1);
    } else if (comp->type == OT_DELETE) {
        ot_comp_delete delete = comp->value.delete;
        return ot_iter_adv(iter, delete.count - 1);
    } else if (comp->type == OT_OPEN_ELEMENT) {
        return ot_iter_adv(iter, 0);
    } else if (comp->type == OT_CLOSE_ELEMENT) {
        return ot_iter_adv(iter, 0);
    } else if (comp->type == OT_FORMATTING_BOUNDARY) {
        return ot_iter_adv(iter, 0);
    }
    
    assert(!"Iterator doesn't know how to handle this component type.");
}
