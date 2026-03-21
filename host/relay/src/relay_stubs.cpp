// =============================================================================
// File:              relay_stubs.cpp
// Author(s):         Chrischn89
// Description:
//   Category C sub-interface stub implementations. All methods return safe
//   defaults (0, false, NULL, empty body). Vtable order matches SDK exactly.
//   Each interface's method list is verified against host/src/bridge/iface_*.h.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "relay_types.h"

// POINT struct needed by CvDLLEventReporterIFaceBase::mouseEvent
#ifndef _WINDEF_
struct POINT { long x; long y; };
#endif

// =============================================================================
// 1. CvDLLEngineIFaceBase — host/src/bridge/iface_engine.h (82 methods)
// =============================================================================

class CvDLLEngineIFaceBase
{
public:
    virtual void cameraLookAt(NiPoint3 lookingPoint) = 0;
    virtual bool isCameraLocked() = 0;
    virtual void SetObeyEntityVisibleFlags(bool bObeyHide) = 0;
    virtual void AutoSave(bool bInitial) = 0;
    virtual void SaveReplay(PlayerTypes ePlayer) = 0;
    virtual void SaveGame(CvString& szFilename, SaveGameTypes eType) = 0;
    virtual void DoTurn() = 0;
    virtual void ClearMinimap() = 0;
    virtual byte GetLandscapePlotTerrainData(uint uiX, uint uiY, uint uiPointX, uint uiPointY) = 0;
    virtual byte GetLandscapePlotHeightData(uint uiX, uint uiY, uint uiPointX, uint uiPointY) = 0;
    virtual LoadType getLoadType() = 0;
    virtual void ClampToWorldCoords(NiPoint3* pPt3, float fOffset) = 0;
    virtual void SetCameraZoom(float zoom) = 0;
    virtual float GetUpdateRate() = 0;
    virtual bool SetUpdateRate(float fUpdateRate) = 0;
    virtual void toggleGlobeview() = 0;
    virtual bool isGlobeviewUp() = 0;
    virtual void toggleResourceLayer() = 0;
    virtual void toggleUnitLayer() = 0;
    virtual void setResourceLayer(bool bOn) = 0;
    virtual void MoveBaseTurnRight(float increment) = 0;
    virtual void MoveBaseTurnLeft(float increment) = 0;
    virtual void SetFlying(bool value) = 0;
    virtual void CycleFlyingMode(int displacement) = 0;
    virtual void SetMouseFlying(bool value) = 0;
    virtual void SetSatelliteMode(bool value) = 0;
    virtual void SetOrthoCamera(bool value) = 0;
    virtual bool GetFlying() = 0;
    virtual bool GetMouseFlying() = 0;
    virtual bool GetSatelliteMode() = 0;
    virtual bool GetOrthoCamera() = 0;
    virtual int InitGraphics() = 0;
    virtual void GetLandscapeDimensions(float& fWidth, float& fHeight) = 0;
    virtual void GetLandscapeGameDimensions(float& fWidth, float& fHeight) = 0;
    virtual uint GetGameCellSizeX() = 0;
    virtual uint GetGameCellSizeY() = 0;
    virtual float GetPointZSpacing() = 0;
    virtual float GetPointXYSpacing() = 0;
    virtual float GetPointXSpacing() = 0;
    virtual float GetPointYSpacing() = 0;
    virtual float GetHeightmapZ(const NiPoint3& pt3, bool bClampAboveWater) = 0;
    virtual void LightenVisibility(uint) = 0;
    virtual void DarkenVisibility(uint) = 0;
    virtual void BlackenVisibility(uint) = 0;
    virtual void RebuildAllPlots() = 0;
    virtual void RebuildPlot(int plotX, int plotY, bool bRebuildHeights, bool bRebuildTextures) = 0;
    virtual void RebuildRiverPlotTile(int plotX, int plotY, bool bRebuildHeights, bool bRebuildTextures) = 0;
    virtual void RebuildTileArt(int plotX, int plotY) = 0;
    virtual void ForceTreeOffsets(int plotX, int plotY) = 0;
    virtual bool GetGridMode() = 0;
    virtual void SetGridMode(bool bVal) = 0;
    virtual void addColoredPlot(int plotX, int plotY, const NiColorA& color, PlotStyles plotStyle, PlotLandscapeLayers layer) = 0;
    virtual void clearColoredPlots(PlotLandscapeLayers layer) = 0;
    virtual void fillAreaBorderPlot(int plotX, int plotY, const NiColorA& color, AreaBorderLayers layer) = 0;
    virtual void clearAreaBorderPlots(AreaBorderLayers layer) = 0;
    virtual void updateFoundingBorder() = 0;
    virtual void addLandmark(CvPlot* plot, const wchar* caption) = 0;
    virtual void TriggerEffect(int iEffect, NiPoint3 pt3Point, float rotation) = 0;
    virtual void printProfileText() = 0;
    virtual void clearSigns() = 0;
    virtual CvPlot* pickPlot(int x, int y, NiPoint3& worldPoint) = 0;
    virtual void SetDirty(EngineDirtyBits eBit, bool bNewValue) = 0;
    virtual bool IsDirty(EngineDirtyBits eBit) = 0;
    virtual void PushFogOfWar(FogOfWarModeTypes eNewMode) = 0;
    virtual FogOfWarModeTypes PopFogOfWar() = 0;
    virtual void setFogOfWarFromStack() = 0;
    virtual void MarkBridgesDirty() = 0;
    virtual void AddLaunch(PlayerTypes playerType) = 0;
    virtual void AddGreatWall(CvCity* city) = 0;
    virtual void RemoveGreatWall(CvCity* city) = 0;
    virtual void MarkPlotTextureAsDirty(int plotX, int plotY) = 0;
};

class CvDLLEngineIFaceImpl : public CvDLLEngineIFaceBase
{
public:
    void cameraLookAt(NiPoint3) {}
    bool isCameraLocked() { return false; }
    void SetObeyEntityVisibleFlags(bool) {}
    void AutoSave(bool) {}
    void SaveReplay(PlayerTypes) {}
    void SaveGame(CvString&, SaveGameTypes) {}
    void DoTurn() {}
    void ClearMinimap() {}
    byte GetLandscapePlotTerrainData(uint, uint, uint, uint) { return 0; }
    byte GetLandscapePlotHeightData(uint, uint, uint, uint) { return 0; }
    LoadType getLoadType() { return (LoadType)0; }
    void ClampToWorldCoords(NiPoint3*, float) {}
    void SetCameraZoom(float) {}
    float GetUpdateRate() { return 0.0f; }
    bool SetUpdateRate(float) { return false; }
    void toggleGlobeview() {}
    bool isGlobeviewUp() { return false; }
    void toggleResourceLayer() {}
    void toggleUnitLayer() {}
    void setResourceLayer(bool) {}
    void MoveBaseTurnRight(float) {}
    void MoveBaseTurnLeft(float) {}
    void SetFlying(bool) {}
    void CycleFlyingMode(int) {}
    void SetMouseFlying(bool) {}
    void SetSatelliteMode(bool) {}
    void SetOrthoCamera(bool) {}
    bool GetFlying() { return false; }
    bool GetMouseFlying() { return false; }
    bool GetSatelliteMode() { return false; }
    bool GetOrthoCamera() { return false; }
    int InitGraphics() { return 0; }
    void GetLandscapeDimensions(float& fWidth, float& fHeight) { fWidth = 0.0f; fHeight = 0.0f; }
    void GetLandscapeGameDimensions(float& fWidth, float& fHeight) { fWidth = 0.0f; fHeight = 0.0f; }
    uint GetGameCellSizeX() { return 0; }
    uint GetGameCellSizeY() { return 0; }
    float GetPointZSpacing() { return 0.0f; }
    float GetPointXYSpacing() { return 0.0f; }
    float GetPointXSpacing() { return 0.0f; }
    float GetPointYSpacing() { return 0.0f; }
    float GetHeightmapZ(const NiPoint3&, bool) { return 0.0f; }
    void LightenVisibility(uint) {}
    void DarkenVisibility(uint) {}
    void BlackenVisibility(uint) {}
    void RebuildAllPlots() {}
    void RebuildPlot(int, int, bool, bool) {}
    void RebuildRiverPlotTile(int, int, bool, bool) {}
    void RebuildTileArt(int, int) {}
    void ForceTreeOffsets(int, int) {}
    bool GetGridMode() { return false; }
    void SetGridMode(bool) {}
    void addColoredPlot(int, int, const NiColorA&, PlotStyles, PlotLandscapeLayers) {}
    void clearColoredPlots(PlotLandscapeLayers) {}
    void fillAreaBorderPlot(int, int, const NiColorA&, AreaBorderLayers) {}
    void clearAreaBorderPlots(AreaBorderLayers) {}
    void updateFoundingBorder() {}
    void addLandmark(CvPlot*, const wchar*) {}
    void TriggerEffect(int, NiPoint3, float) {}
    void printProfileText() {}
    void clearSigns() {}
    CvPlot* pickPlot(int, int, NiPoint3&) { return NULL; }
    void SetDirty(EngineDirtyBits, bool) {}
    bool IsDirty(EngineDirtyBits) { return false; }
    void PushFogOfWar(FogOfWarModeTypes) {}
    FogOfWarModeTypes PopFogOfWar() { return (FogOfWarModeTypes)0; }
    void setFogOfWarFromStack() {}
    void MarkBridgesDirty() {}
    void AddLaunch(PlayerTypes) {}
    void AddGreatWall(CvCity*) {}
    void RemoveGreatWall(CvCity*) {}
    void MarkPlotTextureAsDirty(int, int) {}
};

