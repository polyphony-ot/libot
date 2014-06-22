#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../server.h"
#include "../../client.h"
#include "scenario.h"

bool scenario3(char** msg) {
    setup(2);

    char parent[20] = { 0 };
    ot_op* opa = ot_new_op(0, parent);
    ot_insert(opa, "a");
    ot_client_apply(clients[0], &opa);

    flush_clients();
    flush_server();
    assert_convergence("a", msg);

    ot_op* opb = ot_new_op(1, opa->hash);
    ot_skip(opb, 1);
    ot_insert(opb, "b");
    ot_client_apply(clients[1], &opb);

    flush_clients();
    flush_server();
    assert_convergence("ab", msg);

    ot_op* opc = ot_new_op(0, opb->hash);
    ot_skip(opc, 1);
    ot_delete(opc, 1);
    ot_client_apply(clients[0], &opc);

    flush_clients();
    flush_server();
    assert_convergence("a", msg);

    teardown();

    return true;
}
