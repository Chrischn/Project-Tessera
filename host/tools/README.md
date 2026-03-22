# TesseraHost Debug Tools

## Python Test Clients

- **debug_init_client.py** — Sends `init` + `shutdown` to TesseraHost over TCP. Retries connection for CDB startup timing. Usage: `python debug_init_client.py [port] [base_path]`
- **test_client.py** — Basic TCP test client.

## CDB Debugger Scripts

Used with CDB (command-line WinDbg) via `-cf` flag:

```bash
CDB="/c/Program Files (x86)/Windows Kits/10/Debuggers/x86/cdb.exe"
"$CDB" -cf tools/<script>.txt -logo output.log build/Debug/TesseraHost.exe --port 12345
```

| Script | Purpose | When to use |
|--------|---------|-------------|
| `cdb_init.txt` | Break on AV, dump registers + stack, pass to SEH | General crash debugging |
| `cdb_msgbox.txt` | Break on `int 3` from relay MessageBox stub | Catching "Allocating zero" errors |
| `cdb_heap_corruption.txt` | Catch `STATUS_HEAP_CORRUPTION` (0xC0000374) | Heap corruption analysis |
| `cdb_pageheap_first_av.txt` | Stop on first AV with full stack dump | Page heap overflow detection |
| `cdb_pageheap_detail.txt` | First AV + `!heap -p -a` block info + disassembly | Identifying exact overflowed allocation |
| `cdb_alloczero.txt` | Break near "Allocating zero" with deep stack dump | Legacy — used for early debugging |
| `cdb_crash288k.txt` | Break near callback #288497 crash point | Legacy — from earlier session |

### CDB flags reference

| Flag | Effect |
|------|--------|
| `-cf script.txt` | Run commands from file at startup |
| `-logo file.log` | Log all CDB output to file |
| `-hd` | Disable debug heap (use normal heap — needed to reproduce standalone crashes) |

### Page heap (requires admin)

```
gflags.exe /p /enable TesseraHost.exe /full   # Enable — guard page after every allocation
gflags.exe /p /disable TesseraHost.exe         # Disable when done
```

Use with `cdb_pageheap_detail.txt` to catch the exact instruction that overflows a heap buffer.
