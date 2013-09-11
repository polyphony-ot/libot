#include <stdlib.h>
#include "gen.h"

ot_op* gen_op() {
    int64_t client_id = rand();
    int64_t parent[8];
    for (int i = 0; i < 8; ++i) {
        parent[i] = rand();
    }
    
    int64_t num_comps = rand();
    
    return NULL;
}