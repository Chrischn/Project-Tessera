// =============================================================================
// File:              relay_types.h
// Author(s):         Chrischn89
// Description:
//   C++03-compatible type definitions and interface base class declarations
//   for the TesseraRelay DLL. Vtable order MUST match the SDK exactly.
//   Verified against host/src/bridge/iface_*.h clean-room headers.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#ifndef RELAY_TYPES_H
#define RELAY_TYPES_H

#include <cstddef>
#include <string>
#include <vector>
#include <list>

// =============================================================================
// Platform types (C++03)
// =============================================================================
typedef char TCHAR;
typedef const char* LPCTSTR;
typedef const char* LPCSTR;
typedef char* LPTSTR;
typedef unsigned char byte;
typedef unsigned int uint;
typedef wchar_t wchar;

// =============================================================================
// String types (VS2003 native)
// =============================================================================
typedef std::string CvString;
typedef std::wstring CvWString;

struct CvWStringBuffer {
    std::wstring m_str;
};

// =============================================================================
// Forward-declared opaque types
// =============================================================================
struct CvEntity;
struct CvUnitEntity;
struct CvUnit;
struct CvCity;
struct CvPlot;
struct CvSelectionGroup;
struct CvRiver;
struct CvMissionDefinition;
struct CvDiploParameters;
struct CvSymbol;
struct CvRoute;
struct CvFeature;
struct CvFlagEntity;
struct CvPlotBuilder;
struct CvReplayInfo;
struct CvPopupInfo;
struct CvPopup;
struct CvMessageData;
struct CvCacheObject;
struct CvFont;
struct CvTalkingHeadMessage;
struct CvAudioGame;
struct ProfileSample;
struct TradeData;
struct PyObject;

// Engine opaque types
struct FXml;
struct FXmlSchemaCache;
struct FAStar;
struct FAStarNode;
struct FIniParser;

// Forward-declare base classes
class FDataStreamBase;

// =============================================================================
// Geometry types
// =============================================================================
struct NiPoint3 { float x, y, z; };
struct NiColorA { float r, g, b, a; };

// =============================================================================
// Linked list templates
// =============================================================================
template<typename T> struct CLLNode {
    T m_data;
    CLLNode<T>* m_pNext;
    CLLNode<T>* m_pPrev;
};

template<typename T> struct CLinkList {
    CLLNode<T>* m_pHead;
    CLLNode<T>* m_pTail;
    int m_iLength;
};

struct IDInfo {
    int eOwner;
    int iID;
};

