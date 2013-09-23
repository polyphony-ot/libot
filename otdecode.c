#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "otdecode.h"
#include "hex.h"
#include "jsmn.h"
#include "cJSON.h"

typedef enum {
	NOT_MATCH = 0,
	FORMAT_ERR = -1,
    MISSING_FIELD = -2
} parse_result;

// Compares a JSON token to a block of memory. Returns true if the two blocks
// are equal.
static bool tokcmp(const jsmntok_t* const tok, const char* json,
                   const void* cmp) {
    size_t length = tok->end - tok->start;
    return (memcmp(cmp, json + tok->start, length) == 0);
}

// The following functions try to parse a field from the current JSON token.
// They return the updated index after parsing. If parsing fails, the returned
// value will unchanged (i.e., equal to i).
//
// All of these functions should have a signature of the form:
//   int parse(const char* json, const jsmntok_t* toks, int cursor, ot_op* op)

// {
//   "type": "skip",
//   "count": n
// }
static int parse_skip(const char* json, const jsmntok_t* toks,
                      unsigned int cursor, ot_op* op) {
    unsigned int orig_cursor = cursor;
    
    if (toks[cursor].type != JSMN_OBJECT) {
        return FORMAT_ERR;
    }
    int size = toks[cursor].size;
    cursor++;
    
    int count = 0;
    bool ftype = false, fcount = false;
    for (int i = 0; i < size;) {
        if (tokcmp(toks + cursor, json, "type")) {
            ftype = true;
            ++cursor;
            ++i;
            if (!tokcmp(toks + cursor, json, "skip")) {
                return NOT_MATCH;
            }
            ++cursor;
            ++i;
        } else if (tokcmp(toks + cursor, json, "count")) {
            fcount = true;
            ++cursor;
            ++i;
            count = atoi(json + toks[cursor].start);
            if (count == 0) {
                return FORMAT_ERR;
            }
            ++cursor;
            ++i;
        } else {
            ++i;
        }
    }
    
    if (!ftype || !fcount) {
        return MISSING_FIELD;
    }
    
    ot_comp* comp = array_append(&op->comps);
    comp->type = OT_SKIP;
    comp->value.skip.count = count;
    return (cursor - orig_cursor);
}

static int parse_comps(const char* json, const jsmntok_t* toks,
                       unsigned int cursor, ot_op* op) {
    unsigned int orig_cursor = cursor;
    
    if (toks[cursor].type != JSMN_ARRAY) {
        return FORMAT_ERR;
    }
    int size = toks[cursor].size;
    ++cursor;
    
    int adv;
    for (int i = 0; i < size; ++i) {
        adv = parse_skip(json, toks, cursor, op);
        if (adv > 0) {
            cursor += adv;
            continue;
        }
    }
    
    return (cursor - orig_cursor);
}

// "comps"
// array (size)
// object
// key 1
// field 1
// object
// key 1
// field 1

static int ot_parse_op(ot_op* op, const char* json, jsmntok_t* toks) {
    if (toks[0].type != JSMN_OBJECT) {
        return FORMAT_ERR;
    }
    int size = toks[0].size;
    int cursor = 1;
    int adv;
    
    bool fclient_id = false, fparent = false, fcomps = false;
    for (int i = 0; i < size; ++i) {
        if (tokcmp(toks + cursor, json, "clientId")) {
            fclient_id = true;
            ++cursor;
            op->client_id = atoi(json + toks[cursor].start);
            if (op->client_id == 0) {
                return FORMAT_ERR;
            }
            ++cursor;
            ++i;
        } else if(tokcmp(toks + cursor, json, "parent")) {
            fparent = true;
            ++cursor;
            int res = hextoa(op->parent, json + toks[cursor].start, 128);
            if (res < 0) {
                return FORMAT_ERR;
            }
            ++cursor;
            ++i;
        } else if (tokcmp(toks + cursor, json, "components")) {
            fcomps = true;
            ++cursor;
            adv = parse_comps(json, toks, cursor, op);
            if (adv > 0) {
                cursor += adv;
                ++i;
            } else {
                return adv;
            }
        }
    }
    
    if (!fclient_id || !fparent || !fcomps) {
        return MISSING_FIELD;
    }
    
    return size;
}

ot_op* ot_decode(const char* json) {
    char p[64] = { 0 };
    ot_op* op = ot_new_op(0, p);
    cJSON* root = cJSON_Parse(json);
    
    cJSON* client_idf = cJSON_GetObjectItem(root, "clientId");
    int client_id = 0;
    if (client_idf != NULL) {
        client_id = client_idf->valueint;
    }
    
    cJSON* parentf = cJSON_GetObjectItem(root, "parent");
    char* parent_str = NULL;
    if (parentf != NULL) {
        parent_str = parentf->valuestring;
        hextoa(op->parent, parent_str, 128);
    }
    
    cJSON* components = cJSON_GetObjectItem(root, "components");
    if (components != NULL) {
        int size = cJSON_GetArraySize(components);
        for (int i = 0; i < size; ++i) {
            cJSON* item = cJSON_GetArrayItem(components, i);
            char* type = cJSON_GetObjectItem(item, "type")->valuestring;
            if (memcmp(type, "skip", 4) == 0) {
                ot_comp* skip = array_append(&op->comps);
                skip->type = OT_SKIP;
                skip->value.skip.count = cJSON_GetObjectItem(item, "count")->valueint;
            }
        }
    }
    
    op->client_id = client_id;
    return op;
    
//    int err;
//    jsmn_parser parser;
//    jsmntok_t tokens[128];
//    
//    jsmn_init(&parser);
//    err = jsmn_parse(&parser, json, tokens, 128);
//    char p[64] = { 0 };
//    ot_op* op = ot_new_op(0, p);
//    ot_parse_op(op, json, tokens);
//    array_init(&op->comps, sizeof(ot_comp));
//    return op;
}
