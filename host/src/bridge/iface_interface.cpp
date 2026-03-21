// =============================================================================
// File:              iface_interface.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLInterfaceIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_interface.h"
#include <cstdio>

class CvDLLInterfaceIFaceImpl : public CvDLLInterfaceIFaceBase
{
public:
	// =========================================================================
	// Selection / Look-at
	// =========================================================================

	void lookAtSelectionPlot(bool bRelease) override {
		fprintf(stderr, "[IFACE STUB] lookAtSelectionPlot\n");
	}

	bool canHandleAction(int iAction, CvPlot* pPlot, bool bTestVisible) override {
		fprintf(stderr, "[IFACE STUB] canHandleAction\n");
		return false;
	}

	bool canDoInterfaceMode(InterfaceModeTypes eInterfaceMode, CvSelectionGroup* pSelectionGroup) override {
		fprintf(stderr, "[IFACE STUB] canDoInterfaceMode\n");
		return false;
	}

	CvPlot* getLookAtPlot() override {
		fprintf(stderr, "[IFACE STUB] getLookAtPlot\n");
		return nullptr;
	}

	CvPlot* getSelectionPlot() override {
		fprintf(stderr, "[IFACE STUB] getSelectionPlot\n");
		return nullptr;
	}

	CvUnit* getInterfacePlotUnit(const CvPlot* pPlot, int iIndex) override {
		fprintf(stderr, "[IFACE STUB] getInterfacePlotUnit\n");
		return nullptr;
	}

	CvUnit* getSelectionUnit(int iIndex) override {
		fprintf(stderr, "[IFACE STUB] getSelectionUnit\n");
		return nullptr;
	}

	CvUnit* getHeadSelectedUnit() override {
		fprintf(stderr, "[IFACE STUB] getHeadSelectedUnit\n");
		return nullptr;
	}

	void selectUnit(CvUnit* pUnit, bool bClear, bool bToggle, bool bSound) override {
		fprintf(stderr, "[IFACE STUB] selectUnit\n");
	}

