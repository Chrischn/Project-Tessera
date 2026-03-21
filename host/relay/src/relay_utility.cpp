// =============================================================================
// File:              relay_utility.cpp
// Author(s):         Chrischn89
// Description:
//   VS2003-compiled CvDLLUtilityIFaceBase implementation. The root interface
//   wired into gDLL. Memory ops use native VS2003 malloc/free. File enumeration
//   and logging forward to host via C callbacks. All other methods are stubs.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "relay_types.h"
#include "relay_api.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <malloc.h>

// Access to global callbacks (defined in relay_main.cpp)
extern HostCallbacks* relay_get_callbacks();

// =============================================================================
// Forward-declare sub-interface classes that are not yet fully declared
// in relay_types.h (they are defined in relay_stubs.cpp)
// =============================================================================
// Already declared in relay_types.h:
//   CvDLLEntityIFaceBase, CvDLLXmlIFaceBase
// Need forward declarations for the rest:
class CvDLLEngineIFaceBase;
class CvDLLInterfaceIFaceBase;
class CvDLLIniParserIFaceBase;
class CvDLLSymbolIFaceBase;
class CvDLLFeatureIFaceBase;
class CvDLLRouteIFaceBase;
class CvDLLPlotBuilderIFaceBase;
class CvDLLRiverIFaceBase;
class CvDLLFAStarIFaceBase;
class CvDLLFlagEntityIFaceBase;
class CvDLLPythonIFaceBase;

// =============================================================================
// Extern pointers to all sub-interface singletons
// (defined in relay_stubs.cpp and relay_xml.cpp)
// =============================================================================
extern CvDLLEngineIFaceBase*     g_pRelayEngineIFace;       // relay_stubs.cpp
extern CvDLLEntityIFaceBase*     g_pRelayEntityIFace;       // relay_stubs.cpp
extern CvDLLInterfaceIFaceBase*  g_pRelayInterfaceIFace;    // relay_stubs.cpp
extern CvDLLIniParserIFaceBase*  g_pRelayIniParserIFace;    // relay_stubs.cpp
extern CvDLLSymbolIFaceBase*     g_pRelaySymbolIFace;       // relay_stubs.cpp
extern CvDLLFeatureIFaceBase*    g_pRelayFeatureIFace;      // relay_stubs.cpp
extern CvDLLRouteIFaceBase*      g_pRelayRouteIFace;        // relay_stubs.cpp
extern CvDLLPlotBuilderIFaceBase* g_pRelayPlotBuilderIFace; // relay_stubs.cpp
extern CvDLLRiverIFaceBase*      g_pRelayRiverIFace;       // relay_stubs.cpp
extern CvDLLFAStarIFaceBase*     g_pRelayFAStarIFace;      // relay_stubs.cpp
extern CvDLLXmlIFaceBase*        g_pRelayXmlIFace;         // relay_xml.cpp
extern CvDLLFlagEntityIFaceBase* g_pRelayFlagEntityIFace;  // relay_stubs.cpp
extern CvDLLPythonIFaceBase*     g_pRelayPythonIFace;      // relay_stubs.cpp

// =============================================================================
// CvDLLUtilityIFaceBase — pure virtual base class
// Vtable order MUST match host/src/bridge/iface_utility.h EXACTLY.
// =============================================================================

class CvDLLUtilityIFaceBase
{
public:
    // --- Sub-interface accessors (vtable slots 0-12) ---
    virtual CvDLLEntityIFaceBase* getEntityIFace() = 0;
    virtual CvDLLInterfaceIFaceBase* getInterfaceIFace() = 0;
    virtual CvDLLEngineIFaceBase* getEngineIFace() = 0;
    virtual CvDLLIniParserIFaceBase* getIniParserIFace() = 0;
    virtual CvDLLSymbolIFaceBase* getSymbolIFace() = 0;
    virtual CvDLLFeatureIFaceBase* getFeatureIFace() = 0;
    virtual CvDLLRouteIFaceBase* getRouteIFace() = 0;
    virtual CvDLLPlotBuilderIFaceBase* getPlotBuilderIFace() = 0;
    virtual CvDLLRiverIFaceBase* getRiverIFace() = 0;
    virtual CvDLLFAStarIFaceBase* getFAStarIFace() = 0;
    virtual CvDLLXmlIFaceBase* getXMLIFace() = 0;
    virtual CvDLLFlagEntityIFaceBase* getFlagEntityIFace() = 0;
    virtual CvDLLPythonIFaceBase* getPythonIFace() = 0;

