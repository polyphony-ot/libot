#ifndef libot_client_h
#define libot_client_h

#include <stdlib.h>
#include <stdio.h>
#include "otencode.h"

typedef struct {
    send_func send;
    receive_func receive;
} ot_client;

ot_client* ot_new_client(send_func send, receive_func receive);

void ot_client_send(ot_client* client, ot_op* op);

#endif
