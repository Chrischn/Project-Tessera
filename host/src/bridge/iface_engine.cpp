// =============================================================================
// File:              iface_engine.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLEngineIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_engine.h"
#include <cstdio>

class CvDLLEngineIFaceImpl : public CvDLLEngineIFaceBase
{
public:
	// =========================================================================
	// Camera
	// =========================================================================

	void cameraLookAt(NiPoint3 lookingPoint) override {
		fprintf(stderr, "[ENGINE STUB] cameraLookAt\n");
	}

	bool isCameraLocked() override {
		fprintf(stderr, "[ENGINE STUB] isCameraLocked\n");
		return false;
	}

	// =========================================================================
	// Misc commands
	// =========================================================================

	void SetObeyEntityVisibleFlags(bool bObeyHide) override {
		fprintf(stderr, "[ENGINE STUB] SetObeyEntityVisibleFlags\n");
	}

	void AutoSave(bool bInitial) override {
		fprintf(stderr, "[ENGINE STUB] AutoSave\n");
	}

	void SaveReplay(PlayerTypes ePlayer) override {
		fprintf(stderr, "[ENGINE STUB] SaveReplay\n");
	}

	void SaveGame(CvString& szFilename, SaveGameTypes eType) override {
		fprintf(stderr, "[ENGINE STUB] SaveGame\n");
	}

	void DoTurn() override {
		fprintf(stderr, "[ENGINE STUB] DoTurn\n");
	}

	void ClearMinimap() override {
		fprintf(stderr, "[ENGINE STUB] ClearMinimap\n");
	}

	byte GetLandscapePlotTerrainData(uint uiX, uint uiY, uint uiPointX, uint uiPointY) override {
		fprintf(stderr, "[ENGINE STUB] GetLandscapePlotTerrainData\n");
		return 0;
	}

	byte GetLandscapePlotHeightData(uint uiX, uint uiY, uint uiPointX, uint uiPointY) override {
		fprintf(stderr, "[ENGINE STUB] GetLandscapePlotHeightData\n");
		return 0;
	}

	LoadType getLoadType() override {
		fprintf(stderr, "[ENGINE STUB] getLoadType\n");
		return LoadType{};
	}

	void ClampToWorldCoords(NiPoint3* pPt3, float fOffset) override {
		fprintf(stderr, "[ENGINE STUB] ClampToWorldCoords\n");
	}

	void SetCameraZoom(float zoom) override {
		fprintf(stderr, "[ENGINE STUB] SetCameraZoom\n");
	}

	float GetUpdateRate() override {
		fprintf(stderr, "[ENGINE STUB] GetUpdateRate\n");
		return 0.0f;
	}

	bool SetUpdateRate(float fUpdateRate) override {
		fprintf(stderr, "[ENGINE STUB] SetUpdateRate\n");
		return false;
	}

	void toggleGlobeview() override {
		fprintf(stderr, "[ENGINE STUB] toggleGlobeview\n");
	}

	bool isGlobeviewUp() override {
		fprintf(stderr, "[ENGINE STUB] isGlobeviewUp\n");
		return false;
	}

	void toggleResourceLayer() override {
		fprintf(stderr, "[ENGINE STUB] toggleResourceLayer\n");
	}

	void toggleUnitLayer() override {
		fprintf(stderr, "[ENGINE STUB] toggleUnitLayer\n");
	}

	void setResourceLayer(bool bOn) override {
		fprintf(stderr, "[ENGINE STUB] setResourceLayer\n");
	}

	// =========================================================================
	// Camera movement
	// =========================================================================

	void MoveBaseTurnRight(float increment) override {
		fprintf(stderr, "[ENGINE STUB] MoveBaseTurnRight\n");
	}

	void MoveBaseTurnLeft(float increment) override {
		fprintf(stderr, "[ENGINE STUB] MoveBaseTurnLeft\n");
	}

	void SetFlying(bool value) override {
		fprintf(stderr, "[ENGINE STUB] SetFlying\n");
	}

	void CycleFlyingMode(int displacement) override {
		fprintf(stderr, "[ENGINE STUB] CycleFlyingMode\n");
	}

