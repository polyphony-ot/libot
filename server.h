#ifndef libot_server_h
#define libot_server_h

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "otencode.h"

typedef struct {
    send_func send;
    receive_func receive;
} ot_server;

ot_server* ot_new_server(send_func send, receive_func receive);
void ot_server_send(ot_server* server, ot_op* op);
void ot_server_receive(ot_server* server, ot_op* op);

#endif
