#include <stdio.h>
#include "ot.h"
#include "otdecode.h"

int main(int argc, const char * argv[])
{
    char buf[1024];
    fputs("Operation: ", stdout);
    fgets(buf, 1024, stdin);
    
    char p[64];
    ot_op* op = ot_new_op(0, p);
    ot_decode_err err = ot_decode(op, buf);
    if (err != OT_ERR_NONE) {
        puts("Error when decoding JSON.");
        return err;
    }
    
    char* snapshot = ot_snapshot(op);
    puts(snapshot);
    
    return 0;
}

