#include "client.h"

ot_client* ot_new_client(send_func send, ot_event_func event) {
    ot_client* client = malloc(sizeof(ot_client));
    client->send = send;
    client->event = event;
    client->doc = NULL;
    client->client_id = 0;

    return client;
}

void ot_client_open(ot_client* client, ot_op* op) {
    client->doc = op;
}

void ot_client_receive(ot_client* client, const char* op) {
    fprintf(stderr, "Client received op: %s\n", op);

    ot_op* dec = ot_new_op(0, "");
    ot_decode(dec, op);
    if (dec->client_id == client->client_id) {
        client->client_id = 0;
        fputs("Ignoring op because it was created by this client.\n", stderr);
        return;
    }

    ot_xform_pair p = ot_xform(client->doc, dec);
    ot_op* newdoc = ot_compose(client->doc, p.op2_prime);
    client->doc = newdoc;
}

void ot_client_apply(ot_client* client, ot_op* op) {
    if (client->client_id) {
        fputs("Cannot apply op because we're waiting for a response from the server.\n", stderr);
        return;
    }

    if (client->doc == NULL) {
        client->doc = op;
    } else {
        client->doc = ot_compose(client->doc, op);
    }

    char* enc = ot_encode(op);
    client->send(enc);
    client->client_id = op->client_id;
    fprintf(stderr, "Client sent op: %s\n", enc);
    free(enc);
}