    // --- Memory management (vtable slots 13-20) ---
    virtual void delMem(void* p) = 0;
    virtual void* newMem(size_t size) = 0;
    virtual void delMem(void* p, const char* pcFile, int iLine) = 0;
    virtual void* newMem(size_t size, const char* pcFile, int iLine) = 0;
    virtual void delMemArray(void* p, const char* pcFile, int iLine) = 0;
    virtual void* newMemArray(size_t size, const char* pcFile, int iLine) = 0;
    virtual void* reallocMem(void* a, unsigned int uiBytes, const char* pcFile, int iLine) = 0;
    virtual unsigned int memSize(void* a) = 0;

    // --- Vector clearing (vtable slots 21-23) ---
    virtual void clearVector(std::vector<int>& vec) = 0;
    virtual void clearVector(std::vector<byte>& vec) = 0;
    virtual void clearVector(std::vector<float>& vec) = 0;

    // --- Network (vtable slots 24-30) ---
    virtual int getAssignedNetworkID(int iPlayerID) = 0;
    virtual bool isConnected(int iNetID) = 0;
    virtual bool isGameActive() = 0;
    virtual int GetLocalNetworkID() = 0;
    virtual int GetSyncOOS(int iNetID) = 0;
    virtual int GetOptionsOOS(int iNetID) = 0;
    virtual int GetLastPing(int iNetID) = 0;

    // --- Modem (vtable slots 31-32) ---
    virtual bool IsModem() = 0;
    virtual void SetModem(bool bModem) = 0;

    // --- Buddy (vtable slots 33-34) ---
    virtual void AcceptBuddy(const char* szName, int iRequestID) = 0;
    virtual void RejectBuddy(const char* szName, int iRequestID) = 0;

    // --- Misc (vtable slots 35-40) ---
    virtual void messageControlLog(char* s) = 0;
    virtual int getChtLvl() = 0;
    virtual void setChtLvl(int iLevel) = 0;
    virtual bool GetWorldBuilderMode() = 0;
    virtual int getCurrentLanguage() const = 0;
    virtual void setCurrentLanguage(int iNewLanguage) = 0;
    virtual bool isModularXMLLoading() const = 0;

    // --- Pitboss (vtable slots 41-45) ---
    virtual bool IsPitbossHost() const = 0;
    virtual CvString GetPitbossSmtpHost() const = 0;
    virtual CvWString GetPitbossSmtpLogin() const = 0;
    virtual CvWString GetPitbossSmtpPassword() const = 0;
    virtual CvString GetPitbossEmail() const = 0;

    // --- Send messages (vtable slots 46-63) ---
    virtual void sendMessageData(CvMessageData* pData) = 0;
    virtual void sendPlayerInfo(PlayerTypes eActivePlayer) = 0;
    virtual void sendGameInfo(const CvWString& szGameName, const CvWString& szAdminPassword) = 0;
    virtual void sendPlayerOption(PlayerOptionTypes eOption, bool bValue) = 0;
    virtual void sendChat(const CvWString& szChatString, ChatTargetTypes eTarget) = 0;
    virtual void sendPause(int iPauseID = -1) = 0;
    virtual void sendMPRetire() = 0;
    virtual void sendToggleTradeMessage(PlayerTypes eWho, TradeableItems eItemType, int iData, int iOtherWho, bool bAIOffer, bool bSendToAll = false) = 0;
    virtual void sendClearTableMessage(PlayerTypes eWhoTradingWith) = 0;
    virtual void sendImplementDealMessage(PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList) = 0;
    virtual void sendContactCiv(NetContactTypes eContactType, PlayerTypes eWho) = 0;
    virtual void sendOffer() = 0;
    virtual void sendDiploEvent(PlayerTypes eWhoTradingWith, DiploEventTypes eDiploEvent, int iData1, int iData2) = 0;
    virtual void sendRenegotiate(PlayerTypes eWhoTradingWith) = 0;
    virtual void sendRenegotiateThisItem(PlayerTypes ePlayer2, TradeableItems eItemType, int iData) = 0;
    virtual void sendExitTrade() = 0;
    virtual void sendKillDeal(int iDealID, bool bFromDiplomacy) = 0;
    virtual void sendDiplomacy(PlayerTypes ePlayer, CvDiploParameters* pParams) = 0;
    virtual void sendPopup(PlayerTypes ePlayer, CvPopupInfo* pInfo) = 0;

    // --- Timing (vtable slots 64-67) ---
    virtual int getMillisecsPerTurn() = 0;
    virtual float getSecsPerTurn() = 0;
    virtual int getTurnsPerSecond() = 0;
    virtual int getTurnsPerMinute() = 0;

    // --- Slots (vtable slots 68-69) ---
    virtual void openSlot(PlayerTypes eID) = 0;
    virtual void closeSlot(PlayerTypes eID) = 0;

