#include "attr.h"

static ot_attr* find_attr(const array* attrs, const char* name) {
    ot_attr* data = (ot_attr*)attrs->data;
    for (size_t i = 0; i < attrs->len; ++i) {
        ot_attr* cur = data + i;

        if (strcmp(cur->name, name) == 0) {
            return cur;
        }
    }

    return NULL;
}

static void add_new_attr(array* attrs, const char* name, const char* value) {
    if (value == NULL) {
        return;
    }

    size_t name_size = sizeof(char) * (strlen(name) + 1);
    size_t value_size = sizeof(char) * (strlen(value) + 1);

    ot_attr* attr = array_append(attrs);
    attr->name = malloc(name_size);
    attr->value = malloc(value_size);

    memcpy(attr->name, name, name_size);
    memcpy(attr->value, value, value_size);
}

static void update_attr(ot_attr* attr, const char* value) {
    if (value == NULL) {
        free(attr->value);
        attr->value = NULL;
        return;
    }

    size_t value_size = sizeof(char) * (strlen(value) + 1);
    attr->value = realloc(attr->value, value_size);
    memcpy(attr->value, value, value_size);
}

array* ot_new_attrs() {
    array* arr = malloc(sizeof(array));
    ot_init_attrs(arr);
    return arr;
}

void ot_init_attrs(array* attrs) { array_init(attrs, sizeof(ot_attr)); }

bool ot_attrs_equal(const array* attrs1, const array* attrs2) {
    if (attrs1 == attrs2) {
        return true;
    }

    if (attrs1 == NULL && attrs2->len == 0) {
        return true;
    }

    if (attrs2 == NULL && attrs1->len == 0) {
        return true;
    }

    if (attrs1 == NULL || attrs2 == NULL) {
        return false;
    }

    size_t len1 = attrs1->len;
    size_t len2 = attrs2->len;
    if (len1 != len2) {
        return false;
    }

    ot_attr* data = (ot_attr*)attrs1->data;
    for (size_t i = 0; i < len1; ++i) {
        ot_attr* attr1 = data + i;
        ot_attr* attr2 = find_attr(attrs2, attr1->name);
        if (attr2 == NULL) {
            return false;
        }

        int value_cmp = strcmp(attr1->value, attr2->value);
        if (value_cmp != 0) {
            return false;
        }
    }

    return true;
}

void ot_add_attr(array* attrs, const char* name, const char* value) {
    ot_attr* existing = find_attr(attrs, name);
    if (existing == NULL) {
        add_new_attr(attrs, name, value);
    } else {
        update_attr(existing, value);
    }
}

void ot_remove_attr(array* attrs, const char* name) {
    ot_add_attr(attrs, name, NULL);
}
