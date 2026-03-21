// =============================================================================
// File:              tcp_server.cpp
// Author(s):         Chrischn89
// Description:
//   Winsock2 TCP server providing length-prefixed message framing for the
//   TesseraHost bridge protocol. Accepts a single client connection and
//   supports blocking send/recv of uint32_t LE length-prefixed payloads.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#include "tcp_server.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

TcpServer::TcpServer()
    : listen_sock_(INVALID_SOCKET)
    , client_sock_(INVALID_SOCKET)
    , wsa_initialized_(false)
{}

TcpServer::~TcpServer() {
    shutdown();
}

bool TcpServer::init(int port) {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        fprintf(stderr, "[TcpServer] WSAStartup failed: %d\n", result);
        return false;
    }
    wsa_initialized_ = true;

    listen_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock_ == INVALID_SOCKET) {
        fprintf(stderr, "[TcpServer] socket() failed: %d\n", WSAGetLastError());
        return false;
    }

    // Allow address reuse so we can restart quickly during development.
    BOOL reuse = TRUE;
    setsockopt(listen_sock_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(static_cast<u_short>(port));
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (bind(listen_sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[TcpServer] bind() failed: %d\n", WSAGetLastError());
        return false;
    }

    if (listen(listen_sock_, 1) == SOCKET_ERROR) {
        fprintf(stderr, "[TcpServer] listen() failed: %d\n", WSAGetLastError());
        return false;
    }

    printf("[TcpServer] Listening on 127.0.0.1:%d\n", port);
    return true;
}

bool TcpServer::accept_client() {
    sockaddr_in client_addr{};
    int addr_len = sizeof(client_addr);
    client_sock_ = accept(listen_sock_,
                          reinterpret_cast<sockaddr*>(&client_addr),
                          &addr_len);
    if (client_sock_ == INVALID_SOCKET) {
        fprintf(stderr, "[TcpServer] accept() failed: %d\n", WSAGetLastError());
        return false;
    }

    char ip_buf[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_buf, sizeof(ip_buf));
    printf("[TcpServer] Client connected from %s:%d\n",
           ip_buf, ntohs(client_addr.sin_port));
    return true;
}

bool TcpServer::send_message(const char* data, uint32_t len) {
    if (client_sock_ == INVALID_SOCKET) return false;

    // Send 4-byte LE length prefix.
    uint32_t le_len = len; // x86 is always little-endian, no swap needed
    if (send(client_sock_, reinterpret_cast<const char*>(&le_len), 4, 0) != 4) {
        fprintf(stderr, "[TcpServer] send() length prefix failed: %d\n", WSAGetLastError());
        return false;
    }

    // Send payload in full.
    int total_sent = 0;
    while (total_sent < static_cast<int>(len)) {
        int sent = send(client_sock_, data + total_sent,
                        static_cast<int>(len) - total_sent, 0);
        if (sent == SOCKET_ERROR) {
            fprintf(stderr, "[TcpServer] send() payload failed: %d\n", WSAGetLastError());
            return false;
        }
        total_sent += sent;
    }
    return true;
}

bool TcpServer::recv_exact(char* buf, int n) {
    int total = 0;
    while (total < n) {
        int received = recv(client_sock_, buf + total, n - total, 0);
        if (received <= 0) {
            if (received == 0)
                fprintf(stderr, "[TcpServer] Connection closed by client.\n");
            else
                fprintf(stderr, "[TcpServer] recv() failed: %d\n", WSAGetLastError());
            return false;
        }
        total += received;
    }
    return true;
}

bool TcpServer::recv_message(std::vector<char>& out) {
    if (client_sock_ == INVALID_SOCKET) return false;

    // Read 4-byte LE length prefix.
    uint32_t le_len = 0;
    if (!recv_exact(reinterpret_cast<char*>(&le_len), 4))
        return false;

    // le_len is already host-order on x86 (LE architecture).
    if (le_len == 0) {
        out.clear();
        return true;
    }

    out.resize(le_len);
    if (!recv_exact(out.data(), static_cast<int>(le_len)))
        return false;

    return true;
}

void TcpServer::shutdown() {
    if (client_sock_ != INVALID_SOCKET) {
        closesocket(client_sock_);
        client_sock_ = INVALID_SOCKET;
    }
    if (listen_sock_ != INVALID_SOCKET) {
        closesocket(listen_sock_);
        listen_sock_ = INVALID_SOCKET;
    }
    if (wsa_initialized_) {
        WSACleanup();
        wsa_initialized_ = false;
    }
}
