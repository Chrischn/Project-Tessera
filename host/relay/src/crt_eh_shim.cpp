// =============================================================================
// File:              crt_eh_shim.cpp
// Author(s):         Chrischn89
// Description:
//   Provides the type_info vtable symbol required by RTTI and virtual classes.
//   In the full VS2003 CRT, this comes from typinfo.obj in libcmt.lib.
//   We provide our own minimal implementation to avoid linking against the
//   static CRT (which would mix CRT heaps and cause crashes).
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include <typeinfo>

// The type_info class is declared in <typeinfo> by the VS2003 CRT headers.
// Its vtable symbol (??_7type_info@@6B@) is referenced by compiler-generated
// RTTI data for any class with virtual methods. We need to instantiate it.
//
// The type_info destructor is exported by msvcr71.dll as ??1type_info@@UAE@XZ,
// so linking against msvcrt.lib resolves the actual destructor. We just need
// the vtable to exist as a linkable symbol.
//
// By defining a dummy function that constructs type_info-related data,
// we force the compiler to emit the vtable. However, type_info's constructor
// is private in MSVC, so we can't instantiate it directly.
//
// Instead, we use a linker trick: the vtable IS in msvcrt.lib (it must be,
// since msvcr71.dll exports type_info methods). Let's verify by checking
// if we can resolve it from the import lib without the static CRT.
//
// Actually, the vtable is NOT exported — it's a COMDAT section in typinfo.obj.
// We must provide it ourselves. The vtable for type_info contains just one
// entry: the virtual destructor.

// Provide the vtable by defining a dummy translation unit that forces
// the compiler to emit type_info's vtable. Since type_info has a virtual
// destructor declared in the CRT headers, we need the destructor symbol.
// msvcrt.lib provides ??1type_info@@UAE@XZ (the destructor body).
// We just need the vtable data structure.

// Force type_info vtable emission by taking address of a type_info object.
// typeid() on any polymorphic type returns a const type_info&, which
// requires the type_info vtable to exist.
//
// We define a minimal polymorphic class and use typeid on it.
namespace {
    struct ForceTypeInfoVtable {
        virtual ~ForceTypeInfoVtable() {}
    };

    // This function is never called — it just forces the compiler to emit
    // type_info RTTI structures including the vtable.
    void force_emit() {
        ForceTypeInfoVtable obj;
        (void)typeid(obj);
    }
}