	void SetMouseFlying(bool value) override {
		fprintf(stderr, "[ENGINE STUB] SetMouseFlying\n");
	}

	void SetSatelliteMode(bool value) override {
		fprintf(stderr, "[ENGINE STUB] SetSatelliteMode\n");
	}

	void SetOrthoCamera(bool value) override {
		fprintf(stderr, "[ENGINE STUB] SetOrthoCamera\n");
	}

	bool GetFlying() override {
		fprintf(stderr, "[ENGINE STUB] GetFlying\n");
		return false;
	}

	bool GetMouseFlying() override {
		fprintf(stderr, "[ENGINE STUB] GetMouseFlying\n");
		return false;
	}

	bool GetSatelliteMode() override {
		fprintf(stderr, "[ENGINE STUB] GetSatelliteMode\n");
		return false;
	}

	bool GetOrthoCamera() override {
		fprintf(stderr, "[ENGINE STUB] GetOrthoCamera\n");
		return false;
	}

	// =========================================================================
	// Landscape
	// =========================================================================

	int InitGraphics() override {
		fprintf(stderr, "[ENGINE STUB] InitGraphics\n");
		return 0;
	}

	void GetLandscapeDimensions(float& fWidth, float& fHeight) override {
		fprintf(stderr, "[ENGINE STUB] GetLandscapeDimensions\n");
		fWidth = 0.0f;
		fHeight = 0.0f;
	}

	void GetLandscapeGameDimensions(float& fWidth, float& fHeight) override {
		fprintf(stderr, "[ENGINE STUB] GetLandscapeGameDimensions\n");
		fWidth = 0.0f;
		fHeight = 0.0f;
	}

	uint GetGameCellSizeX() override {
		fprintf(stderr, "[ENGINE STUB] GetGameCellSizeX\n");
		return 0;
	}

	uint GetGameCellSizeY() override {
		fprintf(stderr, "[ENGINE STUB] GetGameCellSizeY\n");
		return 0;
	}

	float GetPointZSpacing() override {
		fprintf(stderr, "[ENGINE STUB] GetPointZSpacing\n");
		return 0.0f;
	}

	float GetPointXYSpacing() override {
		fprintf(stderr, "[ENGINE STUB] GetPointXYSpacing\n");
		return 0.0f;
	}

	float GetPointXSpacing() override {
		fprintf(stderr, "[ENGINE STUB] GetPointXSpacing\n");
		return 0.0f;
	}

	float GetPointYSpacing() override {
		fprintf(stderr, "[ENGINE STUB] GetPointYSpacing\n");
		return 0.0f;
	}

	float GetHeightmapZ(const NiPoint3& pt3, bool bClampAboveWater) override {
		fprintf(stderr, "[ENGINE STUB] GetHeightmapZ\n");
		return 0.0f;
	}

	void LightenVisibility(uint) override {
		fprintf(stderr, "[ENGINE STUB] LightenVisibility\n");
	}

	void DarkenVisibility(uint) override {
		fprintf(stderr, "[ENGINE STUB] DarkenVisibility\n");
	}

	void BlackenVisibility(uint) override {
		fprintf(stderr, "[ENGINE STUB] BlackenVisibility\n");
	}

	void RebuildAllPlots() override {
		fprintf(stderr, "[ENGINE STUB] RebuildAllPlots\n");
	}

	void RebuildPlot(int plotX, int plotY, bool bRebuildHeights, bool bRebuildTextures) override {
		fprintf(stderr, "[ENGINE STUB] RebuildPlot\n");
	}

	void RebuildRiverPlotTile(int plotX, int plotY, bool bRebuildHeights, bool bRebuildTextures) override {
		fprintf(stderr, "[ENGINE STUB] RebuildRiverPlotTile\n");
	}

	void RebuildTileArt(int plotX, int plotY) override {
		fprintf(stderr, "[ENGINE STUB] RebuildTileArt\n");
	}

	void ForceTreeOffsets(int plotX, int plotY) override {
		fprintf(stderr, "[ENGINE STUB] ForceTreeOffsets\n");
	}