// =============================================================================
// 2. CvDLLEntityIFaceBase impl — base class defined in relay_types.h
//    host/src/bridge/iface_entity.h (26 methods)
// =============================================================================

class CvDLLEntityIFaceImpl : public CvDLLEntityIFaceBase
{
public:
    void removeEntity(CvEntity*) {}
    void addEntity(CvEntity*, uint) {}
    void setup(CvEntity*) {}
    void setVisible(CvEntity*, bool) {}
    void createCityEntity(CvCity*) {}
    void createUnitEntity(CvUnit*) {}
    void destroyEntity(CvEntity*&, bool) {}
    void updatePosition(CvEntity*) {}
    void setupFloodPlains(CvRiver*) {}
    bool IsSelected(const CvEntity*) const { return false; }
    void PlayAnimation(CvEntity*, AnimationTypes, float, bool, int, float, float) {}
    void StopAnimation(CvEntity*, AnimationTypes) {}
    void StopAnimation(CvEntity*) {}
    void NotifyEntity(CvUnitEntity*, MissionTypes) {}
    void MoveTo(CvUnitEntity*, const CvPlot*) {}
    void QueueMove(CvUnitEntity*, const CvPlot*) {}
    void ExecuteMove(CvUnitEntity*, float, bool) {}
    void SetPosition(CvUnitEntity*, const CvPlot*) {}
    void AddMission(const CvMissionDefinition*) {}
    void RemoveUnitFromBattle(CvUnit*) {}
    void showPromotionGlow(CvUnitEntity*, bool) {}
    void updateEnemyGlow(CvUnitEntity*) {}
    void updatePromotionLayers(CvUnitEntity*) {}
    void updateGraphicEra(CvUnitEntity*, EraTypes) {}
    void SetSiegeTower(CvUnitEntity*, bool) {}
    bool GetSiegeTower(CvUnitEntity*) { return false; }
};

// =============================================================================
// 3. CvDLLInterfaceIFaceBase — host/src/bridge/iface_interface.h (130+ methods)
//
// NOTE: NiPoint3 default params (e.g. NiPoint3{-1,-1,-1}) are omitted from
// the relay's declaration. Default parameter values are resolved at the call
// site and do NOT affect the vtable layout.
// =============================================================================