// =============================================================================
// Enums (plain C-style — must match SDK ABI)
// =============================================================================
enum PlayerTypes       { NO_PLAYER = -1 };
enum TeamTypes         { NO_TEAM = -1 };
enum SaveGameTypes     { SAVEGAME_NORMAL = 0 };
enum LoadType          { LOADTYPE_NONE = 0 };
enum AnimationTypes    { ANIMATION_DEFAULT = 0 };
enum MissionTypes      { MISSION_NONE = 0 };
enum EraTypes          { NO_ERA = -1 };
enum UnitTypes         { NO_UNIT = -1 };
enum BuildingTypes     { NO_BUILDING = -1 };
enum TechTypes         { NO_TECH = -1 };
enum PromotionTypes    { NO_PROMOTION = -1 };
enum ReligionTypes     { NO_RELIGION = -1 };
enum CorporationTypes  { NO_CORPORATION = -1 };
enum CivicTypes        { NO_CIVIC = -1 };
enum CivilizationTypes { NO_CIVILIZATION = -1 };
enum LeaderHeadTypes   { NO_LEADER = -1 };
enum BuildTypes        { NO_BUILD = -1 };
enum BonusTypes        { NO_BONUS = -1 };
enum HurryTypes        { NO_HURRY = -1 };
enum VictoryTypes      { NO_VICTORY = -1 };
enum GoodyTypes        { GOODY_NONE = 0 };
enum FeatureTypes      { NO_FEATURE = -1 };
enum RouteTypes        { NO_ROUTE = -1 };
enum ImprovementTypes  { NO_IMPROVEMENT = -1 };
enum SymbolTypes       { SYMBOL_NONE = 0 };
enum TradeableItems    { TRADE_NONE = 0 };
enum DiploEventTypes   { DIPLO_NONE = 0 };
enum NetContactTypes   { NETCONTACT_NONE = 0 };
enum ChatTargetTypes   { NO_CHATTARGET = -1 };
enum ProjectTypes      { NO_PROJECT = -1 };
enum ProcessTypes      { NO_PROCESS = -1 };
enum InterfaceModeTypes    { INTERFACEMODE_NONE = 0 };
enum InterfaceMessageTypes { MESSAGE_TYPE_INFO = 0 };
enum InterfaceVisibility   { INTERFACE_SHOW = 0 };
enum MinimapModeTypes      { MINIMAP_DEFAULT = 0 };
enum CityTabTypes          { CITYTAB_NONE = 0 };
enum WidgetTypes           { WIDGET_GENERAL = 0 };
enum PopupStates           { POPUPSTATE_QUEUED = 0 };
enum PopupEventTypes       { POPUPEVENT_NONE = 0 };
enum PopupControlLayout    { POPUP_LAYOUT_CENTER = 0, POPUP_LAYOUT_LEFT, POPUP_LAYOUT_STRETCH };
enum InterfaceDirtyBits    { DIRTY_BIT_NONE = 0 };
enum CameraLookAtTypes     { CAMERALOOKAT_DEFAULT = 0 };
enum PlotStyles            { PLOTSTYLE_NONE = 0 };
enum PlotLandscapeLayers   { PLOTLAYER_NONE = 0 };
enum AreaBorderLayers      { AREABORDER_NONE = 0 };
enum EngineDirtyBits       { ENGINE_DIRTY_NONE = 0 };
enum FogOfWarModeTypes     { FOGOFWAR_DEFAULT = 0 };
enum ColorTypes            { NO_COLOR = -1 };
enum WorldSizeTypes        { WORLDSIZE_NONE = 0 };
enum PlayerOptionTypes     { PLAYEROPTION_NONE = 0 };
enum GraphicOptionTypes    { GRAPHICOPTION_NONE = 0 };
enum YieldTypes            { NO_YIELD = -1 };
enum CommerceTypes         { NO_COMMERCE = -1 };
enum RiverTypes            { RIVER_NONE = 0 };
enum CardinalDirectionTypes { CARDDIR_NONE = 0 };

// Function pointer types for pathfinding
typedef int(*FAPointFunc)(int, int, const void*, FAStar*);
typedef int(*FAHeuristic)(int, int, int, int);
typedef int(*FAStarFunc)(FAStarNode*, FAStarNode*, int, const void*, FAStar*);

// Constants
#ifndef MAX_INT
#define MAX_INT 0x7FFFFFFF
#endif

enum {
    DLL_FONT_LEFT_JUSTIFY   = 1 << 0,
    DLL_FONT_RIGHT_JUSTIFY  = 1 << 1,
    DLL_FONT_CENTER_JUSTIFY = 1 << 2
};

typedef std::list<CvPopupInfo*> CvPopupQueue;

// Long declaration block intentionally omitted for POINT struct.
// The relay never uses POINT directly.

// =============================================================================
// Interface Base Class Declarations
// =============================================================================
// Each class below declares ONLY the pure virtual methods, in the EXACT order
// they appear in the vtable. The relay provides concrete implementations.
//
// IMPORTANT: Copy method signatures from the corresponding host header at
//   host/src/bridge/iface_*.h — these are the authoritative clean-room
//   definitions verified against the SDK.
//
// The full declarations are too large to inline here (~800 methods across all
// interfaces). See each relay_*.cpp file for the impl class that inherits
// from these bases and provides the concrete method bodies.
// =============================================================================