	// =========================================================================
	// Grid
	// =========================================================================

	bool GetGridMode() override {
		fprintf(stderr, "[ENGINE STUB] GetGridMode\n");
		return false;
	}

	void SetGridMode(bool bVal) override {
		fprintf(stderr, "[ENGINE STUB] SetGridMode\n");
	}

	// =========================================================================
	// Plot coloring / borders
	// =========================================================================

	void addColoredPlot(int plotX, int plotY, const NiColorA& color, PlotStyles plotStyle, PlotLandscapeLayers layer) override {
		fprintf(stderr, "[ENGINE STUB] addColoredPlot\n");
	}

	void clearColoredPlots(PlotLandscapeLayers layer) override {
		fprintf(stderr, "[ENGINE STUB] clearColoredPlots\n");
	}

	void fillAreaBorderPlot(int plotX, int plotY, const NiColorA& color, AreaBorderLayers layer) override {
		fprintf(stderr, "[ENGINE STUB] fillAreaBorderPlot\n");
	}

	void clearAreaBorderPlots(AreaBorderLayers layer) override {
		fprintf(stderr, "[ENGINE STUB] clearAreaBorderPlots\n");
	}

	void updateFoundingBorder() override {
		fprintf(stderr, "[ENGINE STUB] updateFoundingBorder\n");
	}

	void addLandmark(CvPlot* plot, const wchar* caption) override {
		fprintf(stderr, "[ENGINE STUB] addLandmark\n");
	}

	// =========================================================================
	// Effects / Profile
	// =========================================================================

	void TriggerEffect(int iEffect, NiPoint3 pt3Point, float rotation) override {
		fprintf(stderr, "[ENGINE STUB] TriggerEffect\n");
	}

	void printProfileText() override {
		fprintf(stderr, "[ENGINE STUB] printProfileText\n");
	}

	// =========================================================================
	// Signs / Pick
	// =========================================================================

	void clearSigns() override {
		fprintf(stderr, "[ENGINE STUB] clearSigns\n");
	}

	CvPlot* pickPlot(int x, int y, NiPoint3& worldPoint) override {
		fprintf(stderr, "[ENGINE STUB] pickPlot\n");
		return nullptr;
	}

	// =========================================================================
	// Dirty bits / Fog of War
	// =========================================================================

	void SetDirty(EngineDirtyBits eBit, bool bNewValue) override {
		fprintf(stderr, "[ENGINE STUB] SetDirty\n");
	}

	bool IsDirty(EngineDirtyBits eBit) override {
		fprintf(stderr, "[ENGINE STUB] IsDirty\n");
		return false;
	}

	void PushFogOfWar(FogOfWarModeTypes eNewMode) override {
		fprintf(stderr, "[ENGINE STUB] PushFogOfWar\n");
	}

	FogOfWarModeTypes PopFogOfWar() override {
		fprintf(stderr, "[ENGINE STUB] PopFogOfWar\n");
		return FogOfWarModeTypes{};
	}

	void setFogOfWarFromStack() override {
		fprintf(stderr, "[ENGINE STUB] setFogOfWarFromStack\n");
	}

	void MarkBridgesDirty() override {
		fprintf(stderr, "[ENGINE STUB] MarkBridgesDirty\n");
	}

	void AddLaunch(PlayerTypes playerType) override {
		fprintf(stderr, "[ENGINE STUB] AddLaunch\n");
	}

	void AddGreatWall(CvCity* city) override {
		fprintf(stderr, "[ENGINE STUB] AddGreatWall\n");
	}

	void RemoveGreatWall(CvCity* city) override {
		fprintf(stderr, "[ENGINE STUB] RemoveGreatWall\n");
	}

	void MarkPlotTextureAsDirty(int plotX, int plotY) override {
		fprintf(stderr, "[ENGINE STUB] MarkPlotTextureAsDirty\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLEngineIFaceImpl g_engineIFaceInstance;
CvDLLEngineIFaceBase* g_pEngineIFace = &g_engineIFaceInstance;
