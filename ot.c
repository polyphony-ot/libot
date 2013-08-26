#include <stdlib.h>
#include <string.h>
#include "ot.h"

static void ot_ensure_size(ot_op* op) {
	if (op->comp_count == 0) {
		op->comp_cap = 1;
		op->comps = malloc(sizeof(ot_comp));
	} else if (op->comp_count == op->comp_cap) {
		op->comp_cap = op->comp_cap * 2;
		op->comps = realloc(op->comps, sizeof(ot_comp) * op->comp_cap);
	}
}

static ot_comp* ot_append_new_comp(ot_op* op) {
	ot_ensure_size(op);
	ot_comp* new = &op->comps[op->comp_count];
	op->comp_count++;
	return new;
}

static ot_comp* ot_append_comp(ot_op* op, ot_comp* comp) {
	ot_ensure_size(op);
	op->comps[op->comp_count] = *comp;
	ot_comp* appended = &op->comps[op->comp_count];
	op->comp_count++;
	return appended;
}

static void ot_free_comp(ot_comp* comp) {
    switch (comp->type) {
        case INSERT:
            rope_free(comp->value.insert.text);
            break;
        default:
            break;
    }
}

ot_op* ot_new_op(int64_t client_id, int64_t* parent) {
	ot_op* op = (ot_op*) malloc(sizeof(ot_op));
	op->client_id = client_id;
	op->comp_count = 0;
	op->comp_cap = 0;
	op->comps = NULL;
    
	memcpy(op->parent, parent, sizeof(int64_t) * 8);
	return op;
}

void ot_free_op(ot_op* op) {
    for (int i = 0; i < op->comp_count; ++i)
	{
        ot_free_comp(op->comps + i);
    }
    free(op->comps);
    free(op);
}

void ot_skip(ot_op* op, int64_t count) {
    ot_comp* comp = ot_append_new_comp(op);
	comp->type = SKIP;
    comp->value.skip.count = count;
}

void ot_insert(ot_op* op, uint8_t* text) {
	ot_comp* comp = ot_append_new_comp(op);
	comp->type = INSERT;
	comp->value.insert.text = rope_new_with_utf8(text);
}

uint8_t* ot_snapshot(ot_op* op) {
    size_t size = sizeof(uint8_t);
	uint8_t* out = NULL;
	int64_t written = 0;
    
	for (int i = 0; i < op->comp_count; ++i)
	{
		if (op->comps[i].type == INSERT) {
            rope* r = op->comps[i].value.insert.text;
            size += rope_byte_count(r);
            out = realloc(out, size);
			written += rope_write_cstr(r, out + written) - 1;
		}
	}
    
	return out;
}
