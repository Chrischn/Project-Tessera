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

int main(int argc, char* argv[]) {
    printf("[TesseraHost] Starting (32-bit host process)\n");
    printf("[TesseraHost] Build: %s %s\n", __DATE__, __TIME__);
    printf("[TesseraHost] sizeof(void*) = %zu (must be 4)\n", sizeof(void*));

    if (sizeof(void*) != 4) {
        fprintf(stderr, "[ERROR] TesseraHost must be compiled as 32-bit!\n");
        return 1;
    }

    printf("[TesseraHost] Ready.\n");
    return 0;
}
