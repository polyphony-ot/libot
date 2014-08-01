#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../../server.h"
#include "../../client.h"
#include "scenario.h"

bool scenario2(char** msg) {
    setup(2);

    ot_op* opc = ot_new_op();
    ot_insert(opc, "ABC");
    ot_client_apply(clients[1], &opc);

    flush_client(1);
    assert_op_snapshot(server->doc->composed, "ABC", msg);
    assert_op_snapshot(clients[1]->doc->composed, "ABC", msg);

    ot_op* opa = ot_new_op();
    ot_insert(opa, "abc");
    ot_client_apply(clients[0], &opa);
    assert_op_snapshot(clients[0]->doc->composed, "abc", msg);

    ot_op* opb = ot_new_op();
    ot_skip(opb, 3);
    ot_insert(opb, "def");
    ot_client_apply(clients[0], &opb);
    assert_op_snapshot(clients[0]->doc->composed, "abcdef", msg);

    flush_server();
    assert_op_snapshot(server->doc->composed, "ABC", msg);
    assert_op_snapshot(clients[0]->doc->composed, "ABCabcdef", msg);
    assert_op_snapshot(clients[1]->doc->composed, "ABC", msg);

    ot_op* ope = ot_new_op();
    ot_skip(ope, 9);
    ot_insert(ope, "ghi");
    ot_client_apply(clients[0], &ope);
    assert_op_snapshot(clients[0]->doc->composed, "ABCabcdefghi", msg);

    ot_op* opd = ot_new_op();
    ot_skip(opd, 3);
    ot_insert(opd, "DEF");
    ot_client_apply(clients[1], &opd);

    flush_client(1);
    assert_op_snapshot(server->doc->composed, "ABCDEF", msg);
    assert_op_snapshot(clients[1]->doc->composed, "ABCDEF", msg);

    flush_server();
    assert_op_snapshot(clients[0]->doc->composed, "ABCDEFabcdefghi", msg);
    assert_op_snapshot(clients[1]->doc->composed, "ABCDEF", msg);

    flush_client(0);
    flush_server();
    assert_op_snapshot(server->doc->composed, "ABCDEFabc", msg);
    assert_op_snapshot(clients[0]->doc->composed, "ABCDEFabcdefghi", msg);
    assert_op_snapshot(clients[1]->doc->composed, "ABCDEFabc", msg);

    flush_client(0);
    flush_server();
    assert_convergence("ABCDEFabcdefghi", msg);

    teardown();

    return true;
}
