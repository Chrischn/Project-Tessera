// =============================================================================
// File:              iface_fastar.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLFAStarIFaceBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLFAStarIFaceBase
{
public:
	virtual FAStar* create() = 0;
	virtual void destroy(FAStar*& ptr, bool bSafeDelete = true) = 0;
	virtual bool GeneratePath(FAStar*, int iXstart, int iYstart, int iXdest, int iYdest, bool bCardinalOnly = false, int iInfo = 0, bool bReuse = false) = 0;
	virtual void Initialize(FAStar*, int iColumns, int iRows, bool bWrapX, bool bWrapY, FAPointFunc DestValidFunc, FAHeuristic HeuristicFunc, FAStarFunc CostFunc, FAStarFunc ValidFunc, FAStarFunc NotifyChildFunc, FAStarFunc NotifyListFunc, void* pData) = 0;
	virtual void SetData(FAStar*, const void* pData) = 0;
	virtual FAStarNode* GetLastNode(FAStar*) = 0;
	virtual bool IsPathStart(FAStar*, int iX, int iY) = 0;
	virtual bool IsPathDest(FAStar*, int iX, int iY) = 0;
	virtual int GetStartX(FAStar*) = 0;
	virtual int GetStartY(FAStar*) = 0;
	virtual int GetDestX(FAStar*) = 0;
	virtual int GetDestY(FAStar*) = 0;
	virtual int GetInfo(FAStar*) = 0;
	virtual void ForceReset(FAStar*) = 0;
};
