#!/usr/bin/env python3
"""Debug client: sends init + shutdown to TesseraHost for cdb debugging."""
import socket
import struct
import json
import sys
import time


def send_msg(sock, obj):
    data = json.dumps(obj).encode('utf-8')
    sock.sendall(struct.pack('<I', len(data)) + data)


def recv_msg(sock, timeout=300):
    sock.settimeout(timeout)
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
    base_path = sys.argv[2] if len(sys.argv) > 2 else r"E:\Programming\Civ4"
    mod_name = sys.argv[3] if len(sys.argv) > 3 else ""

    # Retry connection — cdb needs a moment to start the process
    for attempt in range(10):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(('127.0.0.1', port))
            print(f"[debug_client] Connected to 127.0.0.1:{port}")
            break
        except ConnectionRefusedError:
            print(f"[debug_client] Connection refused, retry {attempt+1}/10...")
            sock.close()
            time.sleep(2)
    else:
        print("[debug_client] Failed to connect after 10 retries")
        sys.exit(1)

    # Send init command
    init_cmd = {"cmd": "init", "base_path": base_path}
    if mod_name:
        init_cmd["mod"] = mod_name
    print(f"[debug_client] Sending init (base_path={base_path}, mod={mod_name or 'none'})...")
    send_msg(sock, init_cmd)

    # Wait for response (XML loading takes a while — 285k+ callbacks)
    print("[debug_client] Waiting for init response (up to 5 min)...")
    resp = recv_msg(sock, timeout=300)
    print(f"[debug_client] init -> {resp}")

    if resp and resp.get("status") == "ok":
        # Send pytest command — PoC for Python 2.4 embedding + data extraction
        print("[debug_client] Sending pytest (Python data extraction PoC)...")
        send_msg(sock, {"cmd": "pytest"})
        resp = recv_msg(sock, timeout=60)
        print(f"[debug_client] pytest -> {resp}")

        if resp and resp.get("status") == "ok":
            # Test get_all_infos for tech
            print("[debug_client] Sending get_all_infos (tech)...")
            send_msg(sock, {"cmd": "get_all_infos", "type": "tech"})
            resp = recv_msg(sock, timeout=60)
            data = resp.get("data", []) if resp else []
            print(f"[debug_client] get_all_infos(tech) -> {len(data)} items")
            if data:
                print(f"[debug_client]   first: {data[0]}")

            # Test remaining info types
            for info_type in ["building", "unit", "promotion", "bonus", "civilization", "era"]:
                print(f"[debug_client] Sending get_all_infos ({info_type})...")
                send_msg(sock, {"cmd": "get_all_infos", "type": info_type})
                resp = recv_msg(sock, timeout=60)
                data = resp.get("data", []) if resp else []
                print(f"[debug_client] get_all_infos({info_type}) -> {len(data)} items")
                if data:
                    print(f"[debug_client]   first: {data[0]}")

    # Send shutdown
    print("[debug_client] Sending shutdown...")
    send_msg(sock, {"cmd": "shutdown"})
    resp = recv_msg(sock, timeout=30)
    print(f"[debug_client] shutdown -> {resp}")

    sock.close()
    print("[debug_client] Done.")


if __name__ == "__main__":
    main()
