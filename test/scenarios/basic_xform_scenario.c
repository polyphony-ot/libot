#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../server.h"
#include "../../client.h"
#include "scenario.h"

bool basic_xform_scenario(char** msg) {
    setup(2);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "abc");
    ot_client_apply(clients[0], &opa);
    flush_client(0);
    assert_op_snapshot(clients[0]->doc->composed, "abc", msg);

    ot_op* opb = ot_new_op();
    ot_insert(opb, "def");
    ot_client_apply(clients[1], &opb);
    flush_client(1);
    assert_op_snapshot(clients[1]->doc->composed, "def", msg);

    flush_server();

    assert_convergence("abcdef", msg);

    teardown();
    return true;
}
