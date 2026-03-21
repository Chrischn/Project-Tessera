// =============================================================================
// File:              iface_feature.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLFeatureIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_feature.h"
#include <cstdio>

class CvDLLFeatureIFaceImpl : public CvDLLFeatureIFaceBase
{
public:
	CvFeature* createFeature() override {
		fprintf(stderr, "[FEATURE STUB] createFeature\n");
		return nullptr;
	}

	void init(CvFeature*, int iID, int iOffset, int iType, CvPlot* pPlot) override {
		fprintf(stderr, "[FEATURE STUB] init\n");
	}

	FeatureTypes getFeature(CvFeature* pObj) override {
		fprintf(stderr, "[FEATURE STUB] getFeature\n");
		return NO_FEATURE;
	}

	void setDummyVisibility(CvFeature* feature, const char* dummyTag, bool show) override {
		fprintf(stderr, "[FEATURE STUB] setDummyVisibility\n");
	}

	void addDummyModel(CvFeature* feature, const char* dummyTag, const char* modelTag) override {
		fprintf(stderr, "[FEATURE STUB] addDummyModel\n");
	}

	void setDummyTexture(CvFeature* feature, const char* dummyTag, const char* textureTag) override {
		fprintf(stderr, "[FEATURE STUB] setDummyTexture\n");
	}

	CvString pickDummyTag(CvFeature* feature, int mouseX, int mouseY) override {
		fprintf(stderr, "[FEATURE STUB] pickDummyTag\n");
		return "";
	}

	void resetModel(CvFeature* feature) override {
		fprintf(stderr, "[FEATURE STUB] resetModel\n");
	}

	// =========================================================================
	// Derived methods (delegate to SymbolIFace in the SDK)
	// =========================================================================

	void destroy(CvFeature*& pObj, bool bSafeDelete) override {
		fprintf(stderr, "[FEATURE STUB] destroy\n");
		pObj = nullptr;
	}

	void Hide(CvFeature* pObj, bool bHide) override {
		fprintf(stderr, "[FEATURE STUB] Hide\n");
	}

	bool IsHidden(CvFeature* pObj) override {
		fprintf(stderr, "[FEATURE STUB] IsHidden\n");
		return false;
	}

	void updatePosition(CvFeature* pObj) override {
		fprintf(stderr, "[FEATURE STUB] updatePosition\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLFeatureIFaceImpl g_featureIFaceInstance;
CvDLLFeatureIFaceBase* g_pFeatureIFace = &g_featureIFaceInstance;
