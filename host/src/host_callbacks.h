// =============================================================================
// File:              host_callbacks.h
// Author(s):         Chrischn89
// Description:
//   Creates and returns a populated HostCallbacks struct. The callbacks wrap
//   the host's pugixml XML logic and file enumeration logic as plain C
//   functions that the VS2003 relay can call safely.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "relay_api.h"

// Create a fully populated HostCallbacks struct.
// All function pointers are set to host-side implementations.
HostCallbacks create_host_callbacks();
