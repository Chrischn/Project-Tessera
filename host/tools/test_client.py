#!/usr/bin/env python3
"""Test client for TesseraHost TCP protocol."""
import socket
import struct
import json
import sys


def send_msg(sock, obj):
    data = json.dumps(obj).encode('utf-8')
    sock.sendall(struct.pack('<I', len(data)) + data)


def recv_msg(sock):
    raw_len = sock.recv(4)
    if not raw_len:
        return None
    length = struct.unpack('<I', raw_len)[0]
    data = b''
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk:
            return None
        data += chunk
    return json.loads(data.decode('utf-8'))


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 12345
    print(f"[test_client] Connecting to 127.0.0.1:{port}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', port))
    print("[test_client] Connected.")

    # Test: ping
    print("[test_client] Sending ping...")
    send_msg(sock, {"cmd": "ping"})
    resp = recv_msg(sock)
    print(f"  ping -> {resp}")
    assert resp is not None, "No response to ping"
    assert resp["status"] == "ok", f"Expected status=ok, got {resp}"
    print("  [PASS] ping returned status=ok")

    # Test: unknown command
    print("[test_client] Sending unknown command...")
    send_msg(sock, {"cmd": "frobnicate"})
    resp = recv_msg(sock)
    print(f"  frobnicate -> {resp}")
    assert resp is not None, "No response to unknown command"
    assert resp["status"] == "error", f"Expected status=error, got {resp}"
    assert "unknown command" in resp.get("message", ""), f"Unexpected message: {resp}"
    print("  [PASS] unknown command returned status=error")

    # Test: shutdown
    print("[test_client] Sending shutdown...")
    send_msg(sock, {"cmd": "shutdown"})
    resp = recv_msg(sock)
    print(f"  shutdown -> {resp}")
    assert resp is not None, "No response to shutdown"
    assert resp["status"] == "ok", f"Expected status=ok, got {resp}"
    print("  [PASS] shutdown returned status=ok")

    sock.close()
    print("[test_client] All tests passed.")


if __name__ == "__main__":
    main()
