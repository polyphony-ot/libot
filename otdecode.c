#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "otdecode.h"
#include "hex.h"
#include "cJSON.h"

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
}