    // --- Map (vtable slots 70-75) ---
    virtual CvWString getMapScriptName() = 0;
    virtual bool getTransferredMap() = 0;
    virtual bool isDescFileName(const char* szFileName) = 0;
    virtual bool isWBMapScript() = 0;
    virtual bool isWBMapNoPlayers() = 0;
    virtual bool pythonMapExists(const char* szMapName) = 0;

    // --- String utilities (vtable slot 76) ---
    virtual void stripSpecialCharacters(CvWString& szName) = 0;

    // --- Globals (vtable slots 77-78) ---
    virtual void initGlobals() = 0;
    virtual void uninitGlobals() = 0;

    // --- Updater (vtable slot 79) ---
    virtual void callUpdater() = 0;

    // --- Compression (vtable slots 80-81) ---
    virtual bool Uncompress(byte** bufIn, unsigned long* bufLenIn, unsigned long maxBufLenOut, int offset = 0) = 0;
    virtual bool Compress(byte** bufIn, unsigned long* bufLenIn, int offset = 0) = 0;

    // --- UI (vtable slots 82-86) ---
    virtual void NiTextOut(const TCHAR* szText) = 0;
    virtual void MessageBox(const TCHAR* szText, const TCHAR* szCaption) = 0;
    virtual void SetDone(bool bDone) = 0;
    virtual bool GetDone() = 0;
    virtual bool GetAutorun() = 0;

    // --- Diplomacy (vtable slots 87-96) ---
    virtual void beginDiplomacy(CvDiploParameters* pDiploParams, PlayerTypes ePlayer) = 0;
    virtual void endDiplomacy() = 0;
    virtual bool isDiplomacy() = 0;
    virtual int getDiplomacyPlayer() = 0;
    virtual void updateDiplomacyAttitude(bool bForce = false) = 0;
    virtual bool isMPDiplomacy() = 0;
    virtual bool isMPDiplomacyScreenUp() = 0;
    virtual int getMPDiplomacyPlayer() = 0;
    virtual void beginMPDiplomacy(PlayerTypes eWhoTalkingTo, bool bRenegotiate = false, bool bSimultaneous = true) = 0;
    virtual void endMPDiplomacy() = 0;

    // --- Audio (vtable slots 97-100) ---
    virtual bool getAudioDisabled() = 0;
    virtual int getAudioTagIndex(const TCHAR* szTag, int iScriptType = -1) = 0;
    virtual void DoSound(int iScriptId) = 0;
    virtual void Do3DSound(int iScriptId, NiPoint3 vPosition) = 0;

    // --- Data streams (vtable slots 101-102) ---
    virtual FDataStreamBase* createFileStream() = 0;
    virtual void destroyDataStream(FDataStreamBase*& stream) = 0;

    // --- Cache objects (vtable slots 103-117) ---
    virtual CvCacheObject* createGlobalTextCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createGlobalDefinesCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createTechInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createBuildingInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createUnitInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createLeaderHeadInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createCivilizationInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createPromotionInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createDiplomacyInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createEventInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createEventTriggerInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createCivicInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createHandicapInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createBonusInfoCacheObject(const TCHAR* szCacheFileName) = 0;
    virtual CvCacheObject* createImprovementInfoCacheObject(const TCHAR* szCacheFileName) = 0;

    // --- Cache ops (vtable slots 118-120) ---
    virtual bool cacheRead(CvCacheObject* pCache, const TCHAR* szSourceFileName = NULL) = 0;
    virtual bool cacheWrite(CvCacheObject* pCache) = 0;
    virtual void destroyCache(CvCacheObject*& pCache) = 0;

    // --- File manager (vtable slot 121) ---
    virtual bool fileManagerEnabled() = 0;

    // --- Logging (vtable slots 122-123) ---
    virtual void logMsg(const TCHAR* pLogFileName, const TCHAR* pBuf, bool bWriteToConsole = false, bool bTimeStamp = true) = 0;
    virtual void logMemState(const char* msg) = 0;

    // --- Symbol ID (vtable slots 124-125) ---
    virtual int getSymbolID(int iID) = 0;
    virtual void setSymbolID(int iID, int iValue) = 0;

    // --- Text (vtable slots 126-129) ---
    virtual CvWString getText(CvWString szIDTag, ...) = 0;
    virtual CvWString getObjectText(CvWString szIDTag, uint uiForm, bool bNoSubs = false) = 0;
    virtual void addText(const TCHAR* szIDTag, const wchar* szString, const wchar* szGender = L"N", const wchar* szPlural = L"false") = 0;
    virtual uint getNumForms(CvWString szIDTag) = 0;

    // --- World / frame (vtable slots 130-131) ---
    virtual WorldSizeTypes getWorldSize() = 0;
    virtual uint getFrameCounter() const = 0;

