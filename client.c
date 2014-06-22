#include "client.h"

static void free_anticipated(ot_client* client) {
    if (client->anticipated == NULL) {
        return;
    }

    if (client->free_anticipated_comps) {
        ot_free_op(client->anticipated);
        client->free_anticipated_comps = false;
    } else {
        free(client->anticipated);
    }
}

static void free_buffer(ot_client* client) {
    if (client->buffer == NULL) {
        return;
    }

    if (client->free_buffer_comps) {
        ot_free_op(client->buffer);
        client->free_buffer_comps = false;
    } else {
        free(client->buffer);
    }

    client->buffer = NULL;
}

static ot_err buffer_op(ot_client* client, ot_op* op) {
    if (client->buffer == NULL) {
        client->buffer = malloc(sizeof(ot_op));
        memcpy(client->buffer, op, sizeof(ot_op));

        // Don't free the buffer op's components because it's a shallow copy of
        // an op in the doc.
        client->free_buffer_comps = false;
        return OT_ERR_NONE;
    }

    ot_op* composed = ot_compose(client->buffer, op);
    if (composed == NULL) {
        char* enc = ot_encode(op);
        fprintf(stderr, "Client couldn't add op to the buffer: %s\n", enc);
        free(enc);
        return OT_ERR_BUFFER_FAILED;
    }

    free_buffer(client);

    // Set the buffer and mark its components as freeable because it doesn't
    // point to anywhere within the doc.
    client->buffer = composed;
    client->free_buffer_comps = true;

    char* enc = ot_encode(composed);
    fprintf(stderr, "Client's buffer is now: %s\n", enc);
    free(enc);

    return 0;
}

static void send_buffer(ot_client* client) {
    if (client->buffer == NULL) {
        return;
    }

    char* enc_buf = ot_encode(client->buffer);
    client->send(enc_buf);
    free(enc_buf);

    free_anticipated(client);
    client->anticipated = malloc(sizeof(ot_op));
    memcpy(client->anticipated, client->buffer, sizeof(ot_op));
    client->free_anticipated_comps = false;

    free_buffer(client);
    client->ack_required = true;
}

static void fire_op_event(ot_client* client, ot_op* op) {
    client->event(OT_OP_APPLIED, op);
}

ot_client* ot_new_client(send_func send, ot_event_func event, uint32_t id) {
    ot_client* client = malloc(sizeof(ot_client));
    client->buffer = NULL;
    client->anticipated = NULL;
    client->send = send;
    client->event = event;
    client->doc = NULL;
    client->client_id = id;
    client->ack_required = false;
    client->free_anticipated_comps = false;
    client->free_buffer_comps = false;

    return client;
}

void ot_free_client(ot_client* client) {
    ot_doc* doc = client->doc;
    if (doc != NULL) {
        ot_free_doc(client->doc);
    }

    free_anticipated(client);
    free_buffer(client);
    free(client);
}

void ot_client_open(ot_client* client, ot_doc* doc) { client->doc = doc; }

void ot_client_receive(ot_client* client, const char* op) {
    fprintf(stderr, "Client received op: %s\n", op);

    ot_op* dec = ot_new_op(0, "");
    ot_err err = ot_decode(dec, op);
    if (err != OT_ERR_NONE) {
        fprintf(stderr, "Client couldn't decode op. Error code: %d.", err);
        ot_free_op(dec);
        return;
    }

    if (dec->client_id == client->client_id) {
        char hex[41];
        atohex((char*)&hex, (char*)&dec->hash, 20);
        fprintf(stderr, "Op %s was acknowledged.\n", hex);
        ot_free_op(dec);

        client->ack_required = false;
        send_buffer(client);
        return;
    }

    ot_op* apply;
    if (client->anticipated == NULL) {
        apply = dec;
    } else {
        ot_xform_pair p = ot_xform(dec, client->anticipated);
        free_anticipated(client);
        ot_free_op(dec);
        client->anticipated = p.op2_prime;
        client->free_anticipated_comps = true;

        if (client->buffer != NULL) {
            ot_xform_pair p2 = ot_xform(client->buffer, p.op1_prime);
            free_buffer(client);
            ot_free_op(p.op1_prime);
            client->buffer = p2.op1_prime;
            client->free_buffer_comps = true;

            apply = p2.op2_prime;
        } else {
            apply = p.op1_prime;
        }
    }

    if (client->doc == NULL) {
        client->doc = ot_new_doc();
    }
    ot_doc_append(client->doc, &apply);
    fire_op_event(client, apply);
}

ot_err ot_client_apply(ot_client* client, ot_op** op) {
    if (client->doc == NULL) {
        client->doc = ot_new_doc();
    }

    ot_doc* doc = client->doc;
    ot_err append_err = ot_doc_append(doc, op);
    if (append_err != OT_ERR_NONE) {
        return append_err;
    }

    ot_err buf_err = buffer_op(client, *op);
    if (buf_err != OT_ERR_NONE) {
        return buf_err;
    }

    if (!client->ack_required) {
        send_buffer(client);
    }

    return 0;
}