// ---------------------------------------------------------------------------
// CvDLLXmlIFaceBase — copy method signatures from host/src/bridge/iface_xml.h
// (42 virtual methods)
// ---------------------------------------------------------------------------
class CvDLLXmlIFaceBase
{
public:
    virtual FXml* CreateFXml(FXmlSchemaCache* pSchemaCache = 0) = 0;
    virtual void DestroyFXml(FXml*& xml) = 0;
    virtual void DestroyFXmlSchemaCache(FXmlSchemaCache*&) = 0;
    virtual FXmlSchemaCache* CreateFXmlSchemaCache() = 0;
    virtual bool LoadXml(FXml* xml, const TCHAR* pszXmlFile) = 0;
    virtual bool Validate(FXml* xml, TCHAR* pszError = NULL) = 0;
    virtual bool LocateNode(FXml* xml, const TCHAR* pszXmlNode) = 0;
    virtual bool LocateFirstSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) = 0;
    virtual bool LocateNextSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) = 0;
    virtual bool NextSibling(FXml* xml) = 0;
    virtual bool PrevSibling(FXml* xml) = 0;
    virtual bool SetToChild(FXml* xml) = 0;
    virtual bool SetToChildByTagName(FXml* xml, const TCHAR* szTagName) = 0;
    virtual bool SetToParent(FXml* xml) = 0;
    virtual bool AddChildNode(FXml* xml, TCHAR* pszNewNode) = 0;
    virtual bool AddSiblingNodeBefore(FXml* xml, TCHAR* pszNewNode) = 0;
    virtual bool AddSiblingNodeAfter(FXml* xml, TCHAR* pszNewNode) = 0;
    virtual bool WriteXml(FXml* xml, TCHAR* pszXmlFile) = 0;
    virtual bool SetInsertedNodeAttribute(FXml* xml, TCHAR* pszAttrName, TCHAR* pszAttrVal) = 0;
    virtual int GetLastNodeTextSize(FXml* xml) = 0;
    virtual bool GetLastNodeText(FXml* xml, TCHAR* pszText) = 0;
    virtual bool GetLastNodeValue(FXml* xml, std::string& pszText) = 0;
    virtual bool GetLastNodeValue(FXml* xml, std::wstring& pszText) = 0;
    virtual bool GetLastNodeValue(FXml* xml, char* pszText) = 0;
    virtual bool GetLastNodeValue(FXml* xml, wchar* pszText) = 0;
    virtual bool GetLastNodeValue(FXml* xml, bool* pbVal) = 0;
    virtual bool GetLastNodeValue(FXml* xml, int* piVal) = 0;
    virtual bool GetLastNodeValue(FXml* xml, float* pfVal) = 0;
    virtual bool GetLastNodeValue(FXml* xml, unsigned int* puiVal) = 0;
    virtual int GetInsertedNodeTextSize(FXml* xml) = 0;
    virtual bool GetInsertedNodeText(FXml* xml, TCHAR* pszText) = 0;
    virtual bool SetInsertedNodeText(FXml* xml, TCHAR* pszText) = 0;
    virtual bool GetLastLocatedNodeType(FXml* xml, TCHAR* pszType) = 0;
    virtual bool GetLastInsertedNodeType(FXml* xml, TCHAR* pszType) = 0;
    virtual bool IsLastLocatedNodeCommentNode(FXml* xml) = 0;
    virtual int NumOfElementsByTagName(FXml* xml, TCHAR* pszTagName) = 0;
    virtual int NumOfChildrenByTagName(FXml* xml, const TCHAR* pszTagName) = 0;
    virtual int GetNumSiblings(FXml* xml) = 0;
    virtual int GetNumChildren(FXml* xml) = 0;
    virtual bool GetLastLocatedNodeTagName(FXml* xml, TCHAR* pszTagName) = 0;
    virtual bool IsAllowXMLCaching() = 0;
    virtual void MapChildren(FXml*) = 0;
};

// ---------------------------------------------------------------------------
// Forward-declare all sub-interface base classes.
// Full declarations are in relay_stubs.cpp (for stub interfaces) and
// relay_utility.cpp (for utility). The relay impl classes inherit from these.
//
// For each interface, the COMPLETE virtual method list is defined in its
// corresponding relay_*.cpp file, matching the host's iface_*.h header.
// ---------------------------------------------------------------------------

