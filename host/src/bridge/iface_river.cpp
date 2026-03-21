// =============================================================================
// File:              iface_river.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLRiverIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_river.h"
#include <cstdio>

class CvDLLRiverIFaceImpl : public CvDLLRiverIFaceBase
{
public:
	CvRiver* createRiver() override {
		fprintf(stderr, "[RIVER STUB] createRiver\n");
		return nullptr;
	}

	void init(CvRiver*, int iID, int iOffset, int iType, CvPlot* pPlot) override {
		fprintf(stderr, "[RIVER STUB] init\n");
	}

	// =========================================================================
	// Derived methods (delegate to RouteIFace in the SDK)
	// =========================================================================

	void destroy(CvRiver*& pObj, bool bSafeDelete) override {
		fprintf(stderr, "[RIVER STUB] destroy\n");
		pObj = nullptr;
	}

	void Hide(CvRiver* pObj, bool bHide) override {
		fprintf(stderr, "[RIVER STUB] Hide\n");
	}

	bool IsHidden(CvRiver* pObj) override {
		fprintf(stderr, "[RIVER STUB] IsHidden\n");
		return false;
	}

	void updatePosition(CvRiver* pObj) override {
		fprintf(stderr, "[RIVER STUB] updatePosition\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLRiverIFaceImpl g_riverIFaceInstance;
CvDLLRiverIFaceBase* g_pRiverIFace = &g_riverIFaceInstance;
