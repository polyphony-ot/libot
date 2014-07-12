#ifndef libot_otencode_h
#define libot_otencode_h

#include <inttypes.h>
#include "ot.h"
#include "doc.h"
#include "cJSON.h"

// Encodes an operation as a UTF-8 JSON string.
char* ot_encode(const ot_op* const op);

// ot_doc_encode encodes a document as a UTF-8 JSON string.
char* ot_encode_doc(const ot_doc* const doc);

#endif
