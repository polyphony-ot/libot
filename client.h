#ifndef libot_client_h
#define libot_client_h

#include <stdlib.h>
#include <stdio.h>
#include "compose.h"
#include "xform.h"
#include "otencode.h"
#include "otdecode.h"

typedef struct {
    send_func send;
    ot_event_func event;
    ot_op* doc;
    uint32_t client_id;
} ot_client;

ot_client* ot_new_client(send_func send, ot_event_func event);

void ot_client_open(ot_client* client, ot_op* op);

void ot_client_receive(ot_client* client, const char* op);

void ot_client_apply(ot_client* client, ot_op* op);

#endif
