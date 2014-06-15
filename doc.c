#include "doc.h"

static void hash_op(ot_op* op) {
    bool free_snapshot = true;
    char* snapshot = ot_snapshot(op);
    if (snapshot == NULL) {
        free_snapshot = false;
        snapshot = "";
    }

    hash_state md;
    sha1_init(&md);
    sha1_process(&md, snapshot, (uint32_t)strlen(snapshot));
    sha1_done(&md, (char*)&op->hash);

    if (free_snapshot) {
        free(snapshot);
    }
}

ot_doc* ot_new_doc(void) {
    ot_doc* doc = malloc(sizeof(ot_doc));
    array_init(&doc->history, sizeof(ot_op));
    doc->composed = NULL;
    return doc;
}

void ot_free_doc(ot_doc* doc) {
    // Free the components of every op in the document's history.
    ot_op* ops = doc->history.data;
    for (size_t i = 0; i < doc->history.len; ++i) {
        ot_op* op = ops + i;
        ot_comp* comps = op->comps.data;
        for (size_t j = 0; j < op->comps.len; ++j) {
            ot_free_comp(comps + j);
        }
        array_free(&op->comps);
    }

    // Free the history array, which frees the all of ops.
    array_free(&doc->history);

    // When the history length is less than 2, then the composed op is just
    // pointing to the first op in the history (which has already been freed).
    if (doc->history.len > 1) {
        ot_free_op(doc->composed);
    }

    free(doc);
}

ot_err ot_doc_append(ot_doc* doc, ot_op** op) {
    // Move the op into the document's history array.
    ot_op* head = array_append(&doc->history);
    memcpy(head, *op, sizeof(ot_op));

    // If we're appending the first op, then the composed state will simply be
    // the first op in the history. If we're appending the second op, then we
    // must ensure that the composed op still points to the first op in the
    // history in case its location changed after calling array_append (which
    // may have reallocated the array to a different location in memory).
    if (doc->history.len <= 2) {
        doc->composed = (ot_op*)doc->history.data;
    }

    // If we're appending any op after the first, then we must compose the new
    // op with the currently composed state to get the new composed state.
    if (doc->history.len > 1) {
        ot_op* new_composed = ot_compose(doc->composed, head);
        if (new_composed == NULL) {
            doc->history.len--;
            return OT_ERR_APPEND_FAILED;
        }

        // Only free the previously composed op if this is at least the 2nd
        // composition (aka 3rd op in the history). This is because the first
        // composed op for a document points to the first op in the doc's
        // history, and we don't want to free anything in the history.
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

    return OT_ERR_NONE;
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
