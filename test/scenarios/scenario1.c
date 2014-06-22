#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../server.h"
#include "../../client.h"
#include "scenario.h"

// In this scenario, there are two clients starting with an empty doc. Client0
// and client1 simultaneously send opA and opC, respectively. opC happens to be
// received first. Client0 then applies and buffers another op, opB, before
// receiving any response. At this point, the server acknowledges opC and opA.
// Finally, client0 sends its buffered op, opB, after receiving the
// acknowledgment of opC.
bool scenario1(char** msg) {
    setup(2);

    char parent[20] = { 0 };
    ot_op* opc = ot_new_op(1, parent);
    ot_insert(opc, "ABC");
    ot_client_apply(clients[1], &opc);
    flush_clients();
    assert_op_snapshot(clients[1]->doc->composed, "ABC", msg);

    ot_op* opa = ot_new_op(0, parent);
    ot_insert(opa, "abc");
    ot_client_apply(clients[0], &opa);
    flush_clients();
    assert_op_snapshot(clients[0]->doc->composed, "abc", msg);

    ot_op* opb = ot_new_op(0, opa->hash);
    ot_skip(opb, 3);
    ot_insert(opb, "def");
    ot_client_apply(clients[0], &opb);
    assert_op_snapshot(clients[0]->doc->composed, "abcdef", msg);

    flush_server();

    assert_op_snapshot(server->doc->composed, "ABCabc", msg);
    assert_op_snapshot(clients[0]->doc->composed, "ABCabcdef", msg);
    assert_op_snapshot(clients[1]->doc->composed, "ABCabc", msg);

    flush_clients();
    flush_server();

    assert_convergence("ABCabcdef", msg);

    teardown();

    return true;
}
