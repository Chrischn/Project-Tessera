/* =============================================================================
 * File:              crt_shim.c
 * Author(s):         Chrischn89
 * Description:
 *   Minimal CRT startup shim for TesseraRelay.dll. Provides symbols that are
 *   in the static CRT (libcmt.lib) but NOT exported by the dynamic CRT DLLs
 *   (msvcr71.dll/msvcp71.dll). This allows linking purely against the dynamic
 *   CRT, ensuring all allocations share the same heap as CvGameCoreDLL.dll.
 *
 * License:
 *   Released under the terms of the GNU General Public License version 3.0
 * ============================================================================= */

/* _fltused — compiler references this when floating-point code is generated */
int _fltused = 0x9875;

/* _load_config_used — referenced by the linker for safe SEH */
int _load_config_used = 0;

/* __chkstk — stack probe for functions with >4KB of local variables */
__declspec(naked) void __cdecl _chkstk(void) {
    __asm {
        push    ecx
        cmp     eax, 1000h
        lea     ecx, [esp+8]
        jb      short last_page
    probe_loop:
        sub     ecx, 1000h
        sub     eax, 1000h
        test    dword ptr [ecx], eax
        cmp     eax, 1000h
        jae     short probe_loop
    last_page:
        sub     ecx, eax
        test    dword ptr [ecx], eax
        mov     eax, esp
        mov     esp, ecx
        mov     ecx, dword ptr [eax]
        mov     eax, dword ptr [eax+4]
        push    eax
        ret
    }
}

/* __except_list — SEH exception registration chain pointer.
   Defined as an absolute symbol (EQU 0) in exsup.asm, NOT as a data variable.
   The compiler and CRT objects access it as fs:__except_list, which must
   resolve to fs:[0] (TEB offset 0 = SEH chain head). A data variable here
   would make fs:__except_list read from TEB+variable_address → crash.
   See exsup.asm for the correct definition. */

/* _DllMainCRTStartup — DLL entry point that initializes the CRT.
   We call the C++ global constructors stored in the .CRT section. */
typedef void (__cdecl *_PVFV)(void);

/* CRT initializer sections — compiler puts global C++ constructor pointers here */
#pragma data_seg(".CRT$XCA")
static _PVFV __xc_a[] = { 0 };
#pragma data_seg(".CRT$XCZ")
static _PVFV __xc_z[] = { 0 };
#pragma data_seg()

#pragma comment(linker, "/merge:.CRT=.rdata")

static void call_constructors(void) {
    _PVFV *p;
    for (p = __xc_a; p < __xc_z; p++) {
        if (*p) (*p)();
    }
}

/* __CxxThrowException — C++ throw implementation.
   msvcr71.dll exports this as _CxxThrowException but the compiler references
   it as __CxxThrowException@8 (stdcall decorated). Our import lib doesn't
   resolve the decorated name, so we provide a stub that calls abort().
   The relay never throws C++ exceptions. */
void __stdcall _CxxThrowException(void* pExceptionObject, void* pThrowInfo) {
    /* Should never be called. If it is, abort immediately. */
    extern void __cdecl abort(void);
    abort();
}

int __stdcall _DllMainCRTStartup(
    void* hDllHandle, unsigned long dwReason, void* lpreserved)
{
    if (dwReason == 1) { /* DLL_PROCESS_ATTACH */
        call_constructors();
    }
    return 1;
}
