#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
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

static void ot_free_fmtbound(ot_comp_fmtbound* fmtbound) {
    ot_fmt* start_data = fmtbound->start.data;
    for (int i = 0; i < fmtbound->start.len; ++i) {
        rope_free(start_data[i].name);
        rope_free(start_data[i].value);
    }
    array_free(&fmtbound->start);
    
    ot_fmt* end_data = fmtbound->end.data;
    for (int i = 0; i < fmtbound->end.len; ++i) {
        rope_free(end_data[i].name);
        rope_free(end_data[i].value);
    }
    array_free(&fmtbound->end);
}

static void ot_free_comp(ot_comp* comp) {
    switch (comp->type) {
        case OT_INSERT:
            rope_free(comp->value.insert.text);
            break;
        case OT_OPEN_ELEMENT:
            rope_free(comp->value.open_element.elem);
            break;
        case OT_FORMATTING_BOUNDARY:
            ot_free_fmtbound(&comp->value.fmtbound);
        default:
            break;
    }
}

ot_op* ot_new_op(int64_t client_id, int64_t* parent) {
	ot_op* op = (ot_op*) malloc(sizeof(ot_op));
    if (op == NULL) {
        exit(1);
    }
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
	comp->type = OT_SKIP;
    comp->value.skip.count = count;
}

void ot_insert(ot_op* op, uint8_t* text) {
	ot_comp* comp = ot_append_new_comp(op);
	comp->type = OT_INSERT;
	comp->value.insert.text = rope_new_with_utf8(text);
}

void ot_delete(ot_op* op, int64_t count) {
    ot_comp* comp = ot_append_new_comp(op);
    comp->type = OT_DELETE;
    comp->value.delete.count = count;
}

void ot_open_element(ot_op* op, uint8_t* elem) {
    ot_comp* comp = ot_append_new_comp(op);
    comp->type = OT_OPEN_ELEMENT;
    comp->value.open_element.elem = rope_new_with_utf8(elem);
}

void ot_close_element(ot_op* op) {
    ot_comp* comp = ot_append_new_comp(op);
    comp->type = OT_CLOSE_ELEMENT;
}

void ot_start_fmt(ot_op* op, uint8_t* name, uint8_t* value) {
    ot_comp* comp;
    ot_comp_fmtbound* fmtbound;
    if (op->comp_count == 0) {
        comp = ot_append_new_comp(op);
        comp->type = OT_FORMATTING_BOUNDARY;
        fmtbound = &comp->value.fmtbound;
        array_init(&comp->value.fmtbound.start, sizeof(ot_fmt));
    } else {
        comp = op->comps + op->comp_count - 1;
        if (comp->type != OT_FORMATTING_BOUNDARY) {
            comp = ot_append_new_comp(op);
            comp->type = OT_FORMATTING_BOUNDARY;
            fmtbound = &comp->value.fmtbound;
            array_init(&comp->value.fmtbound.start, sizeof(ot_fmt));
        } else {
            fmtbound = &comp->value.fmtbound;
        }
    }
    
    ot_fmt* fmt = array_append(&fmtbound->start);
    fmt->name = rope_new_with_utf8(name);
    fmt->value = rope_new_with_utf8(value);
}

uint8_t* ot_snapshot(ot_op* op) {
    size_t size = sizeof(uint8_t);
	uint8_t* snapshot = NULL;
	int64_t written = 0;
    
	for (int i = 0; i < op->comp_count; ++i)
	{
		if (op->comps[i].type == OT_INSERT) {
            rope* r = op->comps[i].value.insert.text;
            size += rope_byte_count(r);
            snapshot = realloc(snapshot, size);
			written += rope_write_cstr(r, snapshot + written) - 1;
		}
	}
    
	return snapshot;
}

static void ot_serialize_fmtstr(uint8_t** buf, size_t* bufsize, size_t* written, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    *bufsize += vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    *buf = realloc(*buf, *bufsize);
    
    va_list args2;
    va_start(args2, fmt);
    *written += vsprintf((char*) *buf + *written, fmt, args2);
    va_end(args2);
}

uint8_t* ot_serialize(ot_op* op) {
    size_t bufsize = sizeof(uint8_t) * 3;
	uint8_t* buf = malloc(bufsize);
    memcpy(buf, "[ \0", bufsize);
	size_t written = 2;
    
	for (int i = 0; i < op->comp_count; ++i)
	{
        ot_comp_type t = op->comps[i].type;
        if (t == OT_SKIP) {
            int64_t count = op->comps[i].value.skip.count;
            char* fmt = "{ \"type\": \"skip\", \"count\": %d }, ";
            ot_serialize_fmtstr(&buf, &bufsize, &written, fmt, count);
        } else if (t == OT_INSERT) {
            rope* r = op->comps[i].value.insert.text;
            uint8_t* textstr = rope_create_cstr(r);
            char* fmt = "{ \"type\": \"insert\", \"text\": \"%s\" }, ";
            ot_serialize_fmtstr(&buf, &bufsize, &written, fmt, textstr);
            free(textstr);
		} else if (t == OT_DELETE) {
            int64_t count = op->comps[i].value.delete.count;
            char* fmt = "{ \"type\": \"delete\", \"count\": %d }, ";
            ot_serialize_fmtstr(&buf, &bufsize, &written, fmt, count);
        } else if (t == OT_OPEN_ELEMENT) {
            rope* r = op->comps[i].value.open_element.elem;
            uint8_t* textstr = rope_create_cstr(r);
            char* fmt = "{ \"type\": \"openElement\", \"element\": \"%s\" }, ";
            ot_serialize_fmtstr(&buf, &bufsize, &written, fmt, textstr);
            free(textstr);
        } else if (t == OT_CLOSE_ELEMENT) {
            char* fmt = "{ \"type\": \"closeElement\" }, ";
            ot_serialize_fmtstr(&buf, &bufsize, &written, fmt);
        }
	}
    
    if (written == 2) {
        buf[1] = ']';
    } else {
        memcpy(buf + written - 2, " ]\0", 3);
    }

	return buf;
}
