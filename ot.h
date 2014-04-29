#ifndef OT_H
#define OT_H

#include <stdint.h>
#include <stdbool.h>
#include "array.h"

typedef struct ot_fmt {
	char* name;
	char* value;
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
	char* text;
} ot_comp_insert;

typedef struct ot_comp_delete
{
	int64_t count;
} ot_comp_delete;

typedef struct ot_comp_open_element
{
	char* elem;
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
bool ot_equal(const ot_op* op1, const ot_op* op2);
void ot_skip(ot_op* op, int64_t count);

// Appends an insert component to an operation. text is copied and not freed, so
// the caller must free it manually.
//
// If op already ends with an insert component, this function will append text
// to the existing insert. Otherwise, it will create a new insert component and
// append it to op.
void ot_insert(ot_op* op, const char* text);
void ot_delete(ot_op* op, int64_t count);
void ot_open_element(ot_op* op, const char* elem);
void ot_close_element(ot_op* op);
void ot_start_fmt(ot_op* op, const char* name, const char* value);
void ot_end_fmt(ot_op* op, const char* name, const char* value);
char* ot_snapshot(ot_op* op);

ot_comp_fmtbound* ot_new_fmtbound();

typedef struct ot_iter {
    const ot_op* op;    // Op to iterator over.
    size_t pos;         // Current component position.
    size_t offset;      // Offset within current component.
    bool started;       // Set true when ot_iter_next is called the first time.
} ot_iter;

// Initializes a new iterator pointing to the -1 position. This means that
// ot_iter_next must also be called before using the iterator.
//
// This was done on purpose so the iterator can be easily used in a while loop.
// I.e. - ot_iter_init(&iter); while(ot_iter_next(ot_iter* iter)) { .. }
void ot_iter_init(ot_iter* iter, const ot_op* op);
bool ot_iter_next(ot_iter* iter);
bool ot_iter_skip(ot_iter* iter, size_t count);

#endif
