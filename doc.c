#include "doc.h"

static void hash_op(ot_op* op) {
    char* snapshot = ot_snapshot(op);
    hash_state md;
    sha1_init(&md);
    sha1_process(&md, snapshot, (uint32_t)strlen(snapshot));
    sha1_done(&md, (char*)&op->hash);
}

ot_doc* ot_new_doc(void) {
    ot_doc* doc = malloc(sizeof(ot_doc));
    array_init(&doc->history, sizeof(ot_op));
    doc->composed = NULL;
    return doc;
}

void ot_free_doc(ot_doc* doc) {
    array_free(&doc->history);
    free(doc);
}

ot_doc_err ot_doc_append(ot_doc* doc, ot_op** op) {
    // Move the op into the document's history array.
    ot_op* head = array_append(&doc->history);
    memcpy(head, *op, sizeof(ot_op));

    if (doc->composed == NULL) {
        // If a document only has one op, then its composed state is simply
        // equal to that op.

        // We have to be careful to not free this op pointer later because it
        // points to head, which is in the doc's history.
        doc->composed = head;
    } else {
        // If this isn't the first op, then we must update the doc's composed
        // state by composing the previous composed op with the new op.

        ot_op* new_composed = ot_compose(doc->composed, head);
        if (new_composed == NULL) {
            doc->history.len--;
            return OT_ERR_APPEND;
        }

        // Only free the previously composed op if this is the 2nd composition
        // (aka 3rd op in the history). This is because the first composed op
        // for a document points to the first op in the doc's history, and we
        // don't want to free anything in the history.
        if (doc->history.len > 2) {
            ot_free_op(doc->composed);
        }
        doc->composed = new_composed;
    }

    // Don't use ot_free_op because we only want to free the ot_op struct, not
    // its components. We must also only free op if the composition was a
    // success.
    free(*op);
    *op = head;

    // The newly composed operation wil have the same hash as the appended op,
    // so we can get away with calculating the hash once and then copying it.
    hash_op(doc->composed);
    memcpy(head->hash, doc->composed->hash, 20);

    return 0;
}

ot_op* ot_doc_compose_after(const ot_doc* doc, const char* after) {
    array history = doc->history;
    if (history.len < 2) {
        return NULL;
    }

    ot_op* ops = (ot_op*)history.data;
    size_t start = 0;
    for (size_t i = history.len - 1; i > 0; --i) {
        ot_op* op = ops + i;
        if (memcmp(op->hash, after, sizeof(char) * 20) == 0) {
            start = i + 1;
            break;
        }
    }

    if (start == 0) {
        return NULL;
    }

    ot_op* composed = ops + start;
    ot_op* temp;
    for (size_t i = start + 1; i < history.len; ++i) {
        temp = ot_compose(composed, ops + i);
        ot_free_op(composed);
        composed = temp;
        if (composed == NULL) {
            return NULL;
        }
    }

    return composed;
}
