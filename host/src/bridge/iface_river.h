// =============================================================================
// File:              iface_river.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLRiverIFaceBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
//   In the SDK, this class is defined in CvDLLSymbolIFaceBase.h.
//   The "derived methods" (destroy, Hide, IsHidden, updatePosition) delegate
//   to CvDLLRouteIFaceBase via gDLL but still occupy vtable slots.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLRiverIFaceBase
{
public:
	virtual CvRiver* createRiver() = 0;
	virtual void init(CvRiver*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;

	// derived methods (delegate to RouteIFace in the SDK, but still virtual vtable slots)
	virtual void destroy(CvRiver*& pObj, bool bSafeDelete = true) = 0;
	virtual void Hide(CvRiver* pObj, bool bHide) = 0;
	virtual bool IsHidden(CvRiver* pObj) = 0;
	virtual void updatePosition(CvRiver* pObj) = 0;
};
