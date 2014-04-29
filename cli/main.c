#include <stdio.h>
#include "ot.h"
#include "compose.h"
#include "otdecode.h"

int main()
{
    char buf[1024];
    fputs("Operation 1: ", stdout);
    fgets(buf, 1024, stdin);
    
    char p[64];
    ot_op* op = ot_new_op(0, p);
    ot_decode_err err = ot_decode(op, buf);
    if (err != OT_ERR_NONE) {
        puts("Error when decoding JSON.");
        return err;
    }
    
    
    char buf2[1024];
    fputs("Operation 2: ", stdout);
    fgets(buf2, 1024, stdin);
    
    ot_op* op2 = ot_new_op(0, p);
    err = ot_decode(op2, buf2);
    if (err != OT_ERR_NONE) {
        puts("Error when decoding JSON.");
        return err;
    }
    
    ot_op* composed = ot_compose(op, op2);
    char* snapshot = ot_snapshot(composed);
    puts("Composed: ");
    puts(snapshot);
    
    return 0;
}

