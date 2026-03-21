// =============================================================================
// File:              main.cpp
// Author(s):         Chrischn89
// Description:
//   TesseraHost entry point. 32-bit host process that bridges CvGameCoreDLL.dll
//   to Godot via TCP. See design spec for architecture details.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>

#include "tcp_server.h"
#include "message_protocol.h"

int main(int argc, char* argv[]) {
    printf("[TesseraHost] Starting (32-bit host process)\n");
    printf("[TesseraHost] Build: %s %s\n", __DATE__, __TIME__);
    printf("[TesseraHost] sizeof(void*) = %zu (must be 4)\n", sizeof(void*));

    if (sizeof(void*) != 4) {
        fprintf(stderr, "[ERROR] TesseraHost must be compiled as 32-bit!\n");
        return 1;
    }

    // Parse --port <N>
    int port = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);
    }
    if (port == 0) {
        fprintf(stderr, "Usage: TesseraHost.exe --port <port>\n");
        return 1;
    }

    TcpServer server;

    if (!server.init(port)) {
        fprintf(stderr, "[ERROR] Failed to initialize TCP server on port %d\n", port);
        return 1;
    }

    printf("[TesseraHost] Waiting for client connection...\n");
    if (!server.accept_client()) {
        fprintf(stderr, "[ERROR] Failed to accept client connection\n");
        server.shutdown();
        return 1;
    }

    // Command loop: recv → dispatch → repeat until shutdown.
    bool shutdown_requested = false;
    std::vector<char> msg_buf;

    printf("[TesseraHost] Entering command loop.\n");
    while (!shutdown_requested) {
        if (!server.recv_message(msg_buf)) {
            printf("[TesseraHost] Client disconnected or recv error — exiting.\n");
            break;
        }

        dispatch_command(msg_buf.data(), static_cast<uint32_t>(msg_buf.size()),
                         server, &shutdown_requested);
    }

    printf("[TesseraHost] Shutting down.\n");
    server.shutdown();
    return 0;
}