class CvDLLInterfaceIFaceBase
{
public:
    virtual void lookAtSelectionPlot(bool bRelease) = 0;
    virtual bool canHandleAction(int iAction, CvPlot* pPlot, bool bTestVisible) = 0;
    virtual bool canDoInterfaceMode(InterfaceModeTypes eInterfaceMode, CvSelectionGroup* pSelectionGroup) = 0;
    virtual CvPlot* getLookAtPlot() = 0;
    virtual CvPlot* getSelectionPlot() = 0;
    virtual CvUnit* getInterfacePlotUnit(const CvPlot* pPlot, int iIndex) = 0;
    virtual CvUnit* getSelectionUnit(int iIndex) = 0;
    virtual CvUnit* getHeadSelectedUnit() = 0;
    virtual void selectUnit(CvUnit* pUnit, bool bClear, bool bToggle, bool bSound) = 0;
    virtual void selectGroup(CvUnit* pUnit, bool bShift, bool bCtrl, bool bAlt) = 0;
    virtual void selectAll(CvPlot* pPlot) = 0;
    virtual bool removeFromSelectionList(CvUnit* pUnit) = 0;
    virtual void makeSelectionListDirty() = 0;
    virtual bool mirrorsSelectionGroup() = 0;
    virtual bool canSelectionListFound() = 0;
    virtual void bringToTop(CvPopup* pPopup) = 0;
    virtual bool isPopupUp() = 0;
    virtual bool isPopupQueued() = 0;
    virtual bool isDiploOrPopupWaiting() = 0;
    virtual CvUnit* getLastSelectedUnit() = 0;
    virtual void setLastSelectedUnit(CvUnit* pUnit) = 0;
    virtual void changePlotListColumn(int iChange) = 0;
    virtual CvPlot* getGotoPlot() = 0;
    virtual CvPlot* getSingleMoveGotoPlot() = 0;
    virtual CvPlot* getOriginalPlot() = 0;
    virtual void playGeneralSound(LPCTSTR pszSound, NiPoint3 vPos) = 0;
    virtual void playGeneralSound(int iSoundId, int iSoundType, NiPoint3 vPos) = 0;
    virtual void clearQueuedPopups() = 0;
    virtual CvSelectionGroup* getSelectionList() = 0;
    virtual void clearSelectionList() = 0;
    virtual void insertIntoSelectionList(CvUnit* pUnit, bool bClear, bool bToggle, bool bGroup, bool bSound, bool bMinimalChange) = 0;
    virtual void selectionListPostChange() = 0;
    virtual void selectionListPreChange() = 0;
    virtual int getSymbolID(int iSymbol) = 0;
    virtual CLLNode<IDInfo>* deleteSelectionListNode(CLLNode<IDInfo>* pNode) = 0;
    virtual CLLNode<IDInfo>* nextSelectionListNode(CLLNode<IDInfo>* pNode) = 0;
    virtual int getLengthSelectionList() = 0;
    virtual CLLNode<IDInfo>* headSelectionListNode() = 0;
    virtual void selectCity(CvCity* pNewValue, bool bTestProduction) = 0;
    virtual void selectLookAtCity(bool bAdd) = 0;
    virtual void addSelectedCity(CvCity* pNewValue, bool bToggle) = 0;
    virtual void clearSelectedCities() = 0;
    virtual bool isCitySelected(CvCity* pCity) = 0;
    virtual CvCity* getHeadSelectedCity() = 0;
    virtual bool isCitySelection() = 0;
    virtual CLLNode<IDInfo>* nextSelectedCitiesNode(CLLNode<IDInfo>* pNode) = 0;
    virtual CLLNode<IDInfo>* headSelectedCitiesNode() = 0;
    virtual void addMessage(PlayerTypes ePlayer, bool bForce, int iLength, CvWString szString, LPCTSTR pszSound,
        InterfaceMessageTypes eType, LPCSTR pszIcon, ColorTypes eFlashColor,
        int iFlashX, int iFlashY, bool bShowOffScreenArrows, bool bShowOnScreenArrows) = 0;
    virtual void addCombatMessage(PlayerTypes ePlayer, CvWString szString) = 0;
    virtual void addQuestMessage(PlayerTypes ePlayer, CvWString szString, int iQuestId) = 0;
    virtual void showMessage(CvTalkingHeadMessage& msg) = 0;
    virtual void flushTalkingHeadMessages() = 0;
    virtual void clearEventMessages() = 0;
    virtual void addPopup(CvPopupInfo* pInfo, PlayerTypes ePlayer, bool bImmediate, bool bFront) = 0;
    virtual void getDisplayedButtonPopups(CvPopupQueue& infos) = 0;
    virtual int getCycleSelectionCounter() = 0;
    virtual void setCycleSelectionCounter(int iNewValue) = 0;
    virtual void changeCycleSelectionCounter(int iChange) = 0;
    virtual int getEndTurnCounter() = 0;
    virtual void setEndTurnCounter(int iNewValue) = 0;
    virtual void changeEndTurnCounter(int iChange) = 0;
    virtual bool isCombatFocus() = 0;
    virtual void setCombatFocus(bool bNewValue) = 0;
    virtual void setDiploQueue(CvDiploParameters* pDiploParams, PlayerTypes ePlayer) = 0;
    virtual bool isDirty(InterfaceDirtyBits eDirtyItem) = 0;
    virtual void setDirty(InterfaceDirtyBits eDirtyItem, bool bNewValue) = 0;
    virtual void makeInterfaceDirty() = 0;
    virtual bool updateCursorType() = 0;
    virtual void updatePythonScreens() = 0;
    virtual void lookAt(NiPoint3 pt3Target, CameraLookAtTypes type, NiPoint3 attackDirection) = 0;
    virtual void centerCamera(CvUnit*) = 0;
    virtual void releaseLockedCamera() = 0;
    virtual bool isFocusedWidget() = 0;
    virtual bool isFocused() = 0;
    virtual bool isBareMapMode() = 0;
    virtual void toggleBareMapMode() = 0;
    virtual bool isShowYields() = 0;
    virtual void toggleYieldVisibleMode() = 0;
    virtual bool isScoresVisible() = 0;
    virtual void toggleScoresVisible() = 0;
    virtual bool isScoresMinimized() = 0;
    virtual void toggleScoresMinimized() = 0;
    virtual bool isNetStatsVisible() = 0;
    virtual int getOriginalPlotCount() = 0;
    virtual bool isCityScreenUp() = 0;
    virtual bool isEndTurnMessage() = 0;
    virtual void setInterfaceMode(InterfaceModeTypes eNewValue) = 0;
    virtual InterfaceModeTypes getInterfaceMode() = 0;
    virtual InterfaceVisibility getShowInterface() = 0;
    virtual CvPlot* getMouseOverPlot() = 0;
    virtual void setFlashing(PlayerTypes eWho, bool bFlashing) = 0;
    virtual bool isFlashing(PlayerTypes eWho) = 0;
    virtual void setDiplomacyLocked(bool bLocked) = 0;
    virtual bool isDiplomacyLocked() = 0;
    virtual void setMinimapColor(MinimapModeTypes eMinimapMode, int iX, int iY, ColorTypes eColor, float fAlpha) = 0;
    virtual unsigned char* getMinimapBaseTexture() const = 0;
    virtual void setEndTurnMessage(bool bNewValue) = 0;
    virtual bool isHasMovedUnit() = 0;
    virtual void setHasMovedUnit(bool bNewValue) = 0;
    virtual bool isForcePopup() = 0;
    virtual void setForcePopup(bool bNewValue) = 0;
    virtual void lookAtCityOffset(int iCity) = 0;
    virtual void toggleTurnLog() = 0;
    virtual void showTurnLog(ChatTargetTypes eTarget) = 0;
    virtual void dirtyTurnLog(PlayerTypes ePlayer) = 0;
    virtual int getPlotListColumn() = 0;
    virtual void verifyPlotListColumn() = 0;
    virtual int getPlotListOffset() = 0;
    virtual void unlockPopupHelp() = 0;
    virtual void showDetails(bool bPasswordOnly) = 0;
    virtual void showAdminDetails() = 0;
    virtual void toggleClockAlarm(bool bValue, int iHour, int iMin) = 0;
    virtual bool isClockAlarmOn() = 0;
    virtual void setScreenDying(int iPythonFileID, bool bDying) = 0;
    virtual bool isExitingToMainMenu() = 0;
    virtual void exitingToMainMenu(const char* szLoadFile) = 0;
    virtual void setWorldBuilder(bool bTurnOn) = 0;
    virtual int getFontLeftJustify() = 0;
    virtual int getFontRightJustify() = 0;
    virtual int getFontCenterJustify() = 0;
    virtual int getFontCenterVertically() = 0;
    virtual int getFontAdditive() = 0;
    virtual void popupSetHeaderString(CvPopup* pPopup, CvWString szText, uint uiFlags) = 0;
    virtual void popupSetBodyString(CvPopup* pPopup, CvWString szText, uint uiFlags, char* szName, CvWString szHelpText) = 0;
    virtual void popupLaunch(CvPopup* pPopup, bool bCreateOkButton, PopupStates bState, int iNumPixelScroll) = 0;
    virtual void popupSetPopupType(CvPopup* pPopup, PopupEventTypes ePopupType, LPCTSTR szArtFileName) = 0;
    virtual void popupSetStyle(CvPopup* pPopup, const char* styleId) = 0;
    virtual void popupAddDDS(CvPopup* pPopup, const char* szIconFilename, int iWidth, int iHeight, CvWString szHelpText) = 0;
    virtual void popupAddSeparator(CvPopup* pPopup, int iSpace) = 0;
    virtual void popupAddGenericButton(CvPopup* pPopup, CvWString szText, const char* szIcon, int iButtonId, WidgetTypes eWidgetType, int iData1, int iData2,
        bool bOption, PopupControlLayout ctrlLayout, unsigned int textJustifcation) = 0;
    virtual void popupCreateEditBox(CvPopup* pPopup, CvWString szDefaultString, WidgetTypes eWidgetType, CvWString szHelpText, int iGroup,
        PopupControlLayout ctrlLayout, unsigned int preferredCharWidth, unsigned int maxCharCount) = 0;
    virtual void popupEnableEditBox(CvPopup* pPopup, int iGroup, bool bEnable) = 0;
    virtual void popupCreateRadioButtons(CvPopup* pPopup, int iNumButtons, int iGroup, WidgetTypes eWidgetType, PopupControlLayout ctrlLayout) = 0;
    virtual void popupSetRadioButtonText(CvPopup* pPopup, int iRadioButtonID, CvWString szText, int iGroup, CvWString szHelpText) = 0;
    virtual void popupCreateCheckBoxes(CvPopup* pPopup, int iNumBoxes, int iGroup, WidgetTypes eWidgetType, PopupControlLayout ctrlLayout) = 0;
    virtual void popupSetCheckBoxText(CvPopup* pPopup, int iCheckBoxID, CvWString szText, int iGroup, CvWString szHelpText) = 0;
    virtual void popupSetCheckBoxState(CvPopup* pPopup, int iCheckBoxID, bool bChecked, int iGroup) = 0;
    virtual void popupSetAsCancelled(CvPopup* pPopup) = 0;
    virtual bool popupIsDying(CvPopup* pPopup) = 0;
    virtual void setCityTabSelectionRow(CityTabTypes eTabType) = 0;
    virtual bool noTechSplash() = 0;
    virtual bool isInAdvancedStart() const = 0;
    virtual void setInAdvancedStart(bool bAdvancedStart) = 0;
    virtual bool isSpaceshipScreenUp() const = 0;
    virtual bool isDebugMenuCreated() const = 0;
    virtual void setBusy(bool bBusy) = 0;
    virtual void getInterfaceScreenIdsForInput(std::vector<int>& aIds) = 0;
    virtual void doPing(int iX, int iY, PlayerTypes ePlayer) = 0;
};

