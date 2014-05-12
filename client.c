#include "client.h"

ot_client* ot_new_client(send_func send, receive_func receive) {
    ot_client* client = malloc(sizeof(ot_client));
    client->send = send;
    client->receive = receive;

    return client;
}

void ot_client_send(ot_client* client, ot_op* op) {
    char* enc = ot_encode(op);
    client->send(enc);
    printf("Client sent op: %s", enc);
}
