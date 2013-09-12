#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ot.h"
#include "hex.h"
#include "jsmn.h"

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

static int tokcmp(jsmntok_t* tok, uint8_t* json, void* cmp) {
    size_t length = tok->end - tok->start;
    return memcmp(cmp, json + tok->start, length);
}

static int try_parse_client_id(uint8_t* json, jsmntok_t* key, jsmntok_t* val, ot_op* op) {
    int cmp = tokcmp(key, json, "clientId");
    if (cmp != 0) {
        return 0;
    }
    
    op->client_id = atoi((char*) json + val->start);
    return 1;
}

static int try_parse_parent(uint8_t* json, jsmntok_t* key, jsmntok_t* val, ot_op* op) {
    int cmp = tokcmp(key, json, "parent");
    if (cmp != 0) {
        return 0;
    }
    
    uint8_t* start = (uint8_t*) json + val->start;
    hextoa((uint8_t*) &op->parent, start, 128);
    
    return 1;
}

static int try_parse_comps(uint8_t* json, jsmntok_t* key, jsmntok_t* valstart, ot_op* op) {
    int cmp = tokcmp(key, json, "components");
    if (cmp != 0) {
        return 0;
    }
    
    // "comps"
    // array (size)
    // object
    // key 1
    // field 1
    // object
    // key 1
    // field 1
    
    for (int i = 0; i < valstart->size; ++i) {
        
    }
    
    return 1;
}

static void ot_parse_op(ot_op* op, uint8_t* json, jsmntok_t* tokens) {
    int num_tokens = tokens[0].size;
    
    for (int i = 1; i < num_tokens; i += 2) {
        if (try_parse_client_id(json, &tokens[i], &tokens[i + 1], op)) {
            
        } else if (try_parse_parent(json, &tokens[i], &tokens[i + 1], op)) {
            
        }
    }
}

ot_op* ot_new_op(int64_t client_id, int64_t* parent) {
	ot_op* op = (ot_op*) malloc(sizeof(ot_op));
	op->client_id = client_id;
    array_init(&op->comps, sizeof(ot_comp));
    memcpy(op->parent, parent, sizeof(int64_t) * 8);
    
	return op;
}

ot_op* ot_new_json(uint8_t* json) {
    int err;
    jsmn_parser parser;
    jsmntok_t tokens[128];
    
    jsmn_init(&parser);
    err = jsmn_parse(&parser, (char*) json, tokens, 128);
    ot_op* op = malloc(sizeof(ot_op));
    ot_parse_op(op, json, tokens);
    array_init(&op->comps, sizeof(ot_comp));
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

void ot_insert(ot_op* op, uint8_t* text) {
	ot_comp* comp = array_append(&op->comps);
	comp->type = OT_INSERT;
	comp->value.insert.text = rope_new_with_utf8(text);
}

void ot_delete(ot_op* op, int64_t count) {
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_DELETE;
    comp->value.delete.count = count;
}

void ot_open_element(ot_op* op, uint8_t* elem) {
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_OPEN_ELEMENT;
    comp->value.open_element.elem = rope_new_with_utf8(elem);
}

void ot_close_element(ot_op* op) {
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_CLOSE_ELEMENT;
}

void ot_start_fmt(ot_op* op, uint8_t* name, uint8_t* value) {
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
    fmt->name = rope_new_with_utf8(name);
    fmt->value = rope_new_with_utf8(value);
}

void ot_end_fmt(ot_op* op, uint8_t* name, uint8_t* value) {
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
    fmt->name = rope_new_with_utf8(name);
    fmt->value = rope_new_with_utf8(value);
}

uint8_t* ot_snapshot(ot_op* op) {
    size_t size = sizeof(uint8_t);
	uint8_t* snapshot = NULL;
	int64_t written = 0;
    ot_comp* comps = op->comps.data;
    
	for (int i = 0; i < op->comps.len; ++i)
	{
		if (comps[i].type == OT_INSERT) {
            rope* r = comps[i].value.insert.text;
            size += rope_byte_count(r);
            snapshot = realloc(snapshot, size);
			written += rope_write_cstr(r, snapshot + written) - 1;
		}
	}
    
	return snapshot;
}
