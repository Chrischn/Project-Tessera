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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "tcp_server.h"
#include "message_protocol.h"
#include "dll_loader.h"

// Crash diagnostics from host_callbacks.cpp
extern int g_cbCallCount;
extern const char* g_cbLastName;
extern const char* g_cbLastXmlFile;

// Vectored exception handler — catches crashes even outside SEH blocks
static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        fprintf(stderr, "\n[CRASH] Access violation at 0x%08X\n",
            (unsigned)ep->ExceptionRecord->ExceptionAddress);
        fprintf(stderr, "[CRASH] Last callback #%d: %s\n", g_cbCallCount, g_cbLastName);
        fprintf(stderr, "[CRASH] Last XML file: %s\n", g_cbLastXmlFile);
        fprintf(stderr, "[CRASH] Fault address: 0x%08X (%s)\n",
            (unsigned)ep->ExceptionRecord->ExceptionInformation[1],
            ep->ExceptionRecord->ExceptionInformation[0] ? "write" : "read");
        fflush(stderr);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char* argv[]) {
    SetUnhandledExceptionFilter(CrashHandler);
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
    DllLoader dll_loader;

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
                         server, &shutdown_requested, dll_loader);
    }

    printf("[TesseraHost] Shutting down.\n");
    dll_loader.unload();
    server.shutdown();
    return 0;
}