class CvDLLInterfaceIFaceImpl : public CvDLLInterfaceIFaceBase
{
public:
    void lookAtSelectionPlot(bool) {}
    bool canHandleAction(int, CvPlot*, bool) { return false; }
    bool canDoInterfaceMode(InterfaceModeTypes, CvSelectionGroup*) { return false; }
    CvPlot* getLookAtPlot() { return NULL; }
    CvPlot* getSelectionPlot() { return NULL; }
    CvUnit* getInterfacePlotUnit(const CvPlot*, int) { return NULL; }
    CvUnit* getSelectionUnit(int) { return NULL; }
    CvUnit* getHeadSelectedUnit() { return NULL; }
    void selectUnit(CvUnit*, bool, bool, bool) {}
    void selectGroup(CvUnit*, bool, bool, bool) {}
    void selectAll(CvPlot*) {}
    bool removeFromSelectionList(CvUnit*) { return false; }
    void makeSelectionListDirty() {}
    bool mirrorsSelectionGroup() { return false; }
    bool canSelectionListFound() { return false; }
    void bringToTop(CvPopup*) {}
    bool isPopupUp() { return false; }
    bool isPopupQueued() { return false; }
    bool isDiploOrPopupWaiting() { return false; }
    CvUnit* getLastSelectedUnit() { return NULL; }
    void setLastSelectedUnit(CvUnit*) {}
    void changePlotListColumn(int) {}
    CvPlot* getGotoPlot() { return NULL; }
    CvPlot* getSingleMoveGotoPlot() { return NULL; }
    CvPlot* getOriginalPlot() { return NULL; }
    void playGeneralSound(LPCTSTR, NiPoint3) {}
    void playGeneralSound(int, int, NiPoint3) {}
    void clearQueuedPopups() {}
    CvSelectionGroup* getSelectionList() { return NULL; }
    void clearSelectionList() {}
    void insertIntoSelectionList(CvUnit*, bool, bool, bool, bool, bool) {}
    void selectionListPostChange() {}
    void selectionListPreChange() {}
    int getSymbolID(int) { return 0; }
    CLLNode<IDInfo>* deleteSelectionListNode(CLLNode<IDInfo>*) { return NULL; }
    CLLNode<IDInfo>* nextSelectionListNode(CLLNode<IDInfo>*) { return NULL; }
    int getLengthSelectionList() { return 0; }
    CLLNode<IDInfo>* headSelectionListNode() { return NULL; }
    void selectCity(CvCity*, bool) {}
    void selectLookAtCity(bool) {}
    void addSelectedCity(CvCity*, bool) {}
    void clearSelectedCities() {}
    bool isCitySelected(CvCity*) { return false; }
    CvCity* getHeadSelectedCity() { return NULL; }
    bool isCitySelection() { return false; }
    CLLNode<IDInfo>* nextSelectedCitiesNode(CLLNode<IDInfo>*) { return NULL; }
    CLLNode<IDInfo>* headSelectedCitiesNode() { return NULL; }
    void addMessage(PlayerTypes, bool, int, CvWString, LPCTSTR,
        InterfaceMessageTypes, LPCSTR, ColorTypes,
        int, int, bool, bool) {}
    void addCombatMessage(PlayerTypes, CvWString) {}
    void addQuestMessage(PlayerTypes, CvWString, int) {}
    void showMessage(CvTalkingHeadMessage&) {}
    void flushTalkingHeadMessages() {}
    void clearEventMessages() {}
    void addPopup(CvPopupInfo*, PlayerTypes, bool, bool) {}
    void getDisplayedButtonPopups(CvPopupQueue&) {}
    int getCycleSelectionCounter() { return 0; }
    void setCycleSelectionCounter(int) {}
    void changeCycleSelectionCounter(int) {}
    int getEndTurnCounter() { return 0; }
    void setEndTurnCounter(int) {}
    void changeEndTurnCounter(int) {}
    bool isCombatFocus() { return false; }
    void setCombatFocus(bool) {}
    void setDiploQueue(CvDiploParameters*, PlayerTypes) {}
    bool isDirty(InterfaceDirtyBits) { return false; }
    void setDirty(InterfaceDirtyBits, bool) {}
    void makeInterfaceDirty() {}
    bool updateCursorType() { return false; }
    void updatePythonScreens() {}
    void lookAt(NiPoint3, CameraLookAtTypes, NiPoint3) {}
    void centerCamera(CvUnit*) {}
    void releaseLockedCamera() {}
    bool isFocusedWidget() { return false; }
    bool isFocused() { return false; }
    bool isBareMapMode() { return false; }
    void toggleBareMapMode() {}
    bool isShowYields() { return false; }
    void toggleYieldVisibleMode() {}
    bool isScoresVisible() { return false; }
    void toggleScoresVisible() {}
    bool isScoresMinimized() { return false; }
    void toggleScoresMinimized() {}
    bool isNetStatsVisible() { return false; }
    int getOriginalPlotCount() { return 0; }
    bool isCityScreenUp() { return false; }
    bool isEndTurnMessage() { return false; }
    void setInterfaceMode(InterfaceModeTypes) {}
    InterfaceModeTypes getInterfaceMode() { return (InterfaceModeTypes)0; }
    InterfaceVisibility getShowInterface() { return (InterfaceVisibility)0; }
    CvPlot* getMouseOverPlot() { return NULL; }
    void setFlashing(PlayerTypes, bool) {}
    bool isFlashing(PlayerTypes) { return false; }
    void setDiplomacyLocked(bool) {}
    bool isDiplomacyLocked() { return false; }
    void setMinimapColor(MinimapModeTypes, int, int, ColorTypes, float) {}
    unsigned char* getMinimapBaseTexture() const { return NULL; }
    void setEndTurnMessage(bool) {}
    bool isHasMovedUnit() { return false; }
    void setHasMovedUnit(bool) {}
    bool isForcePopup() { return false; }
    void setForcePopup(bool) {}
    void lookAtCityOffset(int) {}
    void toggleTurnLog() {}
    void showTurnLog(ChatTargetTypes) {}
    void dirtyTurnLog(PlayerTypes) {}
    int getPlotListColumn() { return 0; }
    void verifyPlotListColumn() {}
    int getPlotListOffset() { return 0; }
    void unlockPopupHelp() {}
    void showDetails(bool) {}
    void showAdminDetails() {}
    void toggleClockAlarm(bool, int, int) {}
    bool isClockAlarmOn() { return false; }
    void setScreenDying(int, bool) {}
    bool isExitingToMainMenu() { return false; }
    void exitingToMainMenu(const char*) {}
    void setWorldBuilder(bool) {}
    int getFontLeftJustify() { return DLL_FONT_LEFT_JUSTIFY; }
    int getFontRightJustify() { return DLL_FONT_RIGHT_JUSTIFY; }
    int getFontCenterJustify() { return DLL_FONT_CENTER_JUSTIFY; }
    int getFontCenterVertically() { return 0; }
    int getFontAdditive() { return 0; }
    void popupSetHeaderString(CvPopup*, CvWString, uint) {}
    void popupSetBodyString(CvPopup*, CvWString, uint, char*, CvWString) {}
    void popupLaunch(CvPopup*, bool, PopupStates, int) {}
    void popupSetPopupType(CvPopup*, PopupEventTypes, LPCTSTR) {}
    void popupSetStyle(CvPopup*, const char*) {}
    void popupAddDDS(CvPopup*, const char*, int, int, CvWString) {}
    void popupAddSeparator(CvPopup*, int) {}
    void popupAddGenericButton(CvPopup*, CvWString, const char*, int, WidgetTypes, int, int,
        bool, PopupControlLayout, unsigned int) {}
    void popupCreateEditBox(CvPopup*, CvWString, WidgetTypes, CvWString, int,
        PopupControlLayout, unsigned int, unsigned int) {}
    void popupEnableEditBox(CvPopup*, int, bool) {}
    void popupCreateRadioButtons(CvPopup*, int, int, WidgetTypes, PopupControlLayout) {}
    void popupSetRadioButtonText(CvPopup*, int, CvWString, int, CvWString) {}
    void popupCreateCheckBoxes(CvPopup*, int, int, WidgetTypes, PopupControlLayout) {}
    void popupSetCheckBoxText(CvPopup*, int, CvWString, int, CvWString) {}
    void popupSetCheckBoxState(CvPopup*, int, bool, int) {}
    void popupSetAsCancelled(CvPopup*) {}
    bool popupIsDying(CvPopup*) { return false; }
    void setCityTabSelectionRow(CityTabTypes) {}
    bool noTechSplash() { return false; }
    bool isInAdvancedStart() const { return false; }
    void setInAdvancedStart(bool) {}
    bool isSpaceshipScreenUp() const { return false; }
    bool isDebugMenuCreated() const { return false; }
    void setBusy(bool) {}
    void getInterfaceScreenIdsForInput(std::vector<int>&) {}
    void doPing(int, int, PlayerTypes) {}
};

// =============================================================================
// 4. CvDLLPythonIFaceBase — host/src/bridge/iface_python.h (12 methods)
// =============================================================================

class CvDLLPythonIFaceBase
{
public:
    virtual bool isInitialized() = 0;
    virtual const char* getMapScriptModule() = 0;
    virtual PyObject* MakeFunctionArgs(void** args, int argc) = 0;
    virtual bool moduleExists(const char* moduleName, bool bLoadIfNecessary) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, long* result) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, CvString* result) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, CvWString* result) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<byte>* pList) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<int>* pIntList) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, int* pIntList, int* iListSize) = 0;
    virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<float>* pFloatList) = 0;
    virtual bool callPythonFunction(const char* szModName, const char* szFxnName, int iArg, long* result) = 0;
    virtual bool pythonUsingDefaultImpl() = 0;
};

class CvDLLPythonIFaceImpl : public CvDLLPythonIFaceBase
{
public:
    bool isInitialized() { return false; }
    const char* getMapScriptModule() { return ""; }
    PyObject* MakeFunctionArgs(void**, int) { return NULL; }
    bool moduleExists(const char*, bool) { return false; }
    bool callFunction(const char*, const char*, void*) { return false; }
    bool callFunction(const char*, const char*, void*, long*) { return false; }
    bool callFunction(const char*, const char*, void*, CvString*) { return false; }
    bool callFunction(const char*, const char*, void*, CvWString*) { return false; }
    bool callFunction(const char*, const char*, void*, std::vector<byte>*) { return false; }
    bool callFunction(const char*, const char*, void*, std::vector<int>*) { return false; }
    bool callFunction(const char*, const char*, void*, int*, int*) { return false; }
    bool callFunction(const char*, const char*, void*, std::vector<float>*) { return false; }
    bool callPythonFunction(const char*, const char*, int, long*) { return false; }
    bool pythonUsingDefaultImpl() { return true; }
};

