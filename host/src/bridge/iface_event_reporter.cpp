// =============================================================================
// File:              iface_event_reporter.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLEventReporterIFaceBase. All methods log and
//   return safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_event_reporter.h"
#include <cstdio>

class CvDLLEventReporterIFaceImpl : public CvDLLEventReporterIFaceBase
{
public:
	void genericEvent(const char* szEventName, void* pythonArgs) override {
		fprintf(stderr, "[EVENT STUB] genericEvent: %s\n", szEventName ? szEventName : "(null)");
	}

	void mouseEvent(int evt, const POINT& ptCursor) override {
		fprintf(stderr, "[EVENT STUB] mouseEvent\n");
	}

	void kbdEvent(int evt, int key) override {
		fprintf(stderr, "[EVENT STUB] kbdEvent\n");
	}

	void gameEnd() override {
		fprintf(stderr, "[EVENT STUB] gameEnd\n");
	}

	void beginGameTurn(int iGameTurn) override {
		fprintf(stderr, "[EVENT STUB] beginGameTurn(%d)\n", iGameTurn);
	}

	void endGameTurn(int iGameTurn) override {
		fprintf(stderr, "[EVENT STUB] endGameTurn(%d)\n", iGameTurn);
	}

	void beginPlayerTurn(int iGameTurn, PlayerTypes) override {
		fprintf(stderr, "[EVENT STUB] beginPlayerTurn\n");
	}

	void endPlayerTurn(int iGameTurn, PlayerTypes) override {
		fprintf(stderr, "[EVENT STUB] endPlayerTurn\n");
	}

	void firstContact(TeamTypes eTeamID1, TeamTypes eTeamID2) override {
		fprintf(stderr, "[EVENT STUB] firstContact\n");
	}

	void combatResult(CvUnit* pWinner, CvUnit* pLoser) override {
		fprintf(stderr, "[EVENT STUB] combatResult\n");
	}

	void improvementBuilt(int iImprovementType, int iX, int iY) override {
		fprintf(stderr, "[EVENT STUB] improvementBuilt\n");
	}

	void improvementDestroyed(int iImprovementType, int iPlayer, int iX, int iY) override {
		fprintf(stderr, "[EVENT STUB] improvementDestroyed\n");
	}

	void routeBuilt(int RouteType, int iX, int iY) override {
		fprintf(stderr, "[EVENT STUB] routeBuilt\n");
	}

	void plotRevealed(CvPlot* pPlot, TeamTypes eTeam) override {
		fprintf(stderr, "[EVENT STUB] plotRevealed\n");
	}

	void plotFeatureRemoved(CvPlot* pPlot, FeatureTypes eFeature, CvCity* pCity) override {
		fprintf(stderr, "[EVENT STUB] plotFeatureRemoved\n");
	}

	void plotPicked(CvPlot* pPlot) override {
		fprintf(stderr, "[EVENT STUB] plotPicked\n");
	}

	void nukeExplosion(CvPlot* pPlot, CvUnit* pNukeUnit) override {
		fprintf(stderr, "[EVENT STUB] nukeExplosion\n");
	}

