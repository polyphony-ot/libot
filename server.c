#include "server.h"

ot_server* ot_new_server(send_func send, receive_func receive) {
    ot_server* server = malloc(sizeof(ot_server));
    server->send = send;
    server->receive = receive;

    return server;
}

void ot_server_send(ot_server* server, ot_op* op) {
    char* enc = ot_encode(op);
    server->send(enc);
    printf("Server sent op: %s", enc);
}

void ot_server_receive(ot_server* server, ot_op* op) {
    assert(server);
    char* enc = ot_encode(op);
    printf("Server received op: %s", enc);
}