// =============================================================================
// 5. CvDLLFAStarIFaceBase — host/src/bridge/iface_fastar.h (14 methods)
// =============================================================================

class CvDLLFAStarIFaceBase
{
public:
    virtual FAStar* create() = 0;
    virtual void destroy(FAStar*& ptr, bool bSafeDelete) = 0;
    virtual bool GeneratePath(FAStar*, int iXstart, int iYstart, int iXdest, int iYdest, bool bCardinalOnly, int iInfo, bool bReuse) = 0;
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

class CvDLLFAStarIFaceImpl : public CvDLLFAStarIFaceBase
{
public:
    FAStar* create() { return NULL; }
    void destroy(FAStar*& ptr, bool) { ptr = NULL; }
    bool GeneratePath(FAStar*, int, int, int, int, bool, int, bool) { return false; }
    void Initialize(FAStar*, int, int, bool, bool, FAPointFunc, FAHeuristic, FAStarFunc, FAStarFunc, FAStarFunc, FAStarFunc, void*) {}
    void SetData(FAStar*, const void*) {}
    FAStarNode* GetLastNode(FAStar*) { return NULL; }
    bool IsPathStart(FAStar*, int, int) { return false; }
    bool IsPathDest(FAStar*, int, int) { return false; }
    int GetStartX(FAStar*) { return 0; }
    int GetStartY(FAStar*) { return 0; }
    int GetDestX(FAStar*) { return 0; }
    int GetDestY(FAStar*) { return 0; }
    int GetInfo(FAStar*) { return 0; }
    void ForceReset(FAStar*) {}
};

// =============================================================================
// 6. CvDLLIniParserIFaceBase — host/src/bridge/iface_ini_parser.h (8 methods)
// =============================================================================

class CvDLLIniParserIFaceBase
{
public:
    virtual FIniParser* create(const char* szFile) = 0;
    virtual void destroy(FIniParser*& pParser, bool bSafeDelete) = 0;
    virtual bool SetGroupKey(FIniParser* pParser, const LPCTSTR pGroupKey) = 0;
    virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, bool* iValue) = 0;
    virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, short* iValue) = 0;
    virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, int* iValue) = 0;
    virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, float* fValue) = 0;
    virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, LPTSTR szValue) = 0;
};

class CvDLLIniParserIFaceImpl : public CvDLLIniParserIFaceBase
{
public:
    FIniParser* create(const char*) { return NULL; }
    void destroy(FIniParser*& pParser, bool) { pParser = NULL; }
    bool SetGroupKey(FIniParser*, const LPCTSTR) { return false; }
    bool GetKeyValue(FIniParser*, const LPCTSTR, bool*) { return false; }
    bool GetKeyValue(FIniParser*, const LPCTSTR, short*) { return false; }
    bool GetKeyValue(FIniParser*, const LPCTSTR, int*) { return false; }
    bool GetKeyValue(FIniParser*, const LPCTSTR, float*) { return false; }
    bool GetKeyValue(FIniParser*, const LPCTSTR, LPTSTR) { return false; }
};

// =============================================================================
// 7. CvDLLSymbolIFaceBase — host/src/bridge/iface_symbol.h (11 methods)
// =============================================================================

class CvDLLSymbolIFaceBase
{
public:
    virtual void init(CvSymbol*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
    virtual CvSymbol* createSymbol() = 0;
    virtual void destroy(CvSymbol*&, bool bSafeDelete) = 0;
    virtual void setAlpha(CvSymbol*, float fAlpha) = 0;
    virtual void setScale(CvSymbol*, float fScale) = 0;
    virtual void Hide(CvSymbol*, bool bHide) = 0;
    virtual bool IsHidden(CvSymbol*) = 0;
    virtual void updatePosition(CvSymbol*) = 0;
    virtual int getID(CvSymbol*) = 0;
    virtual SymbolTypes getSymbol(CvSymbol* pSym) = 0;
    virtual void setTypeYield(CvSymbol*, int iType, int count) = 0;
};

class CvDLLSymbolIFaceImpl : public CvDLLSymbolIFaceBase
{
public:
    void init(CvSymbol*, int, int, int, CvPlot*) {}
    CvSymbol* createSymbol() { return NULL; }
    void destroy(CvSymbol*&, bool) {}
    void setAlpha(CvSymbol*, float) {}
    void setScale(CvSymbol*, float) {}
    void Hide(CvSymbol*, bool) {}
    bool IsHidden(CvSymbol*) { return false; }
    void updatePosition(CvSymbol*) {}
    int getID(CvSymbol*) { return 0; }
    SymbolTypes getSymbol(CvSymbol*) { return (SymbolTypes)0; }
    void setTypeYield(CvSymbol*, int, int) {}
};

// =============================================================================
// 8. CvDLLFeatureIFaceBase — host/src/bridge/iface_feature.h (12 methods)
// =============================================================================

class CvDLLFeatureIFaceBase
{
public:
    virtual CvFeature* createFeature() = 0;
    virtual void init(CvFeature*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
    virtual FeatureTypes getFeature(CvFeature* pObj) = 0;
    virtual void setDummyVisibility(CvFeature* feature, const char* dummyTag, bool show) = 0;
    virtual void addDummyModel(CvFeature* feature, const char* dummyTag, const char* modelTag) = 0;
    virtual void setDummyTexture(CvFeature* feature, const char* dummyTag, const char* textureTag) = 0;
    virtual CvString pickDummyTag(CvFeature* feature, int mouseX, int mouseY) = 0;
    virtual void resetModel(CvFeature* feature) = 0;
    virtual void destroy(CvFeature*& pObj, bool bSafeDelete) = 0;
    virtual void Hide(CvFeature* pObj, bool bHide) = 0;
    virtual bool IsHidden(CvFeature* pObj) = 0;
    virtual void updatePosition(CvFeature* pObj) = 0;
};

class CvDLLFeatureIFaceImpl : public CvDLLFeatureIFaceBase
{
public:
    CvFeature* createFeature() { return NULL; }
    void init(CvFeature*, int, int, int, CvPlot*) {}
    FeatureTypes getFeature(CvFeature*) { return (FeatureTypes)0; }
    void setDummyVisibility(CvFeature*, const char*, bool) {}
    void addDummyModel(CvFeature*, const char*, const char*) {}
    void setDummyTexture(CvFeature*, const char*, const char*) {}
    CvString pickDummyTag(CvFeature*, int, int) { return CvString(); }
    void resetModel(CvFeature*) {}
    void destroy(CvFeature*&, bool) {}
    void Hide(CvFeature*, bool) {}
    bool IsHidden(CvFeature*) { return false; }
    void updatePosition(CvFeature*) {}
};

// =============================================================================
// 9. CvDLLRouteIFaceBase — host/src/bridge/iface_route.h (9 methods)
// =============================================================================

class CvDLLRouteIFaceBase
{
public:
    virtual CvRoute* createRoute() = 0;
    virtual void init(CvRoute*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
    virtual RouteTypes getRoute(CvRoute* pObj) = 0;
    virtual void destroy(CvRoute*& pObj, bool bSafeDelete) = 0;
    virtual void Hide(CvRoute* pObj, bool bHide) = 0;
    virtual bool IsHidden(CvRoute* pObj) = 0;
    virtual void updatePosition(CvRoute* pObj) = 0;
    virtual int getConnectionMask(CvRoute* pObj) = 0;
    virtual void updateGraphicEra(CvRoute* pObj) = 0;
};

class CvDLLRouteIFaceImpl : public CvDLLRouteIFaceBase
{
public:
    CvRoute* createRoute() { return NULL; }
    void init(CvRoute*, int, int, int, CvPlot*) {}
    RouteTypes getRoute(CvRoute*) { return (RouteTypes)0; }
    void destroy(CvRoute*&, bool) {}
    void Hide(CvRoute*, bool) {}
    bool IsHidden(CvRoute*) { return false; }
    void updatePosition(CvRoute*) {}
    int getConnectionMask(CvRoute*) { return 0; }
    void updateGraphicEra(CvRoute*) {}
};

// =============================================================================
// 10. CvDLLRiverIFaceBase — host/src/bridge/iface_river.h (6 methods)
// =============================================================================

class CvDLLRiverIFaceBase
{
public:
    virtual CvRiver* createRiver() = 0;
    virtual void init(CvRiver*, int iID, int iOffset, int iType, CvPlot* pPlot) = 0;
    virtual void destroy(CvRiver*& pObj, bool bSafeDelete) = 0;
    virtual void Hide(CvRiver* pObj, bool bHide) = 0;
    virtual bool IsHidden(CvRiver* pObj) = 0;
    virtual void updatePosition(CvRiver* pObj) = 0;
};

class CvDLLRiverIFaceImpl : public CvDLLRiverIFaceBase
{
public:
    CvRiver* createRiver() { return NULL; }
    void init(CvRiver*, int, int, int, CvPlot*) {}
    void destroy(CvRiver*&, bool) {}
    void Hide(CvRiver*, bool) {}
    bool IsHidden(CvRiver*) { return false; }
    void updatePosition(CvRiver*) {}
};

// =============================================================================
// 11. CvDLLFlagEntityIFaceBase — host/src/bridge/iface_flag_entity.h
//     Inherits from CvDLLEntityIFaceBase (26 inherited + 8 own = 34 methods)
//     Base class CvDLLEntityIFaceBase is defined in relay_types.h.
// =============================================================================

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
    virtual void destroy(CvFlagEntity*& pImp, bool bSafeDelete) = 0;
};

class CvDLLFlagEntityIFaceImpl : public CvDLLFlagEntityIFaceBase
{
public:
    // --- Inherited CvDLLEntityIFaceBase methods (26 total) ---
    void removeEntity(CvEntity*) {}
    void addEntity(CvEntity*, uint) {}
    void setup(CvEntity*) {}
    void setVisible(CvEntity*, bool) {}
    void createCityEntity(CvCity*) {}
    void createUnitEntity(CvUnit*) {}
    void destroyEntity(CvEntity*&, bool) {}
    void updatePosition(CvEntity*) {}
    void setupFloodPlains(CvRiver*) {}
    bool IsSelected(const CvEntity*) const { return false; }
    void PlayAnimation(CvEntity*, AnimationTypes, float, bool, int, float, float) {}
    void StopAnimation(CvEntity*, AnimationTypes) {}
    void StopAnimation(CvEntity*) {}
    void NotifyEntity(CvUnitEntity*, MissionTypes) {}
    void MoveTo(CvUnitEntity*, const CvPlot*) {}
    void QueueMove(CvUnitEntity*, const CvPlot*) {}
    void ExecuteMove(CvUnitEntity*, float, bool) {}
    void SetPosition(CvUnitEntity*, const CvPlot*) {}
    void AddMission(const CvMissionDefinition*) {}
    void RemoveUnitFromBattle(CvUnit*) {}
    void showPromotionGlow(CvUnitEntity*, bool) {}
    void updateEnemyGlow(CvUnitEntity*) {}
    void updatePromotionLayers(CvUnitEntity*) {}
    void updateGraphicEra(CvUnitEntity*, EraTypes) {}
    void SetSiegeTower(CvUnitEntity*, bool) {}
    bool GetSiegeTower(CvUnitEntity*) { return false; }

