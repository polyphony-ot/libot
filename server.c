#include "server.h"

ot_server* ot_new_server(send_func send, ot_event_func event) {
    ot_server* server = malloc(sizeof(ot_server));
    server->send = send;
    server->event = event;
    server->doc = NULL;

    return server;
}

void ot_server_open(ot_server* server, ot_doc* doc) {
    server->doc = doc;
}

void ot_server_receive(ot_server* server, const char* op) {
    fprintf(stderr, "Server received op: %s\n", op);

    ot_op* dec = ot_new_op(0, "");
    ot_decode_err err = ot_decode(dec, op);
    if (err != OT_ERR_NONE) {
        fprintf(stderr, "Server couldn't decode op: %s\n", op);
        ot_free_op(dec);
        return;
    }

    if (server->doc == NULL) {
        server->doc = ot_new_doc();
        ot_op* appended = ot_doc_append(server->doc, dec);
        if (appended == NULL) {
            fprintf(stderr, "Server couldn't append op to a new doc: %s\n", op);
            ot_free_op(dec);
            return;
        }

        char* enc = ot_encode(appended);
        server->send(enc);
        fprintf(stderr, "Server relayed op: %s\n", enc);
        free(enc);
    } else {
        ot_op* composed = ot_doc_compose_after(server->doc, dec->parent);
        if (composed == NULL) {
            fprintf(stderr, "Server couldn't find the parent of the received "
                    "op: %s\n", op);
            ot_free_op(dec);
            return;
        }

        char* composed_enc = ot_encode(composed);
        fprintf(stderr, "Server transforming:\n\t%s\n\t%s\n",
                composed_enc, op);
        free(composed_enc);
        ot_xform_pair p = ot_xform(composed, dec);
        if (p.op1_prime == NULL || p.op2_prime == NULL) {
            fprintf(stderr, "Server couldn't transform the received op: %s\n",
                    op);
            ot_free_op(dec);
            ot_free_op(composed);
            return;
        }

        char* op1_prime_enc = ot_encode(p.op1_prime);
        char* op2_prime_enc = ot_encode(p.op2_prime);
        char* doc_enc = ot_encode(server->doc->composed);

        fprintf(stderr, "Server transformed ops:\n\t%s\n\t%s\n", op1_prime_enc,
                op2_prime_enc);
        fprintf(stderr, "Server appending:\n\t%s\nto doc\n\t%s\n",
                op2_prime_enc, doc_enc);

        free(op1_prime_enc);
        free(op2_prime_enc);
        free(doc_enc);

        p.op2_prime = ot_doc_append(server->doc, p.op2_prime);
        if (p.op2_prime == NULL) {
            fprintf(stderr, "Server couldn't append op.\n");
            ot_free_op(dec);
            ot_free_op(composed);
            ot_free_op(p.op1_prime);
            ot_free_op(p.op2_prime);
            return;
        }
        fprintf(stderr, "Server appended op.\n");

        op2_prime_enc = ot_encode(p.op2_prime);
        server->send(op2_prime_enc);
        fprintf(stderr, "Server relayed op: %s\n", op2_prime_enc);
        free(op2_prime_enc);

        ot_free_op(composed);
        ot_free_op(p.op1_prime);
    }

    //ot_free_op(dec);
    char* doc_enc = ot_encode(server->doc->composed);;
    fprintf(stderr, "Server's document is now: %s\n", doc_enc);
    free(doc_enc);
}
