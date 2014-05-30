#include "client.h"

static void buffer_op(ot_client* client, ot_op* op) {
    if (client->buffer == NULL) {
        client->buffer = op;
    }

    ot_op* composed = ot_compose(client->buffer, op);
    if (composed == NULL) {
        char* enc = ot_encode(op);
        fprintf(stderr, "Client couldn't add op to the buffer: %s\n", enc);
        free(enc);
        return;
    }

    ot_free_op(client->buffer);
    client->buffer = composed;

    char* enc = ot_encode(composed);
    fprintf(stderr, "Client's buffer is now: %s\n", enc);
    free(enc);
}

static void send_buffer(ot_client* client) {
    if (client->buffer == NULL) {
        return;
    }

    char* enc_buf = ot_encode(client->buffer);
    client->send(enc_buf);

    client->sent = client->buffer;
    client->buffer = NULL;
    client->anticipated = client->sent;
}

static void fire_op_event(ot_client* client, ot_op* op) {
    assert(client);
    assert(op);
}

ot_client* ot_new_client(send_func send, ot_event_func event, uint32_t id) {
    ot_client* client = malloc(sizeof(ot_client));
    client->send = send;
    client->event = event;
    client->doc = NULL;
    client->client_id = id;

    return client;
}

void ot_client_open(ot_client* client, ot_doc* doc) {
    client->doc = doc;
}

void ot_client_receive(ot_client* client, const char* op) {
    fprintf(stderr, "Client received op: %s\n", op);

    ot_op* dec = ot_new_op(0, "");
    ot_decode(dec, op);
    if (dec->client_id == client->client_id) {
        char hex[41];
        atohex((char*)&hex, (char*)&dec->hash, 20);
        fprintf(stderr, "Op %s was acknowledged.\n", hex);
        ot_free_op(dec);

        send_buffer(client);
        return;
    }

    ot_xform_pair p = ot_xform(client->anticipated, dec);
    ot_free_op(client->anticipated);
    client->anticipated = p.op1_prime;

    ot_xform_pair p2 = ot_xform(client->buffer, p.op2_prime);
    ot_free_op(client->buffer);
    ot_free_op(p.op2_prime);
    client->buffer = p2.op1_prime;

    if (client->doc == NULL) {
        client->doc = ot_new_doc();
    }
    ot_op* appended = ot_doc_append(client->doc, p2.op2_prime);
    fire_op_event(client, appended);
}

ot_op* ot_client_apply(ot_client* client, ot_op* op) {
    if (client->doc == NULL) {
        client->doc = ot_new_doc();
    }

    ot_doc* doc = client->doc;
    ot_op* appended = ot_doc_append(doc, op);
    buffer_op(client, appended);

    if (client->sent == NULL) {
        send_buffer(client);
    }

    return appended;
}