    // --- Own CvDLLFlagEntityIFaceBase methods (7 total + destroy) ---
    CvFlagEntity* create(PlayerTypes) { return NULL; }
    PlayerTypes getPlayer(CvFlagEntity*) const { return NO_PLAYER; }
    CvPlot* getPlot(CvFlagEntity*) const { return NULL; }
    void setPlot(CvFlagEntity*, CvPlot*, bool) {}
    void updateUnitInfo(CvFlagEntity*, const CvPlot*, bool) {}
    void updateGraphicEra(CvFlagEntity*) {}
    void setVisible(CvFlagEntity*, bool) {}
    void destroy(CvFlagEntity*&, bool) {}
};

// =============================================================================
// 12. CvDLLPlotBuilderIFaceBase — host/src/bridge/iface_plot_builder.h
//     Inherits from CvDLLEntityIFaceBase (26 inherited + 3 own = 29 methods)
//     Base class CvDLLEntityIFaceBase is defined in relay_types.h.
// =============================================================================

class CvDLLPlotBuilderIFaceBase : public CvDLLEntityIFaceBase
{
public:
    virtual void init(CvPlotBuilder*, CvPlot*) = 0;
    virtual CvPlotBuilder* create() = 0;
    virtual void destroy(CvPlotBuilder*& pPlotBuilder, bool bSafeDelete) = 0;
};

class CvDLLPlotBuilderIFaceImpl : public CvDLLPlotBuilderIFaceBase
{
public:
    // --- Inherited CvDLLEntityIFaceBase methods (26 total) ---
    void removeEntity(CvEntity*) {}
    void addEntity(CvEntity*, uint) {}
    void setup(CvEntity*) {}
    void setVisible(CvEntity*, bool) {}
    void createCityEntity(CvCity*) {}
    void createUnitEntity(CvUnit*) {}
    void destroyEntity(CvEntity*&, bool) {}
    void updatePosition(CvEntity*) {}
    void setupFloodPlains(CvRiver*) {}
    bool IsSelected(const CvEntity*) const { return false; }
    void PlayAnimation(CvEntity*, AnimationTypes, float, bool, int, float, float) {}
    void StopAnimation(CvEntity*, AnimationTypes) {}
    void StopAnimation(CvEntity*) {}
    void NotifyEntity(CvUnitEntity*, MissionTypes) {}
    void MoveTo(CvUnitEntity*, const CvPlot*) {}
    void QueueMove(CvUnitEntity*, const CvPlot*) {}
    void ExecuteMove(CvUnitEntity*, float, bool) {}
    void SetPosition(CvUnitEntity*, const CvPlot*) {}
    void AddMission(const CvMissionDefinition*) {}
    void RemoveUnitFromBattle(CvUnit*) {}
    void showPromotionGlow(CvUnitEntity*, bool) {}
    void updateEnemyGlow(CvUnitEntity*) {}
    void updatePromotionLayers(CvUnitEntity*) {}
    void updateGraphicEra(CvUnitEntity*, EraTypes) {}
    void SetSiegeTower(CvUnitEntity*, bool) {}
    bool GetSiegeTower(CvUnitEntity*) { return false; }

