#ifndef libot_xform_h
#define libot_xform_h

#include <stdlib.h>
#include "ot.h"

typedef struct ot_xform_pair {
    ot_op* op1_prime;
    ot_op* op2_prime;
} ot_xform_pair;

ot_xform_pair ot_xform(ot_op* op1, ot_op* op2);

#endif
