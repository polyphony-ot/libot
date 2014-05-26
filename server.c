#include "server.h"

static bool can_append(const ot_doc* doc, const ot_op* op) {
    return memcmp(doc->composed->hash, op->parent, sizeof(char) * 20) == 0;
}

static void append_op(ot_server* server, ot_op* op) {
    ot_doc* doc = server->doc;
    char* doc_enc = ot_encode(doc->composed);
    char* op_enc = ot_encode(op);
    fprintf(stderr, "Server appending:\n\t%s\n\t%s\n", doc_enc, op_enc);
    free(doc_enc);
    free(op_enc);

    ot_op* appended = ot_doc_append(doc, op);
    if (appended == NULL) {
        fprintf(stderr, "Server append failed.\n");
        return;
    }

    char* append_enc = ot_encode(appended);
    server->send(append_enc);
    fprintf(stderr, "Server sent op: %s\n", append_enc);
    free(append_enc);
}

static ot_op* xform(const ot_doc* doc, ot_op* op) {
    char* op_enc = ot_encode(op);
    ot_op* composed = ot_doc_compose_after(doc, op->parent);
    if (composed == NULL) {
        fprintf(stderr, "Server couldn't find the parent of the received "
                        "op: %s\n",
                op_enc);
        free(op_enc);
        return NULL;
    }

    char* composed_enc = ot_encode(composed);
    fprintf(stderr, "Server transforming:\n\t%s\n\t%s\n", composed_enc, op_enc);
    free(composed_enc);
    ot_xform_pair p = ot_xform(composed, op);
    if (p.op1_prime == NULL || p.op2_prime == NULL) {
        fprintf(stderr, "Server transform failed.\n");
        free(op_enc);
        ot_free_op(composed);
        return NULL;
    }

    char* op1_prime_enc = ot_encode(p.op1_prime);
    char* op2_prime_enc = ot_encode(p.op2_prime);

    fprintf(stderr, "Server transform succeeded:\n\t%s\n\t%s\n", op1_prime_enc,
            op2_prime_enc);

    free(op_enc);
    free(op1_prime_enc);
    free(op2_prime_enc);
    ot_free_op(p.op1_prime);

    return p.op2_prime;
}

ot_server* ot_new_server(send_func send, ot_event_func event) {
    ot_server* server = malloc(sizeof(ot_server));
    server->send = send;
    server->event = event;
    server->doc = NULL;

    return server;
}

void ot_server_open(ot_server* server, ot_doc* doc) { server->doc = doc; }

void ot_server_receive(ot_server* server, const char* op) {
    fprintf(stderr, "Server received op: %s\n", op);

    ot_op* dec = ot_new_op(0, "");
    ot_decode_err err = ot_decode(dec, op);
    if (err != OT_ERR_NONE) {
        fprintf(stderr, "Server couldn't decode op: %s\n", op);
        ot_free_op(dec);
        return;
    }

    ot_doc* doc = server->doc;
    if (doc == NULL) {
        server->doc = ot_new_doc();
        append_op(server, dec);
    } else if (can_append(doc, dec)) {
        append_op(server, dec);
    } else {
        ot_op* op_prime = xform(doc, dec);
        append_op(server, op_prime);
    }

    ot_free_op(dec);
    char* doc_enc = ot_encode(doc->composed);
    fprintf(stderr, "Server's document is now: %s\n", doc_enc);
    free(doc_enc);
}
