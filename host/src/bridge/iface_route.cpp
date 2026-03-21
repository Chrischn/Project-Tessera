// =============================================================================
// File:              iface_route.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLRouteIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_route.h"
#include <cstdio>

class CvDLLRouteIFaceImpl : public CvDLLRouteIFaceBase
{
public:
	CvRoute* createRoute() override {
		fprintf(stderr, "[ROUTE STUB] createRoute\n");
		return nullptr;
	}

	void init(CvRoute*, int iID, int iOffset, int iType, CvPlot* pPlot) override {
		fprintf(stderr, "[ROUTE STUB] init\n");
	}

	RouteTypes getRoute(CvRoute* pObj) override {
		fprintf(stderr, "[ROUTE STUB] getRoute\n");
		return NO_ROUTE;
	}

	// =========================================================================
	// Derived methods (delegate to SymbolIFace in the SDK)
	// =========================================================================

	void destroy(CvRoute*& pObj, bool bSafeDelete) override {
		fprintf(stderr, "[ROUTE STUB] destroy\n");
		pObj = nullptr;
	}

	void Hide(CvRoute* pObj, bool bHide) override {
		fprintf(stderr, "[ROUTE STUB] Hide\n");
	}

	bool IsHidden(CvRoute* pObj) override {
		fprintf(stderr, "[ROUTE STUB] IsHidden\n");
		return false;
	}

	void updatePosition(CvRoute* pObj) override {
		fprintf(stderr, "[ROUTE STUB] updatePosition\n");
	}

	int getConnectionMask(CvRoute* pObj) override {
		fprintf(stderr, "[ROUTE STUB] getConnectionMask\n");
		return 0;
	}

	void updateGraphicEra(CvRoute* pObj) override {
		fprintf(stderr, "[ROUTE STUB] updateGraphicEra\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLRouteIFaceImpl g_routeIFaceInstance;
CvDLLRouteIFaceBase* g_pRouteIFace = &g_routeIFaceInstance;
