// =============================================================================
// File:              iface_symbol.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLSymbolIFaceBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLSymbolIFaceBase
{
public:
	virtual void init(CvSymbol*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
	virtual CvSymbol* createSymbol() = 0;
	virtual void destroy(CvSymbol*&, bool bSafeDelete = true) = 0;
	virtual void setAlpha(CvSymbol*, float fAlpha) = 0;
	virtual void setScale(CvSymbol*, float fScale) = 0;
	virtual void Hide(CvSymbol*, bool bHide) = 0;
	virtual bool IsHidden(CvSymbol*) = 0;
	virtual void updatePosition(CvSymbol*) = 0;
	virtual int getID(CvSymbol*) = 0;
	virtual SymbolTypes getSymbol(CvSymbol* pSym) = 0;
	virtual void setTypeYield(CvSymbol*, int iType, int count) = 0;
};