    // --- Key state (vtable slots 132-137) ---
    virtual bool altKey() = 0;
    virtual bool shiftKey() = 0;
    virtual bool ctrlKey() = 0;
    virtual bool scrollLock() = 0;
    virtual bool capsLock() = 0;
    virtual bool numLock() = 0;

    // --- Profiler (vtable slots 138-141) ---
    virtual void ProfilerBegin() = 0;
    virtual void ProfilerEnd() = 0;
    virtual void BeginSample(ProfileSample* pSample) = 0;
    virtual void EndSample(ProfileSample* pSample) = 0;

    // --- Game init (vtable slot 142) ---
    virtual bool isGameInitializing() = 0;

    // --- File enumeration (vtable slots 143-144) ---
    virtual void enumerateFiles(std::vector<CvString>& files, const char* szPattern) = 0;
    virtual void enumerateModuleFiles(std::vector<CvString>& aszFiles, const CvString& refcstrRootDirectory, const CvString& refcstrModularDirectory, const CvString& refcstrExtension, bool bSearchSubdirectories) = 0;

    // --- Save/Load (vtable slots 145-151) ---
    virtual void SaveGame(SaveGameTypes eSaveGame) = 0;
    virtual void LoadGame() = 0;
    virtual int loadReplays(std::vector<CvReplayInfo*>& listReplays) = 0;
    virtual void QuickSave() = 0;
    virtual void QuickLoad() = 0;
    virtual void sendPbemTurn(PlayerTypes ePlayer) = 0;
    virtual void getPassword(PlayerTypes ePlayer) = 0;

    // --- Options (vtable slots 152-154) ---
    virtual bool getGraphicOption(GraphicOptionTypes eGraphicOption) = 0;
    virtual bool getPlayerOption(PlayerOptionTypes ePlayerOption) = 0;
    virtual int getMainMenu() = 0;

    // --- FMP (vtable slots 155-159) ---
    virtual bool isFMPMgrHost() = 0;
    virtual bool isFMPMgrPublic() = 0;
    virtual void handleRetirement(PlayerTypes ePlayer) = 0;
    virtual PlayerTypes getFirstBadConnection() = 0;
    virtual int getConnState(PlayerTypes ePlayer) = 0;

    // --- INI (vtable slot 160) ---
    virtual bool ChangeINIKeyValue(const char* szGroupKey, const char* szKeyValue, const char* szOut) = 0;

    // --- MD5 (vtable slot 161) ---
    virtual char* md5String(char* szString) = 0;

    // --- Mod info (vtable slots 162-163) ---
    virtual const char* getModName(bool bFullPath = true) const = 0;
    virtual bool hasSkippedSaveChecksum() const = 0;

    // --- Report (vtable slot 164) ---
    virtual void reportStatistics() = 0;
};

// =============================================================================
// Implementation class
// =============================================================================

class CvDLLUtilityIFaceImpl : public CvDLLUtilityIFaceBase
{
public:
    // =========================================================================
    // Sub-interface accessors — return singleton pointers from relay_stubs.cpp
    // =========================================================================

    CvDLLEntityIFaceBase* getEntityIFace() {
        return g_pRelayEntityIFace;
    }

    CvDLLInterfaceIFaceBase* getInterfaceIFace() {
        return g_pRelayInterfaceIFace;
    }

    CvDLLEngineIFaceBase* getEngineIFace() {
        return g_pRelayEngineIFace;
    }

    CvDLLIniParserIFaceBase* getIniParserIFace() {
        return g_pRelayIniParserIFace;
    }

    CvDLLSymbolIFaceBase* getSymbolIFace() {
        return g_pRelaySymbolIFace;
    }

    CvDLLFeatureIFaceBase* getFeatureIFace() {
        return g_pRelayFeatureIFace;
    }

    CvDLLRouteIFaceBase* getRouteIFace() {
        return g_pRelayRouteIFace;
    }

    CvDLLPlotBuilderIFaceBase* getPlotBuilderIFace() {
        return g_pRelayPlotBuilderIFace;
    }

    CvDLLRiverIFaceBase* getRiverIFace() {
        return g_pRelayRiverIFace;
    }

    CvDLLFAStarIFaceBase* getFAStarIFace() {
        return g_pRelayFAStarIFace;
    }

    CvDLLXmlIFaceBase* getXMLIFace() {
        return g_pRelayXmlIFace;
    }

    CvDLLFlagEntityIFaceBase* getFlagEntityIFace() {
        return g_pRelayFlagEntityIFace;
    }

    CvDLLPythonIFaceBase* getPythonIFace() {
        return g_pRelayPythonIFace;
    }

    // =========================================================================
    // Memory management — VS2003 native malloc/free (Category B)
    // =========================================================================

    void delMem(void* p) {
        free(p);
    }

    void* newMem(size_t size) {
        return malloc(size);
    }

