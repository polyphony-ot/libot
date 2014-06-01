#include "otdecode.h"

// TODO: Finish implementing decoding of formatting boundaries.
ot_err ot_decode(ot_op* op, const char* json) {
    cJSON* root = cJSON_Parse(json);

    cJSON* client_idf = cJSON_GetObjectItem(root, "clientId");
    if (client_idf == NULL) {
        cJSON_Delete(root);
        return OT_ERR_CLIENT_ID_MISSING;
    }
    op->client_id = (uint32_t)client_idf->valueint;

    cJSON* parentf = cJSON_GetObjectItem(root, "parent");
    if (parentf == NULL) {
        cJSON_Delete(root);
        return OT_ERR_PARENT_MISSING;
    }
    hextoa(op->parent, parentf->valuestring, strlen(parentf->valuestring));

    cJSON* hashf = cJSON_GetObjectItem(root, "hash");
    if (hashf == NULL) {
        cJSON_Delete(root);
        return OT_ERR_HASH_MISSING;
    }
    hextoa(op->hash, hashf->valuestring, strlen(hashf->valuestring));

    cJSON* components = cJSON_GetObjectItem(root, "components");
    if (components == NULL) {
        cJSON_Delete(root);
        return OT_ERR_COMPONENTS_MISSING;
    }

    int size = cJSON_GetArraySize(components);
    for (int i = 0; i < size; ++i) {
        cJSON* item = cJSON_GetArrayItem(components, i);
        char* type = cJSON_GetObjectItem(item, "type")->valuestring;
        if (memcmp(type, "skip", 4) == 0) {
            ot_comp* skip = array_append(&op->comps);
            skip->type = OT_SKIP;
            skip->value.skip.count =
                (uint32_t)cJSON_GetObjectItem(item, "count")->valueint;
        } else if (memcmp(type, "insert", 6) == 0) {
            ot_comp* insert = array_append(&op->comps);
            insert->type = OT_INSERT;

            char* text = cJSON_GetObjectItem(item, "text")->valuestring;
            size_t text_size = sizeof(char) * (strlen(text) + 1);
            insert->value.insert.text = malloc(text_size);
            memcpy(insert->value.insert.text, text, text_size);
        } else if (memcmp(type, "delete", 6) == 0) {
            ot_comp* delete = array_append(&op->comps);
            delete->type = OT_DELETE;
            delete->value.delete.count =
                (uint32_t)cJSON_GetObjectItem(item, "count")->valueint;
        } else if (memcmp(type, "openElement", 11) == 0) {
            ot_comp* open_elem = array_append(&op->comps);
            open_elem->type = OT_OPEN_ELEMENT;
            open_elem->value.open_element.elem =
                cJSON_GetObjectItem(item, "element")->valuestring;
        } else if (memcmp(type, "closeElement", 12) == 0) {
            ot_comp* open_elem = array_append(&op->comps);
            open_elem->type = OT_CLOSE_ELEMENT;
        } else if (memcmp(type, "formattingBoundary", 18) == 0) {
            ot_comp* open_elem = array_append(&op->comps);
            open_elem->type = OT_FORMATTING_BOUNDARY;
        } else {
            cJSON_Delete(root);
            return OT_ERR_INVALID_COMPONENT;
        }
    }

    cJSON_Delete(root);
    return OT_ERR_NONE;
}
