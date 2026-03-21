// =============================================================================
// File:              tcp_server.h
// Author(s):         Chrischn89
// Description:
//   Winsock2 TCP server providing length-prefixed message framing for the
//   TesseraHost bridge protocol. Accepts a single client connection and
//   supports blocking send/recv of uint32_t LE length-prefixed payloads.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include <winsock2.h>
#include <vector>
#include <cstdint>

class TcpServer {
public:
    TcpServer();
    ~TcpServer();

    // Initialize Winsock, create socket, bind to 127.0.0.1:port, start listening.
    // Returns false on failure (error printed to stderr).
    bool init(int port);

    // Block until one client connects. Returns false on failure.
    bool accept_client();

    // Send a length-prefixed message: [4 bytes LE uint32 length][payload].
    // Returns false on send failure.
    bool send_message(const char* data, uint32_t len);

    // Receive a length-prefixed message. Resizes out to the payload size.
    // Returns false if the connection is closed or an error occurs.
    bool recv_message(std::vector<char>& out);

    // Close all sockets and call WSACleanup.
    void shutdown();

private:
    SOCKET listen_sock_;
    SOCKET client_sock_;
    bool   wsa_initialized_;

    // Helper: recv exactly n bytes into buf. Returns false if connection closed or error.
    bool recv_exact(char* buf, int n);
};
