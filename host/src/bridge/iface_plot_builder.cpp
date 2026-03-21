// =============================================================================
// File:              iface_plot_builder.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLPlotBuilderIFaceBase. All methods log and
//   return safe defaults. Real implementations replace stubs incrementally.
//
//   CvDLLPlotBuilderIFaceBase inherits from CvDLLEntityIFaceBase, so this impl
//   must provide ALL inherited entity methods as well as the plot-builder ones.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_plot_builder.h"
#include <cstdio>

class CvDLLPlotBuilderIFaceImpl : public CvDLLPlotBuilderIFaceBase
{
public:
	// =========================================================================
	// Inherited from CvDLLEntityIFaceBase
	// =========================================================================

	void removeEntity(CvEntity*) override {
		fprintf(stderr, "[PLOT STUB] removeEntity\n");
	}

	void addEntity(CvEntity*, uint uiEntAddFlags) override {
		fprintf(stderr, "[PLOT STUB] addEntity\n");
	}

	void setup(CvEntity*) override {
		fprintf(stderr, "[PLOT STUB] setup\n");
	}

	void setVisible(CvEntity*, bool) override {
		fprintf(stderr, "[PLOT STUB] setVisible\n");
	}

	void createCityEntity(CvCity*) override {
		fprintf(stderr, "[PLOT STUB] createCityEntity\n");
	}

	void createUnitEntity(CvUnit*) override {
		fprintf(stderr, "[PLOT STUB] createUnitEntity\n");
	}

	void destroyEntity(CvEntity*& pEntity, bool bSafeDelete) override {
		fprintf(stderr, "[PLOT STUB] destroyEntity\n");
		pEntity = nullptr;
	}

	void updatePosition(CvEntity* gameEntity) override {
		fprintf(stderr, "[PLOT STUB] updatePosition\n");
	}

	void setupFloodPlains(CvRiver* river) override {
		fprintf(stderr, "[PLOT STUB] setupFloodPlains\n");
	}

	bool IsSelected(const CvEntity*) const override {
		fprintf(stderr, "[PLOT STUB] IsSelected\n");
		return false;
	}

	void PlayAnimation(CvEntity*, AnimationTypes eAnim, float fSpeed, bool bQueue, int iLayer, float fStartPct, float fEndPct) override {
		fprintf(stderr, "[PLOT STUB] PlayAnimation\n");
	}

	void StopAnimation(CvEntity*, AnimationTypes eAnim) override {
		fprintf(stderr, "[PLOT STUB] StopAnimation(anim)\n");
	}

	void StopAnimation(CvEntity*) override {
		fprintf(stderr, "[PLOT STUB] StopAnimation(all)\n");
	}

	void NotifyEntity(CvUnitEntity*, MissionTypes eMission) override {
		fprintf(stderr, "[PLOT STUB] NotifyEntity\n");
	}

	void MoveTo(CvUnitEntity*, const CvPlot* pkPlot) override {
		fprintf(stderr, "[PLOT STUB] MoveTo\n");
	}

	void QueueMove(CvUnitEntity*, const CvPlot* pkPlot) override {
		fprintf(stderr, "[PLOT STUB] QueueMove\n");
	}

	void ExecuteMove(CvUnitEntity*, float fTimeToExecute, bool bCombat) override {
		fprintf(stderr, "[PLOT STUB] ExecuteMove\n");
	}

	void SetPosition(CvUnitEntity* pEntity, const CvPlot* pkPlot) override {
		fprintf(stderr, "[PLOT STUB] SetPosition\n");
	}

	void AddMission(const CvMissionDefinition* pDefinition) override {
		fprintf(stderr, "[PLOT STUB] AddMission\n");
	}

	void RemoveUnitFromBattle(CvUnit* pUnit) override {
		fprintf(stderr, "[PLOT STUB] RemoveUnitFromBattle\n");
	}

	void showPromotionGlow(CvUnitEntity* pEntity, bool show) override {
		fprintf(stderr, "[PLOT STUB] showPromotionGlow\n");
	}

	void updateEnemyGlow(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[PLOT STUB] updateEnemyGlow\n");
	}

	void updatePromotionLayers(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[PLOT STUB] updatePromotionLayers\n");
	}

	void updateGraphicEra(CvUnitEntity* pEntity, EraTypes eOldEra) override {
		fprintf(stderr, "[PLOT STUB] updateGraphicEra\n");
	}

	void SetSiegeTower(CvUnitEntity* pEntity, bool show) override {
		fprintf(stderr, "[PLOT STUB] SetSiegeTower\n");
	}

	bool GetSiegeTower(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[PLOT STUB] GetSiegeTower\n");
		return false;
	}

	// =========================================================================
	// PlotBuilder-specific methods
	// =========================================================================

	void init(CvPlotBuilder*, CvPlot*) override {
		fprintf(stderr, "[PLOT STUB] init\n");
	}

	CvPlotBuilder* create() override {
		fprintf(stderr, "[PLOT STUB] create\n");
		return nullptr;
	}

	void destroy(CvPlotBuilder*& pPlotBuilder, bool bSafeDelete) override {
		fprintf(stderr, "[PLOT STUB] destroy\n");
		pPlotBuilder = nullptr;
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLPlotBuilderIFaceImpl g_plotBuilderIFaceInstance;
CvDLLPlotBuilderIFaceBase* g_pPlotBuilderIFace = &g_plotBuilderIFaceInstance;
