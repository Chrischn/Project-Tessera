// =============================================================================
// File:              iface_route.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLRouteIFaceBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
//   In the SDK, this class is defined in CvDLLSymbolIFaceBase.h.
//   The "derived methods" (destroy, Hide, IsHidden, updatePosition) delegate
//   to CvDLLSymbolIFaceBase via gDLL but still occupy vtable slots.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLRouteIFaceBase
{
public:
	virtual CvRoute* createRoute() = 0;
	virtual void init(CvRoute*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
	virtual RouteTypes getRoute(CvRoute* pObj) = 0;

	// derived methods (delegate to SymbolIFace in the SDK, but still virtual vtable slots)
	virtual void destroy(CvRoute*& pObj, bool bSafeDelete = true) = 0;
	virtual void Hide(CvRoute* pObj, bool bHide) = 0;
	virtual bool IsHidden(CvRoute* pObj) = 0;
	virtual void updatePosition(CvRoute* pObj) = 0;
	virtual int getConnectionMask(CvRoute* pObj) = 0;
	virtual void updateGraphicEra(CvRoute* pObj) = 0;
};