	void gotoPlotSet(CvPlot* pPlot, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] gotoPlotSet\n");
	}

	void cityBuilt(CvCity* pCity) override {
		fprintf(stderr, "[EVENT STUB] cityBuilt\n");
	}

	void cityRazed(CvCity* pCity, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] cityRazed\n");
	}

	void cityAcquired(PlayerTypes eOldOwner, PlayerTypes ePlayer, CvCity* pCity, bool bConquest, bool bTrade) override {
		fprintf(stderr, "[EVENT STUB] cityAcquired\n");
	}

	void cityAcquiredAndKept(PlayerTypes ePlayer, CvCity* pCity) override {
		fprintf(stderr, "[EVENT STUB] cityAcquiredAndKept\n");
	}

	void cityLost(CvCity* pCity) override {
		fprintf(stderr, "[EVENT STUB] cityLost\n");
	}

	void cultureExpansion(CvCity* pCity, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] cultureExpansion\n");
	}

	void cityGrowth(CvCity* pCity, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] cityGrowth\n");
	}

	void cityDoTurn(CvCity* pCity, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] cityDoTurn\n");
	}

	void cityBuildingUnit(CvCity* pCity, UnitTypes eUnitType) override {
		fprintf(stderr, "[EVENT STUB] cityBuildingUnit\n");
	}

	void cityBuildingBuilding(CvCity* pCity, BuildingTypes eBuildingType) override {
		fprintf(stderr, "[EVENT STUB] cityBuildingBuilding\n");
	}

	void cityRename(CvCity* pCity) override {
		fprintf(stderr, "[EVENT STUB] cityRename\n");
	}

	void cityHurry(CvCity* pCity, HurryTypes eHurry) override {
		fprintf(stderr, "[EVENT STUB] cityHurry\n");
	}

	void selectionGroupPushMission(CvSelectionGroup* pSelectionGroup, MissionTypes eMission) override {
		fprintf(stderr, "[EVENT STUB] selectionGroupPushMission\n");
	}

	void unitMove(CvPlot* pPlot, CvUnit* pUnit, CvPlot* pOldPlot) override {
		fprintf(stderr, "[EVENT STUB] unitMove\n");
	}

	void unitSetXY(CvPlot* pPlot, CvUnit* pUnit) override {
		fprintf(stderr, "[EVENT STUB] unitSetXY\n");
	}

	void unitCreated(CvUnit* pUnit) override {
		fprintf(stderr, "[EVENT STUB] unitCreated\n");
	}

	void unitBuilt(CvCity* pCity, CvUnit* pUnit) override {
		fprintf(stderr, "[EVENT STUB] unitBuilt\n");
	}

	void unitKilled(CvUnit* pUnit, PlayerTypes eAttacker) override {
		fprintf(stderr, "[EVENT STUB] unitKilled\n");
	}

	void unitLost(CvUnit* pUnit) override {
		fprintf(stderr, "[EVENT STUB] unitLost\n");
	}

	void unitPromoted(CvUnit* pUnit, PromotionTypes ePromotion) override {
		fprintf(stderr, "[EVENT STUB] unitPromoted\n");
	}

	void unitSelected(CvUnit* pUnit) override {
		fprintf(stderr, "[EVENT STUB] unitSelected\n");
	}

	void unitRename(CvUnit* pUnit) override {
		fprintf(stderr, "[EVENT STUB] unitRename\n");
	}

	void unitPillage(CvUnit* pUnit, ImprovementTypes eImprovement, RouteTypes eRoute, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] unitPillage\n");
	}

	void unitSpreadReligionAttempt(CvUnit* pUnit, ReligionTypes eReligion, bool bSuccess) override {
		fprintf(stderr, "[EVENT STUB] unitSpreadReligionAttempt\n");
	}

	void unitGifted(CvUnit* pUnit, PlayerTypes eGiftingPlayer, CvPlot* pPlotLocation) override {
		fprintf(stderr, "[EVENT STUB] unitGifted\n");
	}

	void unitBuildImprovement(CvUnit* pUnit, BuildTypes eBuild, bool bFinished) override {
		fprintf(stderr, "[EVENT STUB] unitBuildImprovement\n");
	}

	void goodyReceived(PlayerTypes ePlayer, CvPlot* pGoodyPlot, CvUnit* pGoodyUnit, GoodyTypes eGoodyType) override {
		fprintf(stderr, "[EVENT STUB] goodyReceived\n");
	}

	void greatPersonBorn(CvUnit* pUnit, PlayerTypes ePlayer, CvCity* pCity) override {
		fprintf(stderr, "[EVENT STUB] greatPersonBorn\n");
	}

	void buildingBuilt(CvCity* pCity, BuildingTypes eBuilding) override {
		fprintf(stderr, "[EVENT STUB] buildingBuilt\n");
	}

	void projectBuilt(CvCity* pCity, ProjectTypes eProject) override {
		fprintf(stderr, "[EVENT STUB] projectBuilt\n");
	}

	void techAcquired(TechTypes eType, TeamTypes eTeam, PlayerTypes ePlayer, bool bAnnounce) override {
		fprintf(stderr, "[EVENT STUB] techAcquired\n");
	}

	void techSelected(TechTypes eTech, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] techSelected\n");
	}

	void religionFounded(ReligionTypes eType, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] religionFounded\n");
	}

	void religionSpread(ReligionTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) override {
		fprintf(stderr, "[EVENT STUB] religionSpread\n");
	}

	void religionRemove(ReligionTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) override {
		fprintf(stderr, "[EVENT STUB] religionRemove\n");
	}

	void corporationFounded(CorporationTypes eType, PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] corporationFounded\n");
	}

	void corporationSpread(CorporationTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) override {
		fprintf(stderr, "[EVENT STUB] corporationSpread\n");
	}

	void corporationRemove(CorporationTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) override {
		fprintf(stderr, "[EVENT STUB] corporationRemove\n");
	}

	void goldenAge(PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] goldenAge\n");
	}

	void endGoldenAge(PlayerTypes ePlayer) override {
		fprintf(stderr, "[EVENT STUB] endGoldenAge\n");
	}

	void changeWar(bool bWar, TeamTypes eTeam, TeamTypes eOtherTeam) override {
		fprintf(stderr, "[EVENT STUB] changeWar\n");
	}

	void setPlayerAlive(PlayerTypes ePlayerID, bool bNewValue) override {
		fprintf(stderr, "[EVENT STUB] setPlayerAlive\n");
	}

	void playerChangeStateReligion(PlayerTypes ePlayerID, ReligionTypes eNewReligion, ReligionTypes eOldReligion) override {
		fprintf(stderr, "[EVENT STUB] playerChangeStateReligion\n");
	}

	void playerGoldTrade(PlayerTypes eFromPlayer, PlayerTypes eToPlayer, int iAmount) override {
		fprintf(stderr, "[EVENT STUB] playerGoldTrade\n");
	}

	void chat(char* szString) override {
		fprintf(stderr, "[EVENT STUB] chat: %s\n", szString ? szString : "(null)");
	}

	void victory(TeamTypes eNewWinner, VictoryTypes eNewVictory) override {
		fprintf(stderr, "[EVENT STUB] victory\n");
	}

	void vassalState(TeamTypes eMaster, TeamTypes eVassal, bool bVassal) override {
		fprintf(stderr, "[EVENT STUB] vassalState\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLEventReporterIFaceImpl g_eventReporterIFaceInstance;
CvDLLEventReporterIFaceBase* g_pEventReporterIFace = &g_eventReporterIFaceInstance;
