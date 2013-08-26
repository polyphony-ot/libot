#ifndef OT_H
#define OT_H

#include <stdint.h>
#include "librope/rope.h"

/*

<op> ::= <int> <parent> <comp> <op>
<comp> ::= <num> | <type>
<num> ::= <int> <type>
<type> ::= <ins> | "d" | "r"
<ins> ::= "i" <str>

int 0
char[64] 0xa840c0d1e
int64 length
char i
length Hello, world!

*/

typedef struct ot_fmt {
	uint8_t* name;
	uint8_t* value;
} ot_fmt;

typedef enum {
	SKIP = 0,
	INSERT = 1,
	DELETE = 2,
	OPEN_ELEMENT = 3,
	CLOSE_ELEMENT = 4,
	FORMATTING_BOUNDARY = 5
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
	uint8_t* elem;
} ot_comp_open_element;

typedef struct ot_comp_fmtbound
{
	ot_fmt* start;
	ot_fmt* end;
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
	int64_t parent[8];
	int64_t comp_count;
	int64_t comp_cap;
	ot_comp* comps;	
} ot_op;

ot_op* ot_new_op(int64_t client_id, int64_t* parent);
void ot_free_op(ot_op* op);
void ot_skip(ot_op* op, int64_t count);
void ot_insert(ot_op* op, uint8_t* text);
void ot_delete(ot_op* op, int64_t count);
void ot_open_element(ot_op* op, uint8_t* elem);
void ot_close_element(ot_op* op);
void ot_fmtbound(ot_op* op, ot_comp_fmtbound fmtbound);
uint8_t* snapshot(ot_op op);

ot_comp_fmtbound* ot_new_fmtbound();
void ot_start_fmt(ot_comp_fmtbound* fmtbound, uint8_t* name, uint8_t* value);
void ot_end_fmt(ot_comp_fmtbound* fmtbound, uint8_t* name, uint8_t* value);

void ot_free_comp(ot_comp* comp);

uint8_t* ot_snapshot(ot_op* op);

#endif