    // --- Own CvDLLPlotBuilderIFaceBase methods (3 total) ---
    void init(CvPlotBuilder*, CvPlot*) {}
    CvPlotBuilder* create() { return NULL; }
    void destroy(CvPlotBuilder*&, bool) {}
};

// =============================================================================
// 13. CvDLLEventReporterIFaceBase — host/src/bridge/iface_event_reporter.h
//     (65 methods)
// =============================================================================

class CvDLLEventReporterIFaceBase
{
public:
    virtual void genericEvent(const char* szEventName, void* pythonArgs) = 0;
    virtual void mouseEvent(int evt, const POINT& ptCursor) = 0;
    virtual void kbdEvent(int evt, int key) = 0;
    virtual void gameEnd() = 0;
    virtual void beginGameTurn(int iGameTurn) = 0;
    virtual void endGameTurn(int iGameTurn) = 0;
    virtual void beginPlayerTurn(int iGameTurn, PlayerTypes) = 0;
    virtual void endPlayerTurn(int iGameTurn, PlayerTypes) = 0;
    virtual void firstContact(TeamTypes eTeamID1, TeamTypes eTeamID2) = 0;
    virtual void combatResult(CvUnit* pWinner, CvUnit* pLoser) = 0;
    virtual void improvementBuilt(int iImprovementType, int iX, int iY) = 0;
    virtual void improvementDestroyed(int iImprovementType, int iPlayer, int iX, int iY) = 0;
    virtual void routeBuilt(int RouteType, int iX, int iY) = 0;
    virtual void plotRevealed(CvPlot* pPlot, TeamTypes eTeam) = 0;
    virtual void plotFeatureRemoved(CvPlot* pPlot, FeatureTypes eFeature, CvCity* pCity) = 0;
    virtual void plotPicked(CvPlot* pPlot) = 0;
    virtual void nukeExplosion(CvPlot* pPlot, CvUnit* pNukeUnit) = 0;
    virtual void gotoPlotSet(CvPlot* pPlot, PlayerTypes ePlayer) = 0;
    virtual void cityBuilt(CvCity* pCity) = 0;
    virtual void cityRazed(CvCity* pCity, PlayerTypes ePlayer) = 0;
    virtual void cityAcquired(PlayerTypes eOldOwner, PlayerTypes ePlayer, CvCity* pCity, bool bConquest, bool bTrade) = 0;
    virtual void cityAcquiredAndKept(PlayerTypes ePlayer, CvCity* pCity) = 0;
    virtual void cityLost(CvCity* pCity) = 0;
    virtual void cultureExpansion(CvCity* pCity, PlayerTypes ePlayer) = 0;
    virtual void cityGrowth(CvCity* pCity, PlayerTypes ePlayer) = 0;
    virtual void cityDoTurn(CvCity* pCity, PlayerTypes ePlayer) = 0;
    virtual void cityBuildingUnit(CvCity* pCity, UnitTypes eUnitType) = 0;
    virtual void cityBuildingBuilding(CvCity* pCity, BuildingTypes eBuildingType) = 0;
    virtual void cityRename(CvCity* pCity) = 0;
    virtual void cityHurry(CvCity* pCity, HurryTypes eHurry) = 0;
    virtual void selectionGroupPushMission(CvSelectionGroup* pSelectionGroup, MissionTypes eMission) = 0;
    virtual void unitMove(CvPlot* pPlot, CvUnit* pUnit, CvPlot* pOldPlot) = 0;
    virtual void unitSetXY(CvPlot* pPlot, CvUnit* pUnit) = 0;
    virtual void unitCreated(CvUnit* pUnit) = 0;
    virtual void unitBuilt(CvCity* pCity, CvUnit* pUnit) = 0;
    virtual void unitKilled(CvUnit* pUnit, PlayerTypes eAttacker) = 0;
    virtual void unitLost(CvUnit* pUnit) = 0;
    virtual void unitPromoted(CvUnit* pUnit, PromotionTypes ePromotion) = 0;
    virtual void unitSelected(CvUnit* pUnit) = 0;
    virtual void unitRename(CvUnit* pUnit) = 0;
    virtual void unitPillage(CvUnit* pUnit, ImprovementTypes eImprovement, RouteTypes eRoute, PlayerTypes ePlayer) = 0;
    virtual void unitSpreadReligionAttempt(CvUnit* pUnit, ReligionTypes eReligion, bool bSuccess) = 0;
    virtual void unitGifted(CvUnit* pUnit, PlayerTypes eGiftingPlayer, CvPlot* pPlotLocation) = 0;
    virtual void unitBuildImprovement(CvUnit* pUnit, BuildTypes eBuild, bool bFinished) = 0;
    virtual void goodyReceived(PlayerTypes ePlayer, CvPlot* pGoodyPlot, CvUnit* pGoodyUnit, GoodyTypes eGoodyType) = 0;
    virtual void greatPersonBorn(CvUnit* pUnit, PlayerTypes ePlayer, CvCity* pCity) = 0;
    virtual void buildingBuilt(CvCity* pCity, BuildingTypes eBuilding) = 0;
    virtual void projectBuilt(CvCity* pCity, ProjectTypes eProject) = 0;
    virtual void techAcquired(TechTypes eType, TeamTypes eTeam, PlayerTypes ePlayer, bool bAnnounce) = 0;
    virtual void techSelected(TechTypes eTech, PlayerTypes ePlayer) = 0;
    virtual void religionFounded(ReligionTypes eType, PlayerTypes ePlayer) = 0;
    virtual void religionSpread(ReligionTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) = 0;
    virtual void religionRemove(ReligionTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) = 0;
    virtual void corporationFounded(CorporationTypes eType, PlayerTypes ePlayer) = 0;
    virtual void corporationSpread(CorporationTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) = 0;
    virtual void corporationRemove(CorporationTypes eType, PlayerTypes ePlayer, CvCity* pSpreadCity) = 0;
    virtual void goldenAge(PlayerTypes ePlayer) = 0;
    virtual void endGoldenAge(PlayerTypes ePlayer) = 0;
    virtual void changeWar(bool bWar, TeamTypes eTeam, TeamTypes eOtherTeam) = 0;
    virtual void setPlayerAlive(PlayerTypes ePlayerID, bool bNewValue) = 0;
    virtual void playerChangeStateReligion(PlayerTypes ePlayerID, ReligionTypes eNewReligion, ReligionTypes eOldReligion) = 0;
    virtual void playerGoldTrade(PlayerTypes eFromPlayer, PlayerTypes eToPlayer, int iAmount) = 0;
    virtual void chat(char* szString) = 0;
    virtual void victory(TeamTypes eNewWinner, VictoryTypes eNewVictory) = 0;
    virtual void vassalState(TeamTypes eMaster, TeamTypes eVassal, bool bVassal) = 0;
};

class CvDLLEventReporterIFaceImpl : public CvDLLEventReporterIFaceBase
{
public:
    void genericEvent(const char*, void*) {}
    void mouseEvent(int, const POINT&) {}
    void kbdEvent(int, int) {}
    void gameEnd() {}
    void beginGameTurn(int) {}
    void endGameTurn(int) {}
    void beginPlayerTurn(int, PlayerTypes) {}
    void endPlayerTurn(int, PlayerTypes) {}
    void firstContact(TeamTypes, TeamTypes) {}
    void combatResult(CvUnit*, CvUnit*) {}
    void improvementBuilt(int, int, int) {}
    void improvementDestroyed(int, int, int, int) {}
    void routeBuilt(int, int, int) {}
    void plotRevealed(CvPlot*, TeamTypes) {}
    void plotFeatureRemoved(CvPlot*, FeatureTypes, CvCity*) {}
    void plotPicked(CvPlot*) {}
    void nukeExplosion(CvPlot*, CvUnit*) {}
    void gotoPlotSet(CvPlot*, PlayerTypes) {}
    void cityBuilt(CvCity*) {}
    void cityRazed(CvCity*, PlayerTypes) {}
    void cityAcquired(PlayerTypes, PlayerTypes, CvCity*, bool, bool) {}
    void cityAcquiredAndKept(PlayerTypes, CvCity*) {}
    void cityLost(CvCity*) {}
    void cultureExpansion(CvCity*, PlayerTypes) {}
    void cityGrowth(CvCity*, PlayerTypes) {}
    void cityDoTurn(CvCity*, PlayerTypes) {}
    void cityBuildingUnit(CvCity*, UnitTypes) {}
    void cityBuildingBuilding(CvCity*, BuildingTypes) {}
    void cityRename(CvCity*) {}
    void cityHurry(CvCity*, HurryTypes) {}
    void selectionGroupPushMission(CvSelectionGroup*, MissionTypes) {}
    void unitMove(CvPlot*, CvUnit*, CvPlot*) {}
    void unitSetXY(CvPlot*, CvUnit*) {}
    void unitCreated(CvUnit*) {}
    void unitBuilt(CvCity*, CvUnit*) {}
    void unitKilled(CvUnit*, PlayerTypes) {}
    void unitLost(CvUnit*) {}
    void unitPromoted(CvUnit*, PromotionTypes) {}
    void unitSelected(CvUnit*) {}
    void unitRename(CvUnit*) {}
    void unitPillage(CvUnit*, ImprovementTypes, RouteTypes, PlayerTypes) {}
    void unitSpreadReligionAttempt(CvUnit*, ReligionTypes, bool) {}
    void unitGifted(CvUnit*, PlayerTypes, CvPlot*) {}
    void unitBuildImprovement(CvUnit*, BuildTypes, bool) {}
    void goodyReceived(PlayerTypes, CvPlot*, CvUnit*, GoodyTypes) {}
    void greatPersonBorn(CvUnit*, PlayerTypes, CvCity*) {}
    void buildingBuilt(CvCity*, BuildingTypes) {}
    void projectBuilt(CvCity*, ProjectTypes) {}
    void techAcquired(TechTypes, TeamTypes, PlayerTypes, bool) {}
    void techSelected(TechTypes, PlayerTypes) {}
    void religionFounded(ReligionTypes, PlayerTypes) {}
    void religionSpread(ReligionTypes, PlayerTypes, CvCity*) {}
    void religionRemove(ReligionTypes, PlayerTypes, CvCity*) {}
    void corporationFounded(CorporationTypes, PlayerTypes) {}
    void corporationSpread(CorporationTypes, PlayerTypes, CvCity*) {}
    void corporationRemove(CorporationTypes, PlayerTypes, CvCity*) {}
    void goldenAge(PlayerTypes) {}
    void endGoldenAge(PlayerTypes) {}
    void changeWar(bool, TeamTypes, TeamTypes) {}
    void setPlayerAlive(PlayerTypes, bool) {}
    void playerChangeStateReligion(PlayerTypes, ReligionTypes, ReligionTypes) {}
    void playerGoldTrade(PlayerTypes, PlayerTypes, int) {}
    void chat(char*) {}
    void victory(TeamTypes, VictoryTypes) {}
    void vassalState(TeamTypes, TeamTypes, bool) {}
};

// =============================================================================
// 14. FDataStreamBase — host/src/bridge/iface_data_stream.h (100 methods)
//     Read/Write overloads for all primitive types
// =============================================================================
// NOTE: FDataStreamBase is forward-declared in relay_types.h. The full class
// definition is here since relay_types.h cannot include it (circular deps).

class FDataStreamBase
{
public:
    virtual void Rewind() = 0;
    virtual bool AtEnd() = 0;
    virtual void FastFwd() = 0;
    virtual unsigned int GetPosition() const = 0;
    virtual void SetPosition(unsigned int position) = 0;
    virtual void Truncate() = 0;
    virtual void Flush() = 0;
    virtual unsigned int GetEOF() const = 0;
    virtual unsigned int GetSizeLeft() const = 0;
    virtual void CopyToMem(void* mem) = 0;

