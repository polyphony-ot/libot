#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "otencode.h"
#include "hex.h"

static void append_str(char** buf,
                       size_t* const bufsize,
                       size_t* const written,
                       const char* const str,
                       const size_t strsize) {
    *bufsize += strsize;
    *buf = realloc(*buf, *bufsize);
    memcpy(*buf + *written, str, strsize);
    *written += strsize;
}

// Writes a format string to a buffer, resizing it if necessary. If buf is
// resized, then it will be updated to point to the reallocated memory and
// bufsize will be udpated with the new buffer size. written will contain the
// number of bytes writen to buf.
static void fmtstr_to_buffer(char** buf,
                             size_t* const bufsize,
                             size_t* const written,
                             const char* const fmt,
                             ...) {
    va_list args;
    va_start(args, fmt);
    *bufsize += vsnprintf(NULL, 0, (char*) fmt, args);
    va_end(args);
    
    *buf = realloc(*buf, *bufsize);
    
    // WARNING: Do not reuse args. vsnprintf may have modified it, so we must
    // declare a new va_list.
    va_list args2;
    va_start(args2, fmt);
    *written += vsprintf((char*) *buf + *written, (char*) fmt, args2);
    va_end(args2);
}

char* ot_encode(const ot_op* const op) {
    ot_comp* comps = op->comps.data;
    
    char* buf = NULL;
    size_t written = 0;
    // Start the buffer size at 1 to accommodate the terminating NULL.
    size_t bufsize = 1;
    char* init_fmt = "{ \"clientId\": %d, "
                       "\"parent\": \"%s\", "
                       "\"components\": [ ";
    char hex[128] = { 0 };
    atohex(&op->parent, hex, 64);
    fmtstr_to_buffer(&buf, &bufsize, &written, init_fmt, op->client_id, hex);
    
	for (int i = 0; i < op->comps.len; ++i)
	{
        ot_comp_type t = comps[i].type;
        if (t == OT_SKIP) {
            int64_t count = comps[i].value.skip.count;
            char* fmt = "{ \"type\": \"skip\", \"count\": %d }, ";
            fmtstr_to_buffer(&buf, &bufsize, &written, fmt, count);
        } else if (t == OT_INSERT) {
            rope* r = comps[i].value.insert.text;
            uint8_t* textstr = rope_create_cstr(r);
            char* fmt = "{ \"type\": \"insert\", \"text\": \"%s\" }, ";
            fmtstr_to_buffer(&buf, &bufsize, &written, fmt, textstr);
            free(textstr);
		} else if (t == OT_DELETE) {
            int64_t count = comps[i].value.delete.count;
            char* fmt = "{ \"type\": \"delete\", \"count\": %d }, ";
            fmtstr_to_buffer(&buf, &bufsize, &written, fmt, count);
        } else if (t == OT_OPEN_ELEMENT) {
            rope* r = comps[i].value.open_element.elem;
            uint8_t* textstr = rope_create_cstr(r);
            char* fmt = "{ \"type\": \"openElement\", \"element\": \"%s\" }, ";
            fmtstr_to_buffer(&buf, &bufsize, &written, fmt, textstr);
            free(textstr);
        } else if (t == OT_CLOSE_ELEMENT) {
            char* fmt = "{ \"type\": \"closeElement\" }, ";
            fmtstr_to_buffer(&buf, &bufsize, &written, fmt);
        }
	}
    
    if (op->comps.len > 0) {
        written -= 2;
        append_str(&buf, &bufsize, &written, " ] }", 5);
    } else {
        append_str(&buf, &bufsize, &written, "] }", 4);
    }
    
	return buf;
}