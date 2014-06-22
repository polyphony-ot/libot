#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

#define MAX_SERVER_QUEUE 1024
#define MAX_CLIENT_QUEUE 1024

#define S(x) #x
#define S_(x) S(x)
#define STRLINE S_(__LINE__)

#define assert_op_snapshot(op, expected, msg)                                  \
    if (!_assert_op_snapshot(op, expected, "Line #" STRLINE                    \
                                           ": Unexpected operation snapshot.", \
                             msg)) {                                           \
        return false;                                                          \
    }

#define assert_convergence(expected, msg)                                      \
    if (!_assert_convergence(expected, "Line #" STRLINE, msg)) {               \
        return false;                                                          \
    }

static ot_server* server;

static size_t clients_len = 0;
static ot_client** clients;

static size_t server_queue_len = 0;
static char* server_queue[MAX_SERVER_QUEUE];

static size_t client_queue_len = 0;
static char* client_queue[MAX_CLIENT_QUEUE];

static void flush_clients() {
    assert(client_queue_len > 0);

    for (size_t i = 0; i < client_queue_len; ++i) {
        ot_server_receive(server, client_queue[i]);
        free(client_queue[i]);
    }

    client_queue_len = 0;
}

static void flush_server() {
    assert(server_queue_len > 0);

    for (size_t i = 0; i < server_queue_len; ++i) {
        for (size_t j = 0; j < clients_len; ++j) {
            ot_client_receive(clients[j], server_queue[i]);
        }
        free(server_queue[i]);
    }

    server_queue_len = 0;
}

static int server_send(const char* op) {
    assert(server_queue_len != MAX_SERVER_QUEUE);

    server_queue[server_queue_len] = strdup(op);
    server_queue_len++;
    return 0;
}

static int client_send(const char* op) {
    assert(client_queue_len != MAX_CLIENT_QUEUE);

    client_queue[client_queue_len] = strdup(op);
    client_queue_len++;
    return 0;
}

static int server_event(ot_event_type t, ot_op* op) {
    assert(t);
    assert(op);
    return 0;
}

static int client_event(ot_event_type t, ot_op* op) {
    assert(t);
    assert(op);
    return 0;
}

static void setup(size_t num_clients) {
    clients_len = num_clients;
    server = ot_new_server(server_send, server_event);

    clients = malloc(sizeof(ot_client*) * num_clients);
    for (size_t i = 0; i < num_clients; ++i) {
        clients[i] = ot_new_client(client_send, client_event, i);
    }
}

static void teardown() {
    for (size_t i = 0; i < clients_len; ++i) {
        ot_free_client(clients[i]);
    }

    ot_free_server(server);
}

static bool assert_ops_equal(ot_op* op1, ot_op* op2, char** msg) {
    bool equal = ot_equal(op1, op2);
    if (!equal) {
        char* op1_enc = ot_encode(op1);
        char* op2_enc = ot_encode(op2);
        asprintf(msg, "Operations aren't equal.\n"
                      "\tOperation1: %s\n"
                      "\tOperation2: %s",
                 op1_enc, op2_enc);
        free(op1_enc);
        free(op2_enc);
    }

    return equal;
}

static bool _assert_op_snapshot(ot_op* op, char* const expected, char* prefix,
                                char** msg) {

    char* actual = ot_snapshot(op);
    int cmp = strcmp(expected, actual);
    if (cmp != 0) {
        char* op_enc = ot_encode(op);
        asprintf(msg, "%s\n"
                      "\tOperation: %s\n"
                      "\tActual Snapshot: %s\n"
                      "\tExpected Snapshot: %s",
                 prefix, op_enc, actual, expected);
        free(actual);
        free(op_enc);
        return false;
    }

    free(actual);
    return true;
}

static bool _assert_convergence(char* const expected, char* loc, char** msg) {
    for (size_t i = 0; i < clients_len; ++i) {
        char* prefix = NULL;
        asprintf(&prefix, "%s: Client %zu didn't converge.", loc, i);
        ot_op* client_op = clients[i]->doc->composed;
        bool passed = _assert_op_snapshot(client_op, expected, prefix, msg);
        free(prefix);
        if (!passed) {
            return false;
        }
    }

    char* prefix = NULL;
    asprintf(&prefix, "%s: The server didn't converge.", loc);
    ot_op* server_op = server->doc->composed;
    bool passed = _assert_op_snapshot(server_op, expected, prefix, msg);
    free(prefix);
    if (!passed) {
        return false;
    }

    return true;
}

#pragma clang diagnostic pop
