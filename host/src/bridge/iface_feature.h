// =============================================================================
// File:              iface_feature.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLFeatureIFaceBase definition. Method order matches SDK
//   vtable layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
//   In the SDK, this class is defined in CvDLLSymbolIFaceBase.h alongside
//   CvDLLSymbolIFaceBase, CvDLLRouteIFaceBase, and CvDLLRiverIFaceBase.
//   We split them into one class per file for clarity.
//
//   The SDK has "derived methods" (destroy, Hide, IsHidden, updatePosition)
//   that delegate to CvDLLSymbolIFaceBase via gDLL. These are virtual and
//   occupy vtable slots, so they must appear in the correct order. For our
//   clean-room interface they are declared pure virtual.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLFeatureIFaceBase
{
public:
	virtual CvFeature* createFeature() = 0;
	virtual void init(CvFeature*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
	virtual FeatureTypes getFeature(CvFeature* pObj) = 0;
	virtual void setDummyVisibility(CvFeature* feature, const char* dummyTag, bool show) = 0;
	virtual void addDummyModel(CvFeature* feature, const char* dummyTag, const char* modelTag) = 0;
	virtual void setDummyTexture(CvFeature* feature, const char* dummyTag, const char* textureTag) = 0;
	virtual CvString pickDummyTag(CvFeature* feature, int mouseX, int mouseY) = 0;
	virtual void resetModel(CvFeature* feature) = 0;

	// derived methods (delegate to SymbolIFace in the SDK, but still virtual vtable slots)
	virtual void destroy(CvFeature*& pObj, bool bSafeDelete = true) = 0;
	virtual void Hide(CvFeature* pObj, bool bHide) = 0;
	virtual bool IsHidden(CvFeature* pObj) = 0;
	virtual void updatePosition(CvFeature* pObj) = 0;
};
