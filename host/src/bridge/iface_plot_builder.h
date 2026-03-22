// =============================================================================
// File:              iface_plot_builder.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLPlotBuilderIFaceBase definition. Method order matches SDK
//   vtable layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
//   Inherits from CvDLLEntityIFaceBase (same as in the SDK).
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_entity.h"

class CvDLLPlotBuilderIFaceBase : public CvDLLEntityIFaceBase
{
public:
	virtual void init(CvPlotBuilder*, CvPlot*) = 0;
	virtual CvPlotBuilder* create() = 0;

	// derived method (delegates to EntityIFace::destroyEntity in the SDK, but still a vtable slot)
	virtual void destroy(CvPlotBuilder*& pPlotBuilder, bool bSafeDelete = true) = 0;
};
