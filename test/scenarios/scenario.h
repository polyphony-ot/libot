#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

// The maximum number of servers that can be used in a scenario.
#define MAX_SERVER_QUEUE 16

// The maximum number of clients that can be used in a scenario.
#define MAX_CLIENTS 16

// The maximum number of operations that can be queued by a client.
#define MAX_CLIENT_QUEUE 16

// Debugging macros for creating a string containing the current line number.
#define S(x) #x
#define S_(x) S(x)
#define STRLINE S_(__LINE__)

// Asserts that an operation's snapshot is equal to an expected string. If it
// isn't, msg will be set to an error message.
#define assert_op_snapshot(op, expected, msg)                                  \
    if (!_assert_op_snapshot(op, expected, "Line #" STRLINE                    \
                                           ": Unexpected operation snapshot.", \
                             msg)) {                                           \
        return false;                                                          \
    }

// Asserts that all of the clients and the server have converged on the same
// document. If some of them have not converged, msg will be set to an error
// message.
#define assert_convergence(expected, msg)                                      \
    if (!_assert_convergence(expected, "Line #" STRLINE, msg)) {               \
        return false;                                                          \
    }

// The single server used in a scenario.
static ot_server* server;

// An array of clients used in a scenario. clients_len is specified when the
// setup function is called.
static size_t clients_len = 0;
static ot_client* clients[MAX_CLIENTS];

// An array of staged server operations.
static size_t server_queue_len = 0;
static char* server_queue[MAX_SERVER_QUEUE];

// Arrays of staged client operations.
static size_t client_queue_lens[MAX_CLIENTS] = { 0 };
static char* client_queues[MAX_CLIENTS][MAX_CLIENT_QUEUE];

// Flushes all operations sent by a specific client from the staging area.
static void flush_client(size_t id) {
    assert(client_queue_lens[id] > 0);

    for (size_t i = 0; i < client_queue_lens[id]; ++i) {
        char* data = client_queues[id][i];
        ot_server_receive(server, data);
        free(data);
    }

    client_queue_lens[id] = 0;
}

// Flushes all sent client operations from the staging area.
static void flush_clients() {
    bool flushed = false;

    for (size_t i = 0; i < clients_len; ++i) {
        if (client_queue_lens[i] > 0) {
            flushed = true;
            flush_client(i);
        }
    }

    assert(flushed);
}

// Flushes all sent server operations from the staging area.
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

// Server send callback that stores op in the staging area until it is flushed.
static int server_send(const char* op) {
    assert(server_queue_len != MAX_SERVER_QUEUE);

    server_queue[server_queue_len] = strdup(op);
    server_queue_len++;
    return 0;
}

// Client send callback that stores op in the staging area until it is flushed.
static int client_send(const char* op) {
    ot_op* dec = ot_new_op();
    ot_decode(dec, op);
    size_t id = dec->client_id;
    ot_free_op(dec);

    assert(client_queue_lens[id] != MAX_CLIENT_QUEUE);

    client_queues[id][client_queue_lens[id]] = strdup(op);
    client_queue_lens[id]++;
    return 0;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

// Server event callback function that simply asserts that an event type was
// provided.
static int server_event(ot_event_type t, ot_op* op) {
    assert(t);
    return 0;
}

// Client event callback function used by all of the clients that simply asserts
// that an event type was provided.
static int client_event(ot_event_type t, ot_op* op) {
    assert(t);
    return 0;
}

#pragma clang diagnostic pop

// This function should be called at the beginning of every scenario. It
// initializes all of the clients and the server. num_clients should be the
// maximum number of clients used in the scenario.
static void setup(size_t num_clients) {
    clients_len = num_clients;
    server = ot_new_server(server_send, server_event);

    for (size_t i = 0; i < num_clients; ++i) {
        clients[i] = ot_new_client(client_send, client_event);
        clients[i]->client_id = i;
    }
}

// This function should be called at the end of every scenario. It cleans up all
// the clients and the server.
static void teardown() {
    for (size_t i = 0; i < clients_len; ++i) {
        ot_free_client(clients[i]);
    }

    ot_free_server(server);
}

// Asserts that two operations are equal. If they aren't, msg will be set to an
// error message.
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

// Asserts that an operation's snapshot is equal to an expected string. If it
// isn't, msg will be set to an error message. A prefix may be provided which
// will be prepended to the message. This function is typically not used
// directly, see the assert_op_snapshot macro instead.
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

// Asserts that all of the clients and the server have converged on the same
// document. If some of them have not converged, msg will be set to an error
// message. loc is the location in the source file where this assert was called.
// This function is typically not used directly, see the assert_convergence
// macro instead.
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
