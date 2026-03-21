// =============================================================================
// File:              iface_entity.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLEntityIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_entity.h"
#include <cstdio>

class CvDLLEntityIFaceImpl : public CvDLLEntityIFaceBase
{
public:
	// =========================================================================
	// Entity lifecycle
	// =========================================================================

	void removeEntity(CvEntity*) override {
		fprintf(stderr, "[ENTITY STUB] removeEntity\n");
	}

	void addEntity(CvEntity*, uint uiEntAddFlags) override {
		fprintf(stderr, "[ENTITY STUB] addEntity\n");
	}

	void setup(CvEntity*) override {
		fprintf(stderr, "[ENTITY STUB] setup\n");
	}

	void setVisible(CvEntity*, bool) override {
		fprintf(stderr, "[ENTITY STUB] setVisible\n");
	}

	void createCityEntity(CvCity*) override {
		fprintf(stderr, "[ENTITY STUB] createCityEntity\n");
	}

	void createUnitEntity(CvUnit*) override {
		fprintf(stderr, "[ENTITY STUB] createUnitEntity\n");
	}

	void destroyEntity(CvEntity*& pEntity, bool bSafeDelete) override {
		fprintf(stderr, "[ENTITY STUB] destroyEntity\n");
		pEntity = nullptr;
	}

	void updatePosition(CvEntity* gameEntity) override {
		fprintf(stderr, "[ENTITY STUB] updatePosition\n");
	}

	void setupFloodPlains(CvRiver* river) override {
		fprintf(stderr, "[ENTITY STUB] setupFloodPlains\n");
	}

	// =========================================================================
	// Selection / Animation
	// =========================================================================

	bool IsSelected(const CvEntity*) const override {
		fprintf(stderr, "[ENTITY STUB] IsSelected\n");
		return false;
	}

	void PlayAnimation(CvEntity*, AnimationTypes eAnim, float fSpeed, bool bQueue, int iLayer, float fStartPct, float fEndPct) override {
		fprintf(stderr, "[ENTITY STUB] PlayAnimation\n");
	}

	void StopAnimation(CvEntity*, AnimationTypes eAnim) override {
		fprintf(stderr, "[ENTITY STUB] StopAnimation(anim)\n");
	}

	void StopAnimation(CvEntity*) override {
		fprintf(stderr, "[ENTITY STUB] StopAnimation(all)\n");
	}

	// =========================================================================
	// Unit entity operations
	// =========================================================================

	void NotifyEntity(CvUnitEntity*, MissionTypes eMission) override {
		fprintf(stderr, "[ENTITY STUB] NotifyEntity\n");
	}

	void MoveTo(CvUnitEntity*, const CvPlot* pkPlot) override {
		fprintf(stderr, "[ENTITY STUB] MoveTo\n");
	}

	void QueueMove(CvUnitEntity*, const CvPlot* pkPlot) override {
		fprintf(stderr, "[ENTITY STUB] QueueMove\n");
	}

	void ExecuteMove(CvUnitEntity*, float fTimeToExecute, bool bCombat) override {
		fprintf(stderr, "[ENTITY STUB] ExecuteMove\n");
	}

	void SetPosition(CvUnitEntity* pEntity, const CvPlot* pkPlot) override {
		fprintf(stderr, "[ENTITY STUB] SetPosition\n");
	}

	void AddMission(const CvMissionDefinition* pDefinition) override {
		fprintf(stderr, "[ENTITY STUB] AddMission\n");
	}

	void RemoveUnitFromBattle(CvUnit* pUnit) override {
		fprintf(stderr, "[ENTITY STUB] RemoveUnitFromBattle\n");
	}

	void showPromotionGlow(CvUnitEntity* pEntity, bool show) override {
		fprintf(stderr, "[ENTITY STUB] showPromotionGlow\n");
	}

	void updateEnemyGlow(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[ENTITY STUB] updateEnemyGlow\n");
	}

	void updatePromotionLayers(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[ENTITY STUB] updatePromotionLayers\n");
	}

	void updateGraphicEra(CvUnitEntity* pEntity, EraTypes eOldEra) override {
		fprintf(stderr, "[ENTITY STUB] updateGraphicEra\n");
	}

	void SetSiegeTower(CvUnitEntity* pEntity, bool show) override {
		fprintf(stderr, "[ENTITY STUB] SetSiegeTower\n");
	}

	bool GetSiegeTower(CvUnitEntity* pEntity) override {
		fprintf(stderr, "[ENTITY STUB] GetSiegeTower\n");
		return false;
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLEntityIFaceImpl g_entityIFaceInstance;
CvDLLEntityIFaceBase* g_pEntityIFace = &g_entityIFaceInstance;
