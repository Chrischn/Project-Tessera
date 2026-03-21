// =============================================================================
// File:              iface_fastar.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLFAStarIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_fastar.h"
#include <cstdio>

class CvDLLFAStarIFaceImpl : public CvDLLFAStarIFaceBase
{
public:
	FAStar* create() override {
		fprintf(stderr, "[FASTAR STUB] create\n");
		return nullptr;
	}

	void destroy(FAStar*& ptr, bool bSafeDelete) override {
		fprintf(stderr, "[FASTAR STUB] destroy\n");
		ptr = nullptr;
	}

	bool GeneratePath(FAStar*, int iXstart, int iYstart, int iXdest, int iYdest, bool bCardinalOnly, int iInfo, bool bReuse) override {
		fprintf(stderr, "[FASTAR STUB] GeneratePath\n");
		return false;
	}

	void Initialize(FAStar*, int iColumns, int iRows, bool bWrapX, bool bWrapY, FAPointFunc DestValidFunc, FAHeuristic HeuristicFunc, FAStarFunc CostFunc, FAStarFunc ValidFunc, FAStarFunc NotifyChildFunc, FAStarFunc NotifyListFunc, void* pData) override {
		fprintf(stderr, "[FASTAR STUB] Initialize\n");
	}

	void SetData(FAStar*, const void* pData) override {
		fprintf(stderr, "[FASTAR STUB] SetData\n");
	}

	FAStarNode* GetLastNode(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] GetLastNode\n");
		return nullptr;
	}

	bool IsPathStart(FAStar*, int iX, int iY) override {
		fprintf(stderr, "[FASTAR STUB] IsPathStart\n");
		return false;
	}

	bool IsPathDest(FAStar*, int iX, int iY) override {
		fprintf(stderr, "[FASTAR STUB] IsPathDest\n");
		return false;
	}

	int GetStartX(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] GetStartX\n");
		return 0;
	}

	int GetStartY(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] GetStartY\n");
		return 0;
	}

	int GetDestX(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] GetDestX\n");
		return 0;
	}

	int GetDestY(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] GetDestY\n");
		return 0;
	}

	int GetInfo(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] GetInfo\n");
		return 0;
	}

	void ForceReset(FAStar*) override {
		fprintf(stderr, "[FASTAR STUB] ForceReset\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLFAStarIFaceImpl g_fastarIFaceInstance;
CvDLLFAStarIFaceBase* g_pFAStarIFace = &g_fastarIFaceInstance;
