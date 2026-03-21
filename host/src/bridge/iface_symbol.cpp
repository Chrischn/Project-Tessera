// =============================================================================
// File:              iface_symbol.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLSymbolIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_symbol.h"
#include <cstdio>

class CvDLLSymbolIFaceImpl : public CvDLLSymbolIFaceBase
{
public:
	void init(CvSymbol*, int iID, int iOffset, int iType, CvPlot* pPlot) override {
		fprintf(stderr, "[SYMBOL STUB] init\n");
	}

	CvSymbol* createSymbol() override {
		fprintf(stderr, "[SYMBOL STUB] createSymbol\n");
		return nullptr;
	}

	void destroy(CvSymbol*& pSym, bool bSafeDelete) override {
		fprintf(stderr, "[SYMBOL STUB] destroy\n");
		pSym = nullptr;
	}

	void setAlpha(CvSymbol*, float fAlpha) override {
		fprintf(stderr, "[SYMBOL STUB] setAlpha\n");
	}

	void setScale(CvSymbol*, float fScale) override {
		fprintf(stderr, "[SYMBOL STUB] setScale\n");
	}

	void Hide(CvSymbol*, bool bHide) override {
		fprintf(stderr, "[SYMBOL STUB] Hide\n");
	}

	bool IsHidden(CvSymbol*) override {
		fprintf(stderr, "[SYMBOL STUB] IsHidden\n");
		return false;
	}

	void updatePosition(CvSymbol*) override {
		fprintf(stderr, "[SYMBOL STUB] updatePosition\n");
	}

	int getID(CvSymbol*) override {
		fprintf(stderr, "[SYMBOL STUB] getID\n");
		return 0;
	}

	SymbolTypes getSymbol(CvSymbol* pSym) override {
		fprintf(stderr, "[SYMBOL STUB] getSymbol\n");
		return SymbolTypes{};
	}

	void setTypeYield(CvSymbol*, int iType, int count) override {
		fprintf(stderr, "[SYMBOL STUB] setTypeYield\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLSymbolIFaceImpl g_symbolIFaceInstance;
CvDLLSymbolIFaceBase* g_pSymbolIFace = &g_symbolIFaceInstance;