    // WriteString overloads
    virtual unsigned int WriteString(const wchar* szName) = 0;
    virtual unsigned int WriteString(const char* szName) = 0;
    virtual unsigned int WriteString(const std::string& szName) = 0;
    virtual unsigned int WriteString(const std::wstring& szName) = 0;
    virtual unsigned int WriteString(int count, std::string values[]) = 0;
    virtual unsigned int WriteString(int count, std::wstring values[]) = 0;

    // ReadString overloads
    virtual unsigned int ReadString(char* szName) = 0;
    virtual unsigned int ReadString(wchar* szName) = 0;
    virtual unsigned int ReadString(std::string& szName) = 0;
    virtual unsigned int ReadString(std::wstring& szName) = 0;
    virtual unsigned int ReadString(int count, std::string values[]) = 0;
    virtual unsigned int ReadString(int count, std::wstring values[]) = 0;

    virtual char* ReadString() = 0;
    virtual wchar* ReadWideString() = 0;

    // Read overloads
    virtual void Read(char*) = 0;
    virtual void Read(byte*) = 0;
    virtual void Read(int count, char values[]) = 0;
    virtual void Read(int count, byte values[]) = 0;
    virtual void Read(bool*) = 0;
    virtual void Read(int count, bool values[]) = 0;
    virtual void Read(short* s) = 0;
    virtual void Read(unsigned short* s) = 0;
    virtual void Read(int count, short values[]) = 0;
    virtual void Read(int count, unsigned short values[]) = 0;
    virtual void Read(int* i) = 0;
    virtual void Read(unsigned int* i) = 0;
    virtual void Read(int count, int values[]) = 0;
    virtual void Read(int count, unsigned int values[]) = 0;
    virtual void Read(long* l) = 0;
    virtual void Read(unsigned long* l) = 0;
    virtual void Read(int count, long values[]) = 0;
    virtual void Read(int count, unsigned long values[]) = 0;
    virtual void Read(float* value) = 0;
    virtual void Read(int count, float values[]) = 0;
    virtual void Read(double* value) = 0;
    virtual void Read(int count, double values[]) = 0;

    // Write overloads
    virtual void Write(char value) = 0;
    virtual void Write(byte value) = 0;
    virtual void Write(int count, const char values[]) = 0;
    virtual void Write(int count, const byte values[]) = 0;
    virtual void Write(bool value) = 0;
    virtual void Write(int count, const bool values[]) = 0;
    virtual void Write(short value) = 0;
    virtual void Write(unsigned short value) = 0;
    virtual void Write(int count, const short values[]) = 0;
    virtual void Write(int count, const unsigned short values[]) = 0;
    virtual void Write(int value) = 0;
    virtual void Write(unsigned int value) = 0;
    virtual void Write(int count, const int values[]) = 0;
    virtual void Write(int count, const unsigned int values[]) = 0;
    virtual void Write(long value) = 0;
    virtual void Write(unsigned long value) = 0;
    virtual void Write(int count, const long values[]) = 0;
    virtual void Write(int count, const unsigned long values[]) = 0;
    virtual void Write(float value) = 0;
    virtual void Write(int count, const float values[]) = 0;
    virtual void Write(double value) = 0;
    virtual void Write(int count, const double values[]) = 0;
};

class FDataStreamImpl : public FDataStreamBase
{
public:
    void Rewind() {}
    bool AtEnd() { return true; }
    void FastFwd() {}
    unsigned int GetPosition() const { return 0; }
    void SetPosition(unsigned int) {}
    void Truncate() {}
    void Flush() {}
    unsigned int GetEOF() const { return 0; }
    unsigned int GetSizeLeft() const { return 0; }
    void CopyToMem(void*) {}

    unsigned int WriteString(const wchar*) { return 0; }
    unsigned int WriteString(const char*) { return 0; }
    unsigned int WriteString(const std::string&) { return 0; }
    unsigned int WriteString(const std::wstring&) { return 0; }
    unsigned int WriteString(int, std::string[]) { return 0; }
    unsigned int WriteString(int, std::wstring[]) { return 0; }

    unsigned int ReadString(char*) { return 0; }
    unsigned int ReadString(wchar*) { return 0; }
    unsigned int ReadString(std::string&) { return 0; }
    unsigned int ReadString(std::wstring&) { return 0; }
    unsigned int ReadString(int, std::string[]) { return 0; }
    unsigned int ReadString(int, std::wstring[]) { return 0; }

    char* ReadString() { return NULL; }
    wchar* ReadWideString() { return NULL; }

    void Read(char*) {}
    void Read(byte*) {}
    void Read(int, char[]) {}
    void Read(int, byte[]) {}
    void Read(bool*) {}
    void Read(int, bool[]) {}
    void Read(short*) {}
    void Read(unsigned short*) {}
    void Read(int, short[]) {}
    void Read(int, unsigned short[]) {}
    void Read(int*) {}
    void Read(unsigned int*) {}
    void Read(int, int[]) {}
    void Read(int, unsigned int[]) {}
    void Read(long*) {}
    void Read(unsigned long*) {}
    void Read(int, long[]) {}
    void Read(int, unsigned long[]) {}
    void Read(float*) {}
    void Read(int, float[]) {}
    void Read(double*) {}
    void Read(int, double[]) {}

    void Write(char) {}
    void Write(byte) {}
    void Write(int, const char[]) {}
    void Write(int, const byte[]) {}
    void Write(bool) {}
    void Write(int, const bool[]) {}
    void Write(short) {}
    void Write(unsigned short) {}
    void Write(int, const short[]) {}
    void Write(int, const unsigned short[]) {}
    void Write(int) {}
    void Write(unsigned int) {}
    void Write(int, const int[]) {}
    void Write(int, const unsigned int[]) {}
    void Write(long) {}
    void Write(unsigned long) {}
    void Write(int, const long[]) {}
    void Write(int, const unsigned long[]) {}
    void Write(float) {}
    void Write(int, const float[]) {}
    void Write(double) {}
    void Write(int, const double[]) {}
};

// =============================================================================
// Static singleton instances
// =============================================================================

static CvDLLEngineIFaceImpl        s_engineIFace;
static CvDLLEntityIFaceImpl        s_entityIFace;
static CvDLLInterfaceIFaceImpl     s_interfaceIFace;
static CvDLLPythonIFaceImpl        s_pythonIFace;
static CvDLLFAStarIFaceImpl        s_fastarIFace;
static CvDLLIniParserIFaceImpl     s_iniParserIFace;
static CvDLLSymbolIFaceImpl        s_symbolIFace;
static CvDLLFeatureIFaceImpl       s_featureIFace;
static CvDLLRouteIFaceImpl         s_routeIFace;
static CvDLLRiverIFaceImpl         s_riverIFace;
static CvDLLFlagEntityIFaceImpl    s_flagEntityIFace;
static CvDLLPlotBuilderIFaceImpl   s_plotBuilderIFace;
static CvDLLEventReporterIFaceImpl s_eventReporterIFace;
static FDataStreamImpl             s_dataStream;

// =============================================================================
// Global pointers (referenced by relay_utility.cpp)
// =============================================================================

CvDLLEngineIFaceBase*        g_pRelayEngineIFace       = &s_engineIFace;
CvDLLEntityIFaceBase*        g_pRelayEntityIFace       = &s_entityIFace;
CvDLLInterfaceIFaceBase*     g_pRelayInterfaceIFace    = &s_interfaceIFace;
CvDLLPythonIFaceBase*        g_pRelayPythonIFace       = &s_pythonIFace;
CvDLLFAStarIFaceBase*        g_pRelayFAStarIFace       = &s_fastarIFace;
CvDLLIniParserIFaceBase*     g_pRelayIniParserIFace    = &s_iniParserIFace;
CvDLLSymbolIFaceBase*        g_pRelaySymbolIFace       = &s_symbolIFace;
CvDLLFeatureIFaceBase*       g_pRelayFeatureIFace      = &s_featureIFace;
CvDLLRouteIFaceBase*         g_pRelayRouteIFace        = &s_routeIFace;
CvDLLRiverIFaceBase*         g_pRelayRiverIFace        = &s_riverIFace;
CvDLLFlagEntityIFaceBase*    g_pRelayFlagEntityIFace   = &s_flagEntityIFace;
CvDLLPlotBuilderIFaceBase*   g_pRelayPlotBuilderIFace  = &s_plotBuilderIFace;
CvDLLEventReporterIFaceBase* g_pRelayEventReporterIFace = &s_eventReporterIFace;
