// =============================================================================
// File:              iface_flag_entity.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLFlagEntityIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
//   CvDLLFlagEntityIFaceBase inherits from CvDLLEntityIFaceBase, so this impl
//   must provide ALL inherited entity methods as well as the flag-specific ones.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_flag_entity.h"
#include <cstdio>

class CvDLLFlagEntityIFaceImpl : public CvDLLFlagEntityIFaceBase
{
public:
	// =========================================================================
	// Inherited from CvDLLEntityIFaceBase
	// =========================================================================

	void removeEntity(CvEntity*) override {
		fprintf(stderr, "[FLAG STUB] removeEntity\n");
	}

	void addEntity(CvEntity*, uint uiEntAddFlags) override {
		fprintf(stderr, "[FLAG STUB] addEntity\n");
	}

	void setup(CvEntity*) override {
		fprintf(stderr, "[FLAG STUB] setup\n");
	}

	void setVisible(CvEntity*, bool) override {
		fprintf(stderr, "[FLAG STUB] setVisible(entity)\n");
	}

	void createCityEntity(CvCity*) override {
		fprintf(stderr, "[FLAG STUB] createCityEntity\n");
	}

	void createUnitEntity(CvUnit*) override {
		fprintf(stderr, "[FLAG STUB] createUnitEntity\n");
	}

	void destroyEntity(CvEntity*& pEntity, bool bSafeDelete) override {
		fprintf(stderr, "[FLAG STUB] destroyEntity\n");
		pEntity = nullptr;
	}

	void updatePosition(CvEntity* gameEntity) override {
		fprintf(stderr, "[FLAG STUB] updatePosition\n");
	}

	void setupFloodPlains(CvRiver* river) override {
		fprintf(stderr, "[FLAG STUB] setupFloodPlains\n");
	}

	bool IsSelected(const CvEntity*) const override {
		fprintf(stderr, "[FLAG STUB] IsSelected\n");
		return false;
	}

	void PlayAnimation(CvEntity*, AnimationTypes eAnim, float fSpeed, bool bQueue, int iLayer, float fStartPct, float fEndPct) override {
		fprintf(stderr, "[FLAG STUB] PlayAnimation\n");
	}

	void StopAnimation(CvEntity*, AnimationTypes eAnim) override {
		fprintf(stderr, "[FLAG STUB] StopAnimation(anim)\n");
	}

	void StopAnimation(CvEntity*) override {
		fprintf(stderr, "[FLAG STUB] StopAnimation(all)\n");
	}

	void NotifyEntity(CvUnitEntity*, MissionTypes eMission) override {
		fprintf(stderr, "[FLAG STUB] NotifyEntity\n");
	}

	void MoveTo(CvUnitEntity*, const CvPlot* pkPlot) override {
		fprintf(stderr, "[FLAG STUB] MoveTo\n");
	}

	void QueueMove(CvUnitEntity*, const CvPlot* pkPlot) override {
		fprintf(stderr, "[FLAG STUB] QueueMove\n");
	}

	void ExecuteMove(CvUnitEntity*, float fTimeToExecute, bool bCombat) override {
		fprintf(stderr, "[FLAG STUB] ExecuteMove\n");
	}

	void SetPosition(CvUnitEntity* pEntity, const CvPlot* pkPlot) override {
		fprintf(stderr, "[FLAG STUB] SetPosition\n");
	}

	void AddMission(const CvMissionDefinition* pDefinition) override {
		fprintf(stderr, "[FLAG STUB] AddMission\n");
	}

	void RemoveUnitFromBattle(CvUnit* pUnit) override {
		fprintf(stderr, "[FLAG STUB] RemoveUnitFromBattle\n");
	}

	void showPromotionGlow(CvUnitEntity* pEntity, bool show) override {
		fprintf(stderr, "[FLAG STUB] showPromotionGlow\n");
	}

	void updateEnemyGlow(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[FLAG STUB] updateEnemyGlow\n");
	}

	void updatePromotionLayers(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[FLAG STUB] updatePromotionLayers\n");
	}

	void updateGraphicEra(CvUnitEntity* pEntity, EraTypes eOldEra) override {
		fprintf(stderr, "[FLAG STUB] updateGraphicEra(unit)\n");
	}

	void SetSiegeTower(CvUnitEntity* pEntity, bool show) override {
		fprintf(stderr, "[FLAG STUB] SetSiegeTower\n");
	}

	bool GetSiegeTower(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[FLAG STUB] GetSiegeTower\n");
		return false;
	}

	// =========================================================================
	// Flag-specific methods
	// =========================================================================

	CvFlagEntity* create(PlayerTypes ePlayer) override {
		fprintf(stderr, "[FLAG STUB] create\n");
		return nullptr;
	}

	PlayerTypes getPlayer(CvFlagEntity* pkFlag) const override {
		fprintf(stderr, "[FLAG STUB] getPlayer\n");
		return NO_PLAYER;
	}

	CvPlot* getPlot(CvFlagEntity* pkFlag) const override {
		fprintf(stderr, "[FLAG STUB] getPlot\n");
		return nullptr;
	}

	void setPlot(CvFlagEntity* pkFlag, CvPlot* pkPlot, bool bOffset) override {
		fprintf(stderr, "[FLAG STUB] setPlot\n");
	}

	void updateUnitInfo(CvFlagEntity* pkFlag, const CvPlot* pkPlot, bool bOffset) override {
		fprintf(stderr, "[FLAG STUB] updateUnitInfo\n");
	}

	void updateGraphicEra(CvFlagEntity* pkFlag) override {
		fprintf(stderr, "[FLAG STUB] updateGraphicEra(flag)\n");
	}

	void setVisible(CvFlagEntity* pEnt, bool bVis) override {
		fprintf(stderr, "[FLAG STUB] setVisible(flag)\n");
	}

	void destroy(CvFlagEntity*& pImp, bool bSafeDelete) override {
		fprintf(stderr, "[FLAG STUB] destroy(flag)\n");
		pImp = nullptr;
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLFlagEntityIFaceImpl g_flagEntityIFaceInstance;
CvDLLFlagEntityIFaceBase* g_pFlagEntityIFace = &g_flagEntityIFaceInstance;
