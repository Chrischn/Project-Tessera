// =============================================================================
// File:              iface_flag_entity.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLFlagEntityIFaceBase definition. Method order matches SDK
//   vtable layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
//   Inherits from CvDLLEntityIFaceBase (same as in the SDK).
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_entity.h"

class CvDLLFlagEntityIFaceBase : public CvDLLEntityIFaceBase
{
public:
	virtual CvFlagEntity* create(PlayerTypes ePlayer) = 0;

	virtual PlayerTypes getPlayer(CvFlagEntity* pkFlag) const = 0;
	virtual CvPlot* getPlot(CvFlagEntity* pkFlag) const = 0;
	virtual void setPlot(CvFlagEntity* pkFlag, CvPlot* pkPlot, bool bOffset) = 0;
	virtual void updateUnitInfo(CvFlagEntity* pkFlag, const CvPlot* pkPlot, bool bOffset) = 0;
	virtual void updateGraphicEra(CvFlagEntity* pkFlag) = 0;
	virtual void setVisible(CvFlagEntity* pEnt, bool bVis) = 0;
	virtual void destroy(CvFlagEntity*& pImp, bool bSafeDelete = true) = 0;
};
