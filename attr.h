#ifndef LIBOT_ATTR_H
#define LIBOT_ATTR_H

#include <stdbool.h>
#include <stdlib.h>
#include "array.h"

typedef struct ot_attr {
    char* name;
    char* value;
} ot_attr;

array* ot_new_attrs();
void ot_init_attrs(array* attrs);

void ot_add_attr(array* attrs, const char* name, const char* value);
void ot_remove_attr(array* attrs, const char* name);
bool ot_attrs_equal(const array* attrs1, const array* attrs2);

#endif
