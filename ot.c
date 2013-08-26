#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
	uint8_t* snapshot = NULL;
	int64_t written = 0;
    
	for (int i = 0; i < op->comp_count; ++i)
	{
		if (op->comps[i].type == INSERT) {
            rope* r = op->comps[i].value.insert.text;
            size += rope_byte_count(r);
            snapshot = realloc(snapshot, size);
			written += rope_write_cstr(r, snapshot + written) - 1;
		}
	}
    
	return snapshot;
}

uint8_t* ot_serialize(ot_op* op) {
    size_t size = sizeof(3);
	uint8_t* json = malloc(2);
    memcpy(json, "[ ", 2);
	size_t written = 2;
    
	for (int i = 0; i < op->comp_count; ++i)
	{
        ot_comp_type t = op->comps[i].type;
        if (t == SKIP) {
            int64_t count = op->comps[i].value.skip.count;
            char* fmtstr = "{ \"type\": \"skip\", \"count\": %d }, ";
            size += snprintf(NULL, 0, fmtstr, count);
            json = realloc(json, size);
            written += sprintf((char*) json + written, fmtstr, count);
        } else if (t == INSERT) {
            rope* r = op->comps[i].value.insert.text;
            char* textstr = (char*) rope_create_cstr(r);
            char* fmtstr = "{ \"type\": \"insert\", \"text\": \"%s\" }, ";
            size += snprintf(NULL, 0, fmtstr, textstr);
            json = realloc(json, size);
            written += sprintf(json + written, fmtstr, textstr);
            free(textstr);
		}
	}
    
    memcpy(json + written - 2, " ]\0", 3);
	return json;
}
