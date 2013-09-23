#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "otdecode.h"
#include "hex.h"
#include "cJSON.h"

ot_decode_err ot_decode(ot_op* op, const char* json) {
    cJSON* root = cJSON_Parse(json);
    
    cJSON* client_idf = cJSON_GetObjectItem(root, "clientId");
    if (client_idf != NULL) {
        op->client_id = client_idf->valueint;
    }
    
    cJSON* parentf = cJSON_GetObjectItem(root, "parent");
    if (parentf != NULL) {
        hextoa(op->parent, parentf->valuestring, 128);
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
    
    return OT_ERR_NONE;
}
