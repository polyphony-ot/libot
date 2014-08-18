#ifndef libot_client_h
#define libot_client_h

#include <stdlib.h>
#include <stdio.h>
#include "array.h"
#include "compose.h"
#include "doc.h"
#include "xform.h"
#include "encode.h"
#include "decode.h"

typedef struct {
    send_func send;
    ot_event_func event;
    ot_doc* doc;
    uint32_t client_id;
    bool ack_required;
    ot_op* anticipated;
    bool free_anticipated_comps;
    ot_op* buffer;
    bool free_buffer_comps;
} ot_client;

ot_client* ot_new_client(send_func send, ot_event_func event);

void ot_free_client(ot_client* client);

void ot_client_open(ot_client* client, ot_doc* doc);

void ot_client_receive(ot_client* client, const char* op);

ot_err ot_client_apply(ot_client* client, ot_op** op);

#endif
