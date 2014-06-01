#ifndef libot_client_h
#define libot_client_h

#include <stdlib.h>
#include <stdio.h>
#include "array.h"
#include "compose.h"
#include "doc.h"
#include "xform.h"
#include "otencode.h"
#include "otdecode.h"

typedef enum {
    // Client couldn't buffer the applied operation, usually because it wasn't
    // composable with the buffer.
    OT_ERR_BUFFER_FAILED = 1,
    OT_ERR_APPEND_FAILED = 2
} ot_client_err;

typedef struct {
    send_func send;
    ot_event_func event;
    ot_doc* doc;
    uint32_t client_id;
    ot_op* sent;
    ot_op* anticipated;
    ot_op* buffer;
} ot_client;

ot_client* ot_new_client(send_func send, ot_event_func event, uint32_t id);

void ot_client_open(ot_client* client, ot_doc* doc);

void ot_client_receive(ot_client* client, const char* op);

ot_client_err ot_client_apply(ot_client* client, ot_op** op);

#endif