	void selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt) override {
		fprintf(stderr, "[IFACE STUB] selectGroup\n");
	}

	void selectAll(CvPlot* pPlot) override {
		fprintf(stderr, "[IFACE STUB] selectAll\n");
	}

	bool removeFromSelectionList(CvUnit* pUnit) override {
		fprintf(stderr, "[IFACE STUB] removeFromSelectionList\n");
		return false;
	}

	void makeSelectionListDirty() override {
		fprintf(stderr, "[IFACE STUB] makeSelectionListDirty\n");
	}

	bool mirrorsSelectionGroup() override {
		fprintf(stderr, "[IFACE STUB] mirrorsSelectionGroup\n");
		return false;
	}

	bool canSelectionListFound() override {
		fprintf(stderr, "[IFACE STUB] canSelectionListFound\n");
		return false;
	}

	// =========================================================================
	// Popups
	// =========================================================================

	void bringToTop(CvPopup* pPopup) override {
		fprintf(stderr, "[IFACE STUB] bringToTop\n");
	}

	bool isPopupUp() override {
		fprintf(stderr, "[IFACE STUB] isPopupUp\n");
		return false;
	}

	bool isPopupQueued() override {
		fprintf(stderr, "[IFACE STUB] isPopupQueued\n");
		return false;
	}

	bool isDiploOrPopupWaiting() override {
		fprintf(stderr, "[IFACE STUB] isDiploOrPopupWaiting\n");
		return false;
	}

	// =========================================================================
	// Last selected / Plot list
	// =========================================================================

	CvUnit* getLastSelectedUnit() override {
		fprintf(stderr, "[IFACE STUB] getLastSelectedUnit\n");
		return nullptr;
	}

	void setLastSelectedUnit(CvUnit* pUnit) override {
		fprintf(stderr, "[IFACE STUB] setLastSelectedUnit\n");
	}

	void changePlotListColumn(int iChange) override {
		fprintf(stderr, "[IFACE STUB] changePlotListColumn\n");
	}

	CvPlot* getGotoPlot() override {
		fprintf(stderr, "[IFACE STUB] getGotoPlot\n");
		return nullptr;
	}

	CvPlot* getSingleMoveGotoPlot() override {
		fprintf(stderr, "[IFACE STUB] getSingleMoveGotoPlot\n");
		return nullptr;
	}

	CvPlot* getOriginalPlot() override {
		fprintf(stderr, "[IFACE STUB] getOriginalPlot\n");
		return nullptr;
	}

	// =========================================================================
	// Sound / Popups
	// =========================================================================

	void playGeneralSound(LPCTSTR pszSound, NiPoint3 vPos) override {
		fprintf(stderr, "[IFACE STUB] playGeneralSound(str)\n");
	}

	void playGeneralSound(int iSoundId, int iSoundType, NiPoint3 vPos) override {
		fprintf(stderr, "[IFACE STUB] playGeneralSound(id)\n");
	}

	void clearQueuedPopups() override {
		fprintf(stderr, "[IFACE STUB] clearQueuedPopups\n");
	}

	// =========================================================================
	// Selection list
	// =========================================================================

	CvSelectionGroup* getSelectionList() override {
		fprintf(stderr, "[IFACE STUB] getSelectionList\n");
		return nullptr;
	}

	void clearSelectionList() override {
		fprintf(stderr, "[IFACE STUB] clearSelectionList\n");
	}

	void insertIntoSelectionList(CvUnit* pUnit, bool bClear, bool bToggle, bool bGroup, bool bSound, bool bMinimalChange) override {
		fprintf(stderr, "[IFACE STUB] insertIntoSelectionList\n");
	}

	void selectionListPostChange() override {
		fprintf(stderr, "[IFACE STUB] selectionListPostChange\n");
	}

	void selectionListPreChange() override {
		fprintf(stderr, "[IFACE STUB] selectionListPreChange\n");
	}

	int getSymbolID(int iSymbol) override {
		fprintf(stderr, "[IFACE STUB] getSymbolID\n");
		return 0;
	}

	CLLNode<IDInfo>* deleteSelectionListNode(CLLNode<IDInfo>* pNode) override {
		fprintf(stderr, "[IFACE STUB] deleteSelectionListNode\n");
		return nullptr;
	}

	CLLNode<IDInfo>* nextSelectionListNode(CLLNode<IDInfo>* pNode) override {
		fprintf(stderr, "[IFACE STUB] nextSelectionListNode\n");
		return nullptr;
	}

	int getLengthSelectionList() override {
		fprintf(stderr, "[IFACE STUB] getLengthSelectionList\n");
		return 0;
	}

	CLLNode<IDInfo>* headSelectionListNode() override {
		fprintf(stderr, "[IFACE STUB] headSelectionListNode\n");
		return nullptr;
	}

	// =========================================================================
	// City selection
	// =========================================================================

	void selectCity(CvCity* pNewValue, bool bTestProduction) override {
		fprintf(stderr, "[IFACE STUB] selectCity\n");
	}

	void selectLookAtCity(bool bAdd) override {
		fprintf(stderr, "[IFACE STUB] selectLookAtCity\n");
	}

	void addSelectedCity(CvCity* pNewValue, bool bToggle) override {
		fprintf(stderr, "[IFACE STUB] addSelectedCity\n");
	}

	void clearSelectedCities() override {
		fprintf(stderr, "[IFACE STUB] clearSelectedCities\n");
	}

	bool isCitySelected(CvCity* pCity) override {
		fprintf(stderr, "[IFACE STUB] isCitySelected\n");
		return false;
	}

	CvCity* getHeadSelectedCity() override {
		fprintf(stderr, "[IFACE STUB] getHeadSelectedCity\n");
		return nullptr;
	}

	bool isCitySelection() override {
		fprintf(stderr, "[IFACE STUB] isCitySelection\n");
		return false;
	}

	CLLNode<IDInfo>* nextSelectedCitiesNode(CLLNode<IDInfo>* pNode) override {
		fprintf(stderr, "[IFACE STUB] nextSelectedCitiesNode\n");
		return nullptr;
	}

	CLLNode<IDInfo>* headSelectedCitiesNode() override {
		fprintf(stderr, "[IFACE STUB] headSelectedCitiesNode\n");
		return nullptr;
	}

	// =========================================================================
	// Messages
	// =========================================================================

	void addMessage(PlayerTypes ePlayer, bool bForce, int iLength, CvWString szString, LPCTSTR pszSound,
		InterfaceMessageTypes eType, LPCSTR pszIcon, ColorTypes eFlashColor,
		int iFlashX, int iFlashY, bool bShowOffScreenArrows, bool bShowOnScreenArrows) override {
		fprintf(stderr, "[IFACE STUB] addMessage\n");
	}

	void addCombatMessage(PlayerTypes ePlayer, CvWString szString) override {
		fprintf(stderr, "[IFACE STUB] addCombatMessage\n");
	}

	void addQuestMessage(PlayerTypes ePlayer, CvWString szString, int iQuestId) override {
		fprintf(stderr, "[IFACE STUB] addQuestMessage\n");
	}

	void showMessage(CvTalkingHeadMessage& msg) override {
		fprintf(stderr, "[IFACE STUB] showMessage\n");
	}

	void flushTalkingHeadMessages() override {
		fprintf(stderr, "[IFACE STUB] flushTalkingHeadMessages\n");
	}

	void clearEventMessages() override {
		fprintf(stderr, "[IFACE STUB] clearEventMessages\n");
	}

	void addPopup(CvPopupInfo* pInfo, PlayerTypes ePlayer, bool bImmediate, bool bFront) override {
		fprintf(stderr, "[IFACE STUB] addPopup\n");
	}

	void getDisplayedButtonPopups(CvPopupQueue& infos) override {
		fprintf(stderr, "[IFACE STUB] getDisplayedButtonPopups\n");
	}

	// =========================================================================
	// Cycle / End turn counters
	// =========================================================================

	int getCycleSelectionCounter() override {
		fprintf(stderr, "[IFACE STUB] getCycleSelectionCounter\n");
		return 0;
	}

	void setCycleSelectionCounter(int iNewValue) override {
		fprintf(stderr, "[IFACE STUB] setCycleSelectionCounter\n");
	}

	void changeCycleSelectionCounter(int iChange) override {
		fprintf(stderr, "[IFACE STUB] changeCycleSelectionCounter\n");
	}

	int getEndTurnCounter() override {
		fprintf(stderr, "[IFACE STUB] getEndTurnCounter\n");
		return 0;
	}

	void setEndTurnCounter(int iNewValue) override {
		fprintf(stderr, "[IFACE STUB] setEndTurnCounter\n");
	}

	void changeEndTurnCounter(int iChange) override {
		fprintf(stderr, "[IFACE STUB] changeEndTurnCounter\n");
	}

	// =========================================================================
	// Combat focus / Diplomacy
	// =========================================================================

	bool isCombatFocus() override {
		fprintf(stderr, "[IFACE STUB] isCombatFocus\n");
		return false;
	}

	void setCombatFocus(bool bNewValue) override {
		fprintf(stderr, "[IFACE STUB] setCombatFocus\n");
	}

	void setDiploQueue(CvDiploParameters* pDiploParams, PlayerTypes ePlayer) override {
		fprintf(stderr, "[IFACE STUB] setDiploQueue\n");
	}

	// =========================================================================
	// Dirty bits / UI state
	// =========================================================================

	bool isDirty(InterfaceDirtyBits eDirtyItem) override {
		fprintf(stderr, "[IFACE STUB] isDirty\n");
		return false;
	}

	void setDirty(InterfaceDirtyBits eDirtyItem, bool bNewValue) override {
		fprintf(stderr, "[IFACE STUB] setDirty\n");
	}

	void makeInterfaceDirty() override {
		fprintf(stderr, "[IFACE STUB] makeInterfaceDirty\n");
	}

	bool updateCursorType() override {
		fprintf(stderr, "[IFACE STUB] updateCursorType\n");
		return false;
	}

	void updatePythonScreens() override {
		fprintf(stderr, "[IFACE STUB] updatePythonScreens\n");
	}

	// =========================================================================
	// Camera
	// =========================================================================

	void lookAt(NiPoint3 pt3Target, CameraLookAtTypes type, NiPoint3 attackDirection) override {
		fprintf(stderr, "[IFACE STUB] lookAt\n");
	}

	void centerCamera(CvUnit*) override {
		fprintf(stderr, "[IFACE STUB] centerCamera\n");
	}

	void releaseLockedCamera() override {
		fprintf(stderr, "[IFACE STUB] releaseLockedCamera\n");
	}

	bool isFocusedWidget() override {
		fprintf(stderr, "[IFACE STUB] isFocusedWidget\n");
		return false;
	}

	bool isFocused() override {
		fprintf(stderr, "[IFACE STUB] isFocused\n");
		return false;
	}

	bool isBareMapMode() override {
		fprintf(stderr, "[IFACE STUB] isBareMapMode\n");
		return false;
	}

	void toggleBareMapMode() override {
		fprintf(stderr, "[IFACE STUB] toggleBareMapMode\n");
	}

	bool isShowYields() override {
		fprintf(stderr, "[IFACE STUB] isShowYields\n");
		return false;
	}

	void toggleYieldVisibleMode() override {
		fprintf(stderr, "[IFACE STUB] toggleYieldVisibleMode\n");
	}

	bool isScoresVisible() override {
		fprintf(stderr, "[IFACE STUB] isScoresVisible\n");
		return false;
	}

	void toggleScoresVisible() override {
		fprintf(stderr, "[IFACE STUB] toggleScoresVisible\n");
	}

	bool isScoresMinimized() override {
		fprintf(stderr, "[IFACE STUB] isScoresMinimized\n");
		return false;
	}

	void toggleScoresMinimized() override {
		fprintf(stderr, "[IFACE STUB] toggleScoresMinimized\n");
	}

	bool isNetStatsVisible() override {
		fprintf(stderr, "[IFACE STUB] isNetStatsVisible\n");
		return false;
	}

	// =========================================================================
	// Misc UI queries
	// =========================================================================

	int getOriginalPlotCount() override {
		fprintf(stderr, "[IFACE STUB] getOriginalPlotCount\n");
		return 0;
	}

	bool isCityScreenUp() override {
		fprintf(stderr, "[IFACE STUB] isCityScreenUp\n");
		return false;
	}

	bool isEndTurnMessage() override {
		fprintf(stderr, "[IFACE STUB] isEndTurnMessage\n");
		return false;
	}

	void setInterfaceMode(InterfaceModeTypes eNewValue) override {
		fprintf(stderr, "[IFACE STUB] setInterfaceMode\n");
	}

	InterfaceModeTypes getInterfaceMode() override {
		fprintf(stderr, "[IFACE STUB] getInterfaceMode\n");
		return InterfaceModeTypes{};
	}

	InterfaceVisibility getShowInterface() override {
		fprintf(stderr, "[IFACE STUB] getShowInterface\n");
		return InterfaceVisibility{};
	}

	CvPlot* getMouseOverPlot() override {
		fprintf(stderr, "[IFACE STUB] getMouseOverPlot\n");
		return nullptr;
	}

	void setFlashing(PlayerTypes eWho, bool bFlashing) override {
		fprintf(stderr, "[IFACE STUB] setFlashing\n");
	}

	bool isFlashing(PlayerTypes eWho) override {
		fprintf(stderr, "[IFACE STUB] isFlashing\n");
		return false;
	}

	void setDiplomacyLocked(bool bLocked) override {
		fprintf(stderr, "[IFACE STUB] setDiplomacyLocked\n");
	}

	bool isDiplomacyLocked() override {
		fprintf(stderr, "[IFACE STUB] isDiplomacyLocked\n");
		return false;
	}

	// =========================================================================
	// Minimap
	// =========================================================================

	void setMinimapColor(MinimapModeTypes eMinimapMode, int iX, int iY, ColorTypes eColor, float fAlpha) override {
		fprintf(stderr, "[IFACE STUB] setMinimapColor\n");
	}

	unsigned char* getMinimapBaseTexture() const override {
		fprintf(stderr, "[IFACE STUB] getMinimapBaseTexture\n");
		return nullptr;
	}

	void setEndTurnMessage(bool bNewValue) override {
		fprintf(stderr, "[IFACE STUB] setEndTurnMessage\n");
	}

	// =========================================================================
	// Moved unit
	// =========================================================================

	bool isHasMovedUnit() override {
		fprintf(stderr, "[IFACE STUB] isHasMovedUnit\n");
		return false;
	}

	void setHasMovedUnit(bool bNewValue) override {
		fprintf(stderr, "[IFACE STUB] setHasMovedUnit\n");
	}

	// =========================================================================
	// Force popup
	// =========================================================================

	bool isForcePopup() override {
		fprintf(stderr, "[IFACE STUB] isForcePopup\n");
		return false;
	}

	void setForcePopup(bool bNewValue) override {
		fprintf(stderr, "[IFACE STUB] setForcePopup\n");
	}

	// =========================================================================
	// City offset / Turn log
	// =========================================================================

	void lookAtCityOffset(int iCity) override {
		fprintf(stderr, "[IFACE STUB] lookAtCityOffset\n");
	}

	void toggleTurnLog() override {
		fprintf(stderr, "[IFACE STUB] toggleTurnLog\n");
	}

	void showTurnLog(ChatTargetTypes eTarget) override {
		fprintf(stderr, "[IFACE STUB] showTurnLog\n");
	}

	void dirtyTurnLog(PlayerTypes ePlayer) override {
		fprintf(stderr, "[IFACE STUB] dirtyTurnLog\n");
	}

	// =========================================================================
	// Plot list
	// =========================================================================

	int getPlotListColumn() override {
		fprintf(stderr, "[IFACE STUB] getPlotListColumn\n");
		return 0;
	}

	void verifyPlotListColumn() override {
		fprintf(stderr, "[IFACE STUB] verifyPlotListColumn\n");
	}

	int getPlotListOffset() override {
		fprintf(stderr, "[IFACE STUB] getPlotListOffset\n");
		return 0;
	}

	// =========================================================================
	// Popup help
	// =========================================================================

	void unlockPopupHelp() override {
		fprintf(stderr, "[IFACE STUB] unlockPopupHelp\n");
	}

	// =========================================================================
	// Details / Admin
	// =========================================================================

	void showDetails(bool bPasswordOnly) override {
		fprintf(stderr, "[IFACE STUB] showDetails\n");
	}

	void showAdminDetails() override {
		fprintf(stderr, "[IFACE STUB] showAdminDetails\n");
	}

	// =========================================================================
	// Clock alarm
	// =========================================================================

	void toggleClockAlarm(bool bValue, int iHour, int iMin) override {
		fprintf(stderr, "[IFACE STUB] toggleClockAlarm\n");
	}

	bool isClockAlarmOn() override {
		fprintf(stderr, "[IFACE STUB] isClockAlarmOn\n");
		return false;
	}

	// =========================================================================
	// Screen state
	// =========================================================================

	void setScreenDying(int iPythonFileID, bool bDying) override {
		fprintf(stderr, "[IFACE STUB] setScreenDying\n");
	}

	bool isExitingToMainMenu() override {
		fprintf(stderr, "[IFACE STUB] isExitingToMainMenu\n");
		return false;
	}

	void exitingToMainMenu(const char* szLoadFile) override {
		fprintf(stderr, "[IFACE STUB] exitingToMainMenu\n");
	}

	void setWorldBuilder(bool bTurnOn) override {
		fprintf(stderr, "[IFACE STUB] setWorldBuilder\n");
	}

	// =========================================================================
	// Font justification
	// =========================================================================

	int getFontLeftJustify() override {
		fprintf(stderr, "[IFACE STUB] getFontLeftJustify\n");
		return DLL_FONT_LEFT_JUSTIFY;
	}

	int getFontRightJustify() override {
		fprintf(stderr, "[IFACE STUB] getFontRightJustify\n");
		return DLL_FONT_RIGHT_JUSTIFY;
	}

	int getFontCenterJustify() override {
		fprintf(stderr, "[IFACE STUB] getFontCenterJustify\n");
		return DLL_FONT_CENTER_JUSTIFY;
	}

	int getFontCenterVertically() override {
		fprintf(stderr, "[IFACE STUB] getFontCenterVertically\n");
		return 0;
	}

	int getFontAdditive() override {
		fprintf(stderr, "[IFACE STUB] getFontAdditive\n");
		return 0;
	}

	// =========================================================================
	// Popup configuration
	// =========================================================================

	void popupSetHeaderString(CvPopup* pPopup, CvWString szText, uint uiFlags) override {
		fprintf(stderr, "[IFACE STUB] popupSetHeaderString\n");
	}

	void popupSetBodyString(CvPopup* pPopup, CvWString szText, uint uiFlags, char* szName, CvWString szHelpText) override {
		fprintf(stderr, "[IFACE STUB] popupSetBodyString\n");
	}

	void popupLaunch(CvPopup* pPopup, bool bCreateOkButton, PopupStates bState, int iNumPixelScroll) override {
		fprintf(stderr, "[IFACE STUB] popupLaunch\n");
	}

	void popupSetPopupType(CvPopup* pPopup, PopupEventTypes ePopupType, LPCTSTR szArtFileName) override {
		fprintf(stderr, "[IFACE STUB] popupSetPopupType\n");
	}

	void popupSetStyle(CvPopup* pPopup, const char* styleId) override {
		fprintf(stderr, "[IFACE STUB] popupSetStyle\n");
	}

	void popupAddDDS(CvPopup* pPopup, const char* szIconFilename, int iWidth, int iHeight, CvWString szHelpText) override {
		fprintf(stderr, "[IFACE STUB] popupAddDDS\n");
	}

	void popupAddSeparator(CvPopup* pPopup, int iSpace) override {
		fprintf(stderr, "[IFACE STUB] popupAddSeparator\n");
	}

	void popupAddGenericButton(CvPopup* pPopup, CvWString szText, const char* szIcon, int iButtonId, WidgetTypes eWidgetType, int iData1, int iData2,
		bool bOption, PopupControlLayout ctrlLayout, unsigned int textJustifcation) override {
		fprintf(stderr, "[IFACE STUB] popupAddGenericButton\n");
	}

	void popupCreateEditBox(CvPopup* pPopup, CvWString szDefaultString, WidgetTypes eWidgetType, CvWString szHelpText, int iGroup,
		PopupControlLayout ctrlLayout, unsigned int preferredCharWidth, unsigned int maxCharCount) override {
		fprintf(stderr, "[IFACE STUB] popupCreateEditBox\n");
	}

	void popupEnableEditBox(CvPopup* pPopup, int iGroup, bool bEnable) override {
		fprintf(stderr, "[IFACE STUB] popupEnableEditBox\n");
	}

	void popupCreateRadioButtons(CvPopup* pPopup, int iNumButtons, int iGroup, WidgetTypes eWidgetType, PopupControlLayout ctrlLayout) override {
		fprintf(stderr, "[IFACE STUB] popupCreateRadioButtons\n");
	}

	void popupSetRadioButtonText(CvPopup* pPopup, int iRadioButtonID, CvWString szText, int iGroup, CvWString szHelpText) override {
		fprintf(stderr, "[IFACE STUB] popupSetRadioButtonText\n");
	}

	void popupCreateCheckBoxes(CvPopup* pPopup, int iNumBoxes, int iGroup, WidgetTypes eWidgetType, PopupControlLayout ctrlLayout) override {
		fprintf(stderr, "[IFACE STUB] popupCreateCheckBoxes\n");
	}

	void popupSetCheckBoxText(CvPopup* pPopup, int iCheckBoxID, CvWString szText, int iGroup, CvWString szHelpText) override {
		fprintf(stderr, "[IFACE STUB] popupSetCheckBoxText\n");
	}

	void popupSetCheckBoxState(CvPopup* pPopup, int iCheckBoxID, bool bChecked, int iGroup) override {
		fprintf(stderr, "[IFACE STUB] popupSetCheckBoxState\n");
	}

	void popupSetAsCancelled(CvPopup* pPopup) override {
		fprintf(stderr, "[IFACE STUB] popupSetAsCancelled\n");
	}

	bool popupIsDying(CvPopup* pPopup) override {
		fprintf(stderr, "[IFACE STUB] popupIsDying\n");
		return false;
	}

	void setCityTabSelectionRow(CityTabTypes eTabType) override {
		fprintf(stderr, "[IFACE STUB] setCityTabSelectionRow\n");
	}

	// =========================================================================
	// Misc
	// =========================================================================

	bool noTechSplash() override {
		fprintf(stderr, "[IFACE STUB] noTechSplash\n");
		return false;
	}

	bool isInAdvancedStart() const override {
		fprintf(stderr, "[IFACE STUB] isInAdvancedStart\n");
		return false;
	}

	void setInAdvancedStart(bool bAdvancedStart) override {
		fprintf(stderr, "[IFACE STUB] setInAdvancedStart\n");
	}

	bool isSpaceshipScreenUp() const override {
		fprintf(stderr, "[IFACE STUB] isSpaceshipScreenUp\n");
		return false;
	}

	bool isDebugMenuCreated() const override {
		fprintf(stderr, "[IFACE STUB] isDebugMenuCreated\n");
		return false;
	}

	void setBusy(bool bBusy) override {
		fprintf(stderr, "[IFACE STUB] setBusy\n");
	}

	void getInterfaceScreenIdsForInput(std::vector<int>& aIds) override {
		fprintf(stderr, "[IFACE STUB] getInterfaceScreenIdsForInput\n");
	}

	void doPing(int iX, int iY, PlayerTypes ePlayer) override {
		fprintf(stderr, "[IFACE STUB] doPing\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLInterfaceIFaceImpl g_interfaceIFaceInstance;
CvDLLInterfaceIFaceBase* g_pInterfaceIFace = &g_interfaceIFaceInstance;