    void delMem(void* p, const char* pcFile, int iLine) {
        (void)pcFile; (void)iLine;
        free(p);
    }

    void* newMem(size_t size, const char* pcFile, int iLine) {
        (void)pcFile; (void)iLine;
        return malloc(size);
    }

    void delMemArray(void* p, const char* pcFile, int iLine) {
        (void)pcFile; (void)iLine;
        free(p);
    }

    void* newMemArray(size_t size, const char* pcFile, int iLine) {
        (void)pcFile; (void)iLine;
        return malloc(size);
    }

    void* reallocMem(void* a, unsigned int uiBytes, const char* pcFile, int iLine) {
        (void)pcFile; (void)iLine;
        return realloc(a, uiBytes);
    }

    unsigned int memSize(void* a) {
        (void)a;
        // _msize is MSVC-specific; safe fallback
#ifdef _MSC_VER
        if (a) return (unsigned int)_msize(a);
#endif
        return 0;
    }

    // =========================================================================
    // Vector clearing — native (Category B)
    // =========================================================================

    void clearVector(std::vector<int>& vec) {
        vec.clear();
    }

    void clearVector(std::vector<byte>& vec) {
        vec.clear();
    }

    void clearVector(std::vector<float>& vec) {
        vec.clear();
    }

    // =========================================================================
    // Network stubs (Category C)
    // =========================================================================

    int getAssignedNetworkID(int iPlayerID) {
        (void)iPlayerID;
        return 0;
    }

    bool isConnected(int iNetID) {
        (void)iNetID;
        return false;
    }

    bool isGameActive() {
        return false;
    }

    int GetLocalNetworkID() {
        return 0;
    }

    int GetSyncOOS(int iNetID) {
        (void)iNetID;
        return 0;
    }

    int GetOptionsOOS(int iNetID) {
        (void)iNetID;
        return 0;
    }

    int GetLastPing(int iNetID) {
        (void)iNetID;
        return 0;
    }

    // =========================================================================
    // Modem stubs (Category C)
    // =========================================================================

    bool IsModem() {
        return false;
    }

    void SetModem(bool bModem) {
        (void)bModem;
    }

    // =========================================================================
    // Buddy stubs (Category C)
    // =========================================================================

    void AcceptBuddy(const char* szName, int iRequestID) {
        (void)szName; (void)iRequestID;
    }

    void RejectBuddy(const char* szName, int iRequestID) {
        (void)szName; (void)iRequestID;
    }

    // =========================================================================
    // Misc — callbacks where available, stubs otherwise (Category B/C)
    // =========================================================================

    void messageControlLog(char* s) {
        (void)s;
    }

