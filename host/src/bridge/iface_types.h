// =============================================================================
// File:              iface_types.h
// Author(s):         Chrischn89
// Description:
//   Forward declarations and type aliases for clean-room CvDLL*IFaceBase
//   interface definitions. Types crossing the DLL boundary are defined here.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <list>

// =============================================================================
// 1. Windows/Platform Types
// =============================================================================
#ifndef TCHAR
typedef char TCHAR;
#endif
#ifndef LPCTSTR
typedef const char* LPCTSTR;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef LPTSTR
typedef char* LPTSTR;
#endif
typedef unsigned char byte;
typedef unsigned int uint;
typedef wchar_t wchar;

#ifndef _WINDEF_
struct POINT { long x; long y; };
#endif

// =============================================================================
// 2. Forward-declared opaque types (game entities)
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

// =============================================================================
// 3. Engine/library opaque types
// =============================================================================
struct FXml;            // pugixml wrapper (defined in iface_xml.h)
struct FXmlSchemaCache; // schema cache placeholder
struct FAStar;          // pathfinder handle
struct FAStarNode;      // path node
struct FIniParser;      // INI parser handle

// =============================================================================
// 4. FDataStreamBase (abstract binary stream — full def in iface_data_stream.h)
// =============================================================================
class FDataStreamBase;

// =============================================================================
// 5. Geometry types (Gamebryo value types — must be fully defined)
// =============================================================================
struct NiPoint3 { float x, y, z; };
struct NiColorA { float r, g, b, a; };

// =============================================================================
// 6. Python type (forward declaration only — Python not in MVP)
// =============================================================================
struct PyObject;

// =============================================================================
// 7. String types
// =============================================================================
// CvString/CvWString — for MVP, alias to std::string/std::wstring.
// If ABI incompatibility with VS2003-compiled DLLs is detected (Task 9),
// swap to VS2003-layout-compatible structs.
using CvString = std::string;
using CvWString = std::wstring;

// CvWStringBuffer — simplified placeholder
struct CvWStringBuffer {
    std::wstring m_str;
};

// =============================================================================
// 8. Linked list templates
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

// =============================================================================
// 9. IDInfo struct
// =============================================================================
struct IDInfo {
    int eOwner;
    int iID;
};

// =============================================================================
// 10. Enum types used in interface method signatures
//     Plain enums (not enum class) to match C-style ABI of the original SDK.
// =============================================================================

// Player/Team
enum PlayerTypes       { NO_PLAYER = -1 };
enum TeamTypes         { NO_TEAM = -1 };

// Save/Load
enum SaveGameTypes     { SAVEGAME_NORMAL = 0 };
enum LoadType          {};

// Animation/Mission
enum AnimationTypes    {};
enum MissionTypes      {};
enum EraTypes          { NO_ERA = -1 };

// Units/Buildings/Techs (game object types)
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
enum GoodyTypes        {};

// Terrain
enum FeatureTypes      { NO_FEATURE = -1 };
enum RouteTypes        { NO_ROUTE = -1 };
enum ImprovementTypes  { NO_IMPROVEMENT = -1 };
enum SymbolTypes       {};

// Trade/Diplomacy
enum TradeableItems    {};
enum DiploEventTypes   {};
enum NetContactTypes   {};
enum ChatTargetTypes   { NO_CHATTARGET = -1 };

// UI/Interface
enum InterfaceModeTypes    {};
enum InterfaceMessageTypes {};
enum InterfaceVisibility   {};
enum MinimapModeTypes      {};
enum CityTabTypes          {};
enum WidgetTypes           {};
enum PopupStates           { POPUPSTATE_QUEUED = 0 };
enum PopupEventTypes       {};
enum PopupControlLayout    {};
enum InterfaceDirtyBits    {};

// Camera
enum CameraLookAtTypes       {};
enum CameraMovementSpeeds    {};
enum CameraAnimationTypes    {};

// Graphics/Visual
enum PlotStyles              {};
enum PlotLandscapeLayers     {};
enum AreaBorderLayers        {};
enum EngineDirtyBits         {};
enum FogOfWarModeTypes       {};
enum ColorTypes              { NO_COLOR = -1 };
enum WorldSizeTypes          {};

// Options
enum PlayerOptionTypes       {};
enum GraphicOptionTypes      {};

// Yield/Commerce (referenced in event reporter)
enum YieldTypes    { NO_YIELD = -1 };
enum CommerceTypes { NO_COMMERCE = -1 };

// River
enum RiverTypes              {};
enum CardinalDirectionTypes  {};

// =============================================================================
// 11. Function pointer typedefs for pathfinding
// =============================================================================
typedef int(*FAPointFunc)(int, int, const void*, FAStar*);
typedef int(*FAHeuristic)(int, int, int, int);
typedef int(*FAStarFunc)(FAStarNode*, FAStarNode*, int, const void*, FAStar*);

// =============================================================================
// 12. Constants used as default parameter values
// =============================================================================
#ifndef MAX_INT
#define MAX_INT 0x7FFFFFFF
#endif
