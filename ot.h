#ifndef OT_H
#define OT_H

#include <stdint.h>
#include "librope/rope.h"
#include "array.h"

typedef struct ot_fmt {
	rope* name;
	rope* value;
} ot_fmt;

typedef enum {
	OT_SKIP = 0,
	OT_INSERT = 1,
	OT_DELETE = 2,
	OT_OPEN_ELEMENT = 3,
	OT_CLOSE_ELEMENT = 4,
	OT_FORMATTING_BOUNDARY = 5
} ot_comp_type;

typedef struct ot_comp_skip {
	int64_t count;
} ot_comp_skip;

typedef struct ot_comp_insert
{
	rope* text;
} ot_comp_insert;

typedef struct ot_comp_delete
{
	int64_t count;
} ot_comp_delete;

typedef struct ot_comp_open_element
{
	rope* elem;
} ot_comp_open_element;

typedef struct ot_comp_fmtbound
{
    array start;
    array end;
} ot_comp_fmtbound;

typedef struct ot_comp {
	ot_comp_type type;
	union {
		ot_comp_skip skip;
		ot_comp_insert insert;
		ot_comp_delete delete;
		ot_comp_open_element open_element;
		ot_comp_fmtbound fmtbound;
	} value;
} ot_comp;

typedef struct ot_op {
	int64_t client_id;
    char parent[64];
	array comps;
} ot_op;

ot_op* ot_new_op(int64_t client_id, char parent[64]);
void ot_free_op(ot_op* op);
void ot_skip(ot_op* op, int64_t count);
void ot_insert(ot_op* op, uint8_t* text);
void ot_delete(ot_op* op, int64_t count);
void ot_open_element(ot_op* op, uint8_t* elem);
void ot_close_element(ot_op* op);
void ot_start_fmt(ot_op* op, uint8_t* name, uint8_t* value);
void ot_end_fmt(ot_op* op, uint8_t* name, uint8_t* value);
ot_op* ot_compose(ot_op* op1, ot_op* op2);
char* ot_snapshot(ot_op* op);

ot_comp_fmtbound* ot_new_fmtbound();

typedef struct ot_iter {
    ot_op* op;      // Op to iterator over.
    size_t pos;     // Current component position.
    size_t offset;  // Offset within current component.
} ot_iter;

void ot_iter_init(ot_iter* iter);
void ot_iter_next(ot_iter* iter);

#endif