    int getChtLvl() {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->get_cheat_level) return cb->get_cheat_level();
        return 0;
    }

    void setChtLvl(int iLevel) {
        (void)iLevel;
    }

    bool GetWorldBuilderMode() {
        return false;
    }

    int getCurrentLanguage() const {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->get_current_language) return cb->get_current_language();
        return 0;
    }

    void setCurrentLanguage(int iNewLanguage) {
        (void)iNewLanguage;
    }

    bool isModularXMLLoading() const {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->is_modular_xml_loading) return cb->is_modular_xml_loading() != 0;
        return false;
    }

    // =========================================================================
    // Pitboss stubs (Category A/C)
    // =========================================================================

    bool IsPitbossHost() const {
        return false;
    }

    CvString GetPitbossSmtpHost() const {
        return CvString();
    }

    CvWString GetPitbossSmtpLogin() const {
        return CvWString();
    }

    CvWString GetPitbossSmtpPassword() const {
        return CvWString();
    }

    CvString GetPitbossEmail() const {
        return CvString();
    }

    // =========================================================================
    // Send messages — stubs (Category A/C)
    // =========================================================================

    void sendMessageData(CvMessageData* pData) {
        (void)pData;
    }

    void sendPlayerInfo(PlayerTypes eActivePlayer) {
        (void)eActivePlayer;
    }

    void sendGameInfo(const CvWString& szGameName, const CvWString& szAdminPassword) {
        (void)szGameName; (void)szAdminPassword;
    }

    void sendPlayerOption(PlayerOptionTypes eOption, bool bValue) {
        (void)eOption; (void)bValue;
    }

    void sendChat(const CvWString& szChatString, ChatTargetTypes eTarget) {
        (void)szChatString; (void)eTarget;
    }

    void sendPause(int iPauseID) {
        (void)iPauseID;
    }

    void sendMPRetire() {
    }

    void sendToggleTradeMessage(PlayerTypes eWho, TradeableItems eItemType, int iData, int iOtherWho, bool bAIOffer, bool bSendToAll) {
        (void)eWho; (void)eItemType; (void)iData; (void)iOtherWho; (void)bAIOffer; (void)bSendToAll;
    }

    void sendClearTableMessage(PlayerTypes eWhoTradingWith) {
        (void)eWhoTradingWith;
    }

    void sendImplementDealMessage(PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList) {
        (void)eOtherWho; (void)pOurList; (void)pTheirList;
    }

    void sendContactCiv(NetContactTypes eContactType, PlayerTypes eWho) {
        (void)eContactType; (void)eWho;
    }

    void sendOffer() {
    }

    void sendDiploEvent(PlayerTypes eWhoTradingWith, DiploEventTypes eDiploEvent, int iData1, int iData2) {
        (void)eWhoTradingWith; (void)eDiploEvent; (void)iData1; (void)iData2;
    }

    void sendRenegotiate(PlayerTypes eWhoTradingWith) {
        (void)eWhoTradingWith;
    }

    void sendRenegotiateThisItem(PlayerTypes ePlayer2, TradeableItems eItemType, int iData) {
        (void)ePlayer2; (void)eItemType; (void)iData;
    }

    void sendExitTrade() {
    }

    void sendKillDeal(int iDealID, bool bFromDiplomacy) {
        (void)iDealID; (void)bFromDiplomacy;
    }

    void sendDiplomacy(PlayerTypes ePlayer, CvDiploParameters* pParams) {
        (void)ePlayer; (void)pParams;
    }

    void sendPopup(PlayerTypes ePlayer, CvPopupInfo* pInfo) {
        (void)ePlayer; (void)pInfo;
    }

    // =========================================================================
    // Timing stubs (Category C)
    // =========================================================================

    int getMillisecsPerTurn() {
        return 0;
    }

    float getSecsPerTurn() {
        return 0.0f;
    }

    int getTurnsPerSecond() {
        return 0;
    }

    int getTurnsPerMinute() {
        return 0;
    }

    // =========================================================================
    // Slot stubs (Category C)
    // =========================================================================

    void openSlot(PlayerTypes eID) {
        (void)eID;
    }

    void closeSlot(PlayerTypes eID) {
        (void)eID;
    }

    // =========================================================================
    // Map stubs (Category A/C)
    // =========================================================================

    CvWString getMapScriptName() {
        return CvWString();
    }

    bool getTransferredMap() {
        return false;
    }

    bool isDescFileName(const char* szFileName) {
        (void)szFileName;
        return false;
    }

    bool isWBMapScript() {
        return false;
    }

    bool isWBMapNoPlayers() {
        return false;
    }

    bool pythonMapExists(const char* szMapName) {
        (void)szMapName;
        return false;
    }

    // =========================================================================
    // String utilities (Category A)
    // =========================================================================

    void stripSpecialCharacters(CvWString& szName) {
        // No-op stub
        (void)szName;
    }

    // =========================================================================
    // Globals stubs (Category C)
    // =========================================================================

    void initGlobals() {
    }

    void uninitGlobals() {
    }

    // =========================================================================
    // Updater stub (Category C)
    // =========================================================================

    void callUpdater() {
    }

    // =========================================================================
    // Compression stubs (Category C)
    // =========================================================================

    bool Uncompress(byte** bufIn, unsigned long* bufLenIn, unsigned long maxBufLenOut, int offset) {
        (void)bufIn; (void)bufLenIn; (void)maxBufLenOut; (void)offset;
        return false;
    }

    bool Compress(byte** bufIn, unsigned long* bufLenIn, int offset) {
        (void)bufIn; (void)bufLenIn; (void)offset;
        return false;
    }

    // =========================================================================
    // UI stubs (Category C)
    // =========================================================================

    void NiTextOut(const TCHAR* szText) {
        (void)szText;
    }

    void MessageBox(const TCHAR* szText, const TCHAR* szCaption) {
        (void)szText; (void)szCaption;
        fprintf(stderr, "[TesseraRelay] MessageBox: %s — %s\n",
            szCaption ? szCaption : "(null)",
            szText ? szText : "(null)");
    }

    void SetDone(bool bDone) {
        (void)bDone;
    }

    bool GetDone() {
        return false;
    }

    bool GetAutorun() {
        return false;
    }

    // =========================================================================
    // Diplomacy stubs (Category C)
    // =========================================================================

    void beginDiplomacy(CvDiploParameters* pDiploParams, PlayerTypes ePlayer) {
        (void)pDiploParams; (void)ePlayer;
    }

    void endDiplomacy() {
    }

    bool isDiplomacy() {
        return false;
    }

    int getDiplomacyPlayer() {
        return -1;
    }

    void updateDiplomacyAttitude(bool bForce) {
        (void)bForce;
    }

    bool isMPDiplomacy() {
        return false;
    }

    bool isMPDiplomacyScreenUp() {
        return false;
    }

    int getMPDiplomacyPlayer() {
        return -1;
    }

    void beginMPDiplomacy(PlayerTypes eWhoTalkingTo, bool bRenegotiate, bool bSimultaneous) {
        (void)eWhoTalkingTo; (void)bRenegotiate; (void)bSimultaneous;
    }

    void endMPDiplomacy() {
    }

    // =========================================================================
    // Audio — callback + stubs (Category B/C)
    // =========================================================================

    bool getAudioDisabled() {
        return true;
    }

    int getAudioTagIndex(const TCHAR* szTag, int iScriptType) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->get_audio_tag_index) return cb->get_audio_tag_index(szTag, iScriptType);
        return -1;
    }

    void DoSound(int iScriptId) {
        (void)iScriptId;
    }

    void Do3DSound(int iScriptId, NiPoint3 vPosition) {
        (void)iScriptId; (void)vPosition;
    }

    // =========================================================================
    // Data stream stubs (Category C)
    // =========================================================================

    FDataStreamBase* createFileStream() {
        return NULL;
    }

    void destroyDataStream(FDataStreamBase*& stream) {
        stream = NULL;
    }

    // =========================================================================
    // Cache object creation — callback forwarding (Category B)
    // =========================================================================

    CvCacheObject* createGlobalTextCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createGlobalDefinesCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createTechInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createBuildingInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createUnitInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createLeaderHeadInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createCivilizationInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createPromotionInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createDiplomacyInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createEventInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createEventTriggerInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createCivicInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createHandicapInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createBonusInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    CvCacheObject* createImprovementInfoCacheObject(const TCHAR* szCacheFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->create_cache_object)
            return static_cast<CvCacheObject*>(cb->create_cache_object(szCacheFileName));
        return NULL;
    }

    // =========================================================================
    // Cache operations — callback forwarding (Category B)
    // =========================================================================

    bool cacheRead(CvCacheObject* pCache, const TCHAR* szSourceFileName) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->cache_read)
            return cb->cache_read(static_cast<void*>(pCache), szSourceFileName) != 0;
        return false;
    }

    bool cacheWrite(CvCacheObject* pCache) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->cache_write)
            return cb->cache_write(static_cast<void*>(pCache)) != 0;
        return false;
    }

    void destroyCache(CvCacheObject*& pCache) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->destroy_cache && pCache) {
            cb->destroy_cache(static_cast<void*>(pCache));
            pCache = NULL;
        }
    }

    // =========================================================================
    // File manager stub (Category C)
    // =========================================================================

    bool fileManagerEnabled() {
        return false;
    }

    // =========================================================================
    // Logging — callback forwarding (Category B)
    // =========================================================================

    void logMsg(const TCHAR* pLogFileName, const TCHAR* pBuf, bool bWriteToConsole, bool bTimeStamp) {
        (void)bWriteToConsole; (void)bTimeStamp;
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->log_msg) {
            cb->log_msg(pLogFileName, pBuf);
        } else {
            fprintf(stderr, "[TesseraRelay LOG] %s: %s\n",
                pLogFileName ? pLogFileName : "(null)",
                pBuf ? pBuf : "(null)");
        }
    }

    void logMemState(const char* msg) {
        (void)msg;
    }

    // =========================================================================
    // Symbol ID stubs (Category C)
    // =========================================================================

    int getSymbolID(int iID) {
        (void)iID;
        return 0;
    }

    void setSymbolID(int iID, int iValue) {
        (void)iID; (void)iValue;
    }

    // =========================================================================
    // Text — callback + placeholder (Category A)
    // =========================================================================

    CvWString getText(CvWString szIDTag, ...) {
        // Return the tag string itself as placeholder.
        // Variadic args are consumed by VS2003 — no way to forward safely.
        return szIDTag;
    }

    CvWString getObjectText(CvWString szIDTag, uint uiForm, bool bNoSubs) {
        (void)uiForm; (void)bNoSubs;
        return szIDTag;
    }

    void addText(const TCHAR* szIDTag, const wchar* szString, const wchar* szGender, const wchar* szPlural) {
        (void)szIDTag; (void)szString; (void)szGender; (void)szPlural;
    }

    uint getNumForms(CvWString szIDTag) {
        (void)szIDTag;
        return 0;
    }

    // =========================================================================
    // World / frame stubs (Category C)
    // =========================================================================

    WorldSizeTypes getWorldSize() {
        return WORLDSIZE_NONE;
    }

    uint getFrameCounter() const {
        return 0;
    }

    // =========================================================================
    // Key state stubs (Category C)
    // =========================================================================

    bool altKey() { return false; }
    bool shiftKey() { return false; }
    bool ctrlKey() { return false; }
    bool scrollLock() { return false; }
    bool capsLock() { return false; }
    bool numLock() { return false; }

    // =========================================================================
    // Profiler stubs (Category C)
    // =========================================================================

    void ProfilerBegin() {}
    void ProfilerEnd() {}
    void BeginSample(ProfileSample* pSample) { (void)pSample; }
    void EndSample(ProfileSample* pSample) { (void)pSample; }

    // =========================================================================
    // Game init — callback (Category B)
    // =========================================================================

    bool isGameInitializing() {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->is_game_initializing) return cb->is_game_initializing() != 0;
        return true;
    }

    // =========================================================================
    // File enumeration — callback forwarding (Category A)
    // =========================================================================

    void enumerateFiles(std::vector<CvString>& files, const char* szPattern) {
        HostCallbacks* cb = relay_get_callbacks();
        if (!cb || !cb->enumerate_files) return;

        const char** paths = NULL;
        int count = cb->enumerate_files(szPattern, &paths);
        if (count > 0 && paths) {
            int i;
            for (i = 0; i < count; ++i) {
                if (paths[i]) {
                    files.push_back(CvString(paths[i]));
                }
            }
            if (cb->free_file_list) {
                cb->free_file_list(paths, count);
            }
        }
    }

    void enumerateModuleFiles(std::vector<CvString>& aszFiles, const CvString& refcstrRootDirectory, const CvString& refcstrModularDirectory, const CvString& refcstrExtension, bool bSearchSubdirectories) {
        HostCallbacks* cb = relay_get_callbacks();
        if (!cb || !cb->enumerate_module_files) return;

        const char** paths = NULL;
        int count = cb->enumerate_module_files(
            refcstrRootDirectory.c_str(),
            refcstrModularDirectory.c_str(),
            refcstrExtension.c_str(),
            bSearchSubdirectories ? 1 : 0,
            &paths);
        if (count > 0 && paths) {
            int i;
            for (i = 0; i < count; ++i) {
                if (paths[i]) {
                    aszFiles.push_back(CvString(paths[i]));
                }
            }
            if (cb->free_file_list) {
                cb->free_file_list(paths, count);
            }
        }
    }

    // =========================================================================
    // Save/Load stubs (Category A/C)
    // =========================================================================

    void SaveGame(SaveGameTypes eSaveGame) {
        (void)eSaveGame;
    }

    void LoadGame() {
    }

    int loadReplays(std::vector<CvReplayInfo*>& listReplays) {
        (void)listReplays;
        return 0;
    }

    void QuickSave() {
    }

    void QuickLoad() {
    }

    void sendPbemTurn(PlayerTypes ePlayer) {
        (void)ePlayer;
    }

    void getPassword(PlayerTypes ePlayer) {
        (void)ePlayer;
    }

    // =========================================================================
    // Options stubs (Category C)
    // =========================================================================

    bool getGraphicOption(GraphicOptionTypes eGraphicOption) {
        (void)eGraphicOption;
        return false;
    }

    bool getPlayerOption(PlayerOptionTypes ePlayerOption) {
        (void)ePlayerOption;
        return false;
    }

    int getMainMenu() {
        return 0;
    }

    // =========================================================================
    // FMP stubs (Category C)
    // =========================================================================

    bool isFMPMgrHost() {
        return false;
    }

    bool isFMPMgrPublic() {
        return false;
    }

    void handleRetirement(PlayerTypes ePlayer) {
        (void)ePlayer;
    }

    PlayerTypes getFirstBadConnection() {
        return NO_PLAYER;
    }

    int getConnState(PlayerTypes ePlayer) {
        (void)ePlayer;
        return 0;
    }

    // =========================================================================
    // INI stub (Category C)
    // =========================================================================

    bool ChangeINIKeyValue(const char* szGroupKey, const char* szKeyValue, const char* szOut) {
        (void)szGroupKey; (void)szKeyValue; (void)szOut;
        return false;
    }

    // =========================================================================
    // MD5 stub (Category C)
    // =========================================================================

    char* md5String(char* szString) {
        (void)szString;
        return NULL;
    }

    // =========================================================================
    // Mod info — callback forwarding (Category A/B)
    // =========================================================================

    const char* getModName(bool bFullPath) const {
        HostCallbacks* cb = relay_get_callbacks();
        if (cb && cb->get_mod_name) return cb->get_mod_name(bFullPath ? 1 : 0);
        return "";
    }

    bool hasSkippedSaveChecksum() const {
        return false;
    }

    // =========================================================================
    // Report stub (Category C)
    // =========================================================================

    void reportStatistics() {
    }
};

// =============================================================================
// Global singleton + extern pointer
// =============================================================================
static CvDLLUtilityIFaceImpl g_relayUtilityInstance;
CvDLLUtilityIFaceBase* g_pRelayUtilityIFace = &g_relayUtilityInstance;