// CvDLLEntityIFaceBase — see host/src/bridge/iface_entity.h (25 methods)
class CvDLLEntityIFaceBase
{
public:
    virtual void removeEntity(CvEntity*) = 0;
    virtual void addEntity(CvEntity*, uint uiEntAddFlags) = 0;
    virtual void setup(CvEntity*) = 0;
    virtual void setVisible(CvEntity*, bool) = 0;
    virtual void createCityEntity(CvCity*) = 0;
    virtual void createUnitEntity(CvUnit*) = 0;
    virtual void destroyEntity(CvEntity*&, bool bSafeDelete = true) = 0;
    virtual void updatePosition(CvEntity* gameEntity) = 0;
    virtual void setupFloodPlains(CvRiver* river) = 0;
    virtual bool IsSelected(const CvEntity*) const = 0;
    virtual void PlayAnimation(CvEntity*, AnimationTypes eAnim, float fSpeed = 1.0f, bool bQueue = false, int iLayer = 0, float fStartPct = 0.0f, float fEndPct = 1.0f) = 0;
    virtual void StopAnimation(CvEntity*, AnimationTypes eAnim) = 0;
    virtual void StopAnimation(CvEntity*) = 0;
    virtual void NotifyEntity(CvUnitEntity*, MissionTypes eMission) = 0;
    virtual void MoveTo(CvUnitEntity*, const CvPlot* pkPlot) = 0;
    virtual void QueueMove(CvUnitEntity*, const CvPlot* pkPlot) = 0;
    virtual void ExecuteMove(CvUnitEntity*, float fTimeToExecute, bool bCombat) = 0;
    virtual void SetPosition(CvUnitEntity* pEntity, const CvPlot* pkPlot) = 0;
    virtual void AddMission(const CvMissionDefinition* pDefinition) = 0;
    virtual void RemoveUnitFromBattle(CvUnit* pUnit) = 0;
    virtual void showPromotionGlow(CvUnitEntity* pEntity, bool show) = 0;
    virtual void updateEnemyGlow(CvUnitEntity* pEntity) = 0;
    virtual void updatePromotionLayers(CvUnitEntity* pEntity) = 0;
    virtual void updateGraphicEra(CvUnitEntity* pEntity, EraTypes eOldEra = NO_ERA) = 0;
    virtual void SetSiegeTower(CvUnitEntity* pEntity, bool show) = 0;
    virtual bool GetSiegeTower(CvUnitEntity* pEntity) = 0;
};

// CvDLLEngineIFaceBase — see host/src/bridge/iface_engine.h (82 methods)
// CvDLLInterfaceIFaceBase — see host/src/bridge/iface_interface.h (130+ methods)
// CvDLLPythonIFaceBase — see host/src/bridge/iface_python.h (12 methods)
// CvDLLFAStarIFaceBase — see host/src/bridge/iface_fastar.h (14 methods)
// CvDLLIniParserIFaceBase — see host/src/bridge/iface_ini_parser.h (8 methods)
// CvDLLSymbolIFaceBase — see host/src/bridge/iface_symbol.h (11 methods)
// CvDLLFeatureIFaceBase — see host/src/bridge/iface_feature.h (12 methods)
// CvDLLRouteIFaceBase — see host/src/bridge/iface_route.h (9 methods)
// CvDLLRiverIFaceBase — see host/src/bridge/iface_river.h (6 methods)
// CvDLLFlagEntityIFaceBase : CvDLLEntityIFaceBase — see host/src/bridge/iface_flag_entity.h (8 own + 26 inherited)
// CvDLLPlotBuilderIFaceBase : CvDLLEntityIFaceBase — see host/src/bridge/iface_plot_builder.h (3 own + 26 inherited)
// CvDLLEventReporterIFaceBase — see host/src/bridge/iface_event_reporter.h (65+ methods)
// FDataStreamBase — see host/src/bridge/iface_data_stream.h (80+ methods)
//
// These base class declarations are defined directly in relay_stubs.cpp
// to keep this header focused. Each impl class in relay_stubs.cpp copies
// the EXACT virtual method list from the host header, adapted to C++03.

// ---------------------------------------------------------------------------
// CvDLLUtilityIFaceBase — see host/src/bridge/iface_utility.h (~95 methods)
// Defined directly in relay_utility.cpp with all methods.
// ---------------------------------------------------------------------------

#endif /* RELAY_TYPES_H */
