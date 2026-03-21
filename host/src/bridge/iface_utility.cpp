// =============================================================================
// File:              iface_utility.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLUtilityIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_utility.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

// =============================================================================
// Sub-interface singleton externs (defined in their own iface_*.cpp files)
// =============================================================================
extern CvDLLEngineIFaceBase*        g_pEngineIFace;
extern CvDLLEntityIFaceBase*        g_pEntityIFace;
extern CvDLLInterfaceIFaceBase*     g_pInterfaceIFace;
extern CvDLLPythonIFaceBase*        g_pPythonIFace;
extern CvDLLFAStarIFaceBase*        g_pFAStarIFace;
extern CvDLLIniParserIFaceBase*     g_pIniParserIFace;
extern CvDLLEventReporterIFaceBase* g_pEventReporterIFace;
extern CvDLLSymbolIFaceBase*        g_pSymbolIFace;
extern CvDLLFeatureIFaceBase*       g_pFeatureIFace;
extern CvDLLRouteIFaceBase*         g_pRouteIFace;
extern CvDLLRiverIFaceBase*         g_pRiverIFace;
extern CvDLLFlagEntityIFaceBase*    g_pFlagEntityIFace;
extern CvDLLPlotBuilderIFaceBase*   g_pPlotBuilderIFace;
extern CvDLLXmlIFaceBase*           g_pXmlIFace;

// =============================================================================
// Impl class
// =============================================================================

class CvDLLUtilityIFaceImpl : public CvDLLUtilityIFaceBase
{
public:
	// =========================================================================
	// Sub-interface accessors
	// =========================================================================

	CvDLLEntityIFaceBase* getEntityIFace() override {
		fprintf(stderr, "[UTILITY STUB] getEntityIFace\n");
		return g_pEntityIFace;
	}

	CvDLLInterfaceIFaceBase* getInterfaceIFace() override {
		fprintf(stderr, "[UTILITY STUB] getInterfaceIFace\n");
		return g_pInterfaceIFace;
	}

	CvDLLEngineIFaceBase* getEngineIFace() override {
		fprintf(stderr, "[UTILITY STUB] getEngineIFace\n");
		return g_pEngineIFace;
	}

	CvDLLIniParserIFaceBase* getIniParserIFace() override {
		fprintf(stderr, "[UTILITY STUB] getIniParserIFace\n");
		return g_pIniParserIFace;
	}

	CvDLLSymbolIFaceBase* getSymbolIFace() override {
		fprintf(stderr, "[UTILITY STUB] getSymbolIFace\n");
		return g_pSymbolIFace;
	}

	CvDLLFeatureIFaceBase* getFeatureIFace() override {
		fprintf(stderr, "[UTILITY STUB] getFeatureIFace\n");
		return g_pFeatureIFace;
	}

	CvDLLRouteIFaceBase* getRouteIFace() override {
		fprintf(stderr, "[UTILITY STUB] getRouteIFace\n");
		return g_pRouteIFace;
	}

	CvDLLPlotBuilderIFaceBase* getPlotBuilderIFace() override {
		fprintf(stderr, "[UTILITY STUB] getPlotBuilderIFace\n");
		return g_pPlotBuilderIFace;
	}

	CvDLLRiverIFaceBase* getRiverIFace() override {
		fprintf(stderr, "[UTILITY STUB] getRiverIFace\n");
		return g_pRiverIFace;
	}

	CvDLLFAStarIFaceBase* getFAStarIFace() override {
		fprintf(stderr, "[UTILITY STUB] getFAStarIFace\n");
		return g_pFAStarIFace;
	}

	CvDLLXmlIFaceBase* getXMLIFace() override {
		fprintf(stderr, "[UTILITY STUB] getXMLIFace\n");
		return g_pXmlIFace;
	}

	CvDLLFlagEntityIFaceBase* getFlagEntityIFace() override {
		fprintf(stderr, "[UTILITY STUB] getFlagEntityIFace\n");
		return g_pFlagEntityIFace;
	}

	CvDLLPythonIFaceBase* getPythonIFace() override {
		fprintf(stderr, "[UTILITY STUB] getPythonIFace\n");
		return g_pPythonIFace;
	}

	// =========================================================================
	// Memory management
	// =========================================================================

	void delMem(void* p) override {
		fprintf(stderr, "[UTILITY STUB] delMem(%p)\n", p);
		free(p);
	}

	void* newMem(size_t size) override {
		fprintf(stderr, "[UTILITY STUB] newMem(%zu)\n", size);
		return malloc(size);
	}

	void delMem(void* p, const char* pcFile, int iLine) override {
		fprintf(stderr, "[UTILITY STUB] delMem(%p, %s:%d)\n", p, pcFile ? pcFile : "", iLine);
		free(p);
	}

	void* newMem(size_t size, const char* pcFile, int iLine) override {
		fprintf(stderr, "[UTILITY STUB] newMem(%zu, %s:%d)\n", size, pcFile ? pcFile : "", iLine);
		return malloc(size);
	}

	void delMemArray(void* p, const char* pcFile, int iLine) override {
		fprintf(stderr, "[UTILITY STUB] delMemArray(%p, %s:%d)\n", p, pcFile ? pcFile : "", iLine);
		free(p);
	}

	void* newMemArray(size_t size, const char* pcFile, int iLine) override {
		fprintf(stderr, "[UTILITY STUB] newMemArray(%zu, %s:%d)\n", size, pcFile ? pcFile : "", iLine);
		return malloc(size);
	}

	void* reallocMem(void* a, unsigned int uiBytes, const char* pcFile, int iLine) override {
		fprintf(stderr, "[UTILITY STUB] reallocMem(%p, %u, %s:%d)\n", a, uiBytes, pcFile ? pcFile : "", iLine);
		return realloc(a, uiBytes);
	}

	unsigned int memSize(void* a) override {
		fprintf(stderr, "[UTILITY STUB] memSize\n");
		return 0;
	}

	// =========================================================================
	// Vector clearing
	// =========================================================================

	void clearVector(std::vector<int>& vec) override {
		fprintf(stderr, "[UTILITY STUB] clearVector(int)\n");
		vec.clear();
	}

	void clearVector(std::vector<byte>& vec) override {
		fprintf(stderr, "[UTILITY STUB] clearVector(byte)\n");
		vec.clear();
	}

	void clearVector(std::vector<float>& vec) override {
		fprintf(stderr, "[UTILITY STUB] clearVector(float)\n");
		vec.clear();
	}

	// =========================================================================
	// Network
	// =========================================================================

	int getAssignedNetworkID(int iPlayerID) override {
		fprintf(stderr, "[UTILITY STUB] getAssignedNetworkID(%d)\n", iPlayerID);
		return 0;
	}

	bool isConnected(int iNetID) override {
		fprintf(stderr, "[UTILITY STUB] isConnected(%d)\n", iNetID);
		return false;
	}

	bool isGameActive() override {
		fprintf(stderr, "[UTILITY STUB] isGameActive\n");
		return false;
	}

	int GetLocalNetworkID() override {
		fprintf(stderr, "[UTILITY STUB] GetLocalNetworkID\n");
		return 0;
	}

	int GetSyncOOS(int iNetID) override {
		fprintf(stderr, "[UTILITY STUB] GetSyncOOS(%d)\n", iNetID);
		return 0;
	}

	int GetOptionsOOS(int iNetID) override {
		fprintf(stderr, "[UTILITY STUB] GetOptionsOOS(%d)\n", iNetID);
		return 0;
	}

	int GetLastPing(int iNetID) override {
		fprintf(stderr, "[UTILITY STUB] GetLastPing(%d)\n", iNetID);
		return 0;
	}

	// =========================================================================
	// Modem
	// =========================================================================

	bool IsModem() override {
		fprintf(stderr, "[UTILITY STUB] IsModem\n");
		return false;
	}

	void SetModem(bool bModem) override {
		fprintf(stderr, "[UTILITY STUB] SetModem(%d)\n", bModem);
	}

	// =========================================================================
	// Buddy system
	// =========================================================================

	void AcceptBuddy(const char* szName, int iRequestID) override {
		fprintf(stderr, "[UTILITY STUB] AcceptBuddy: %s\n", szName ? szName : "(null)");
	}

	void RejectBuddy(const char* szName, int iRequestID) override {
		fprintf(stderr, "[UTILITY STUB] RejectBuddy: %s\n", szName ? szName : "(null)");
	}

	// =========================================================================
	// Message control / Cheat level
	// =========================================================================

	void messageControlLog(char* s) override {
		fprintf(stderr, "[UTILITY STUB] messageControlLog: %s\n", s ? s : "(null)");
	}

	int getChtLvl() override {
		fprintf(stderr, "[UTILITY STUB] getChtLvl\n");
		return 0;
	}

	void setChtLvl(int iLevel) override {
		fprintf(stderr, "[UTILITY STUB] setChtLvl(%d)\n", iLevel);
	}

	bool GetWorldBuilderMode() override {
		fprintf(stderr, "[UTILITY STUB] GetWorldBuilderMode\n");
		return false;
	}

	int getCurrentLanguage() const override {
		fprintf(stderr, "[UTILITY STUB] getCurrentLanguage\n");
		return 0;
	}

	void setCurrentLanguage(int iNewLanguage) override {
		fprintf(stderr, "[UTILITY STUB] setCurrentLanguage(%d)\n", iNewLanguage);
	}

	bool isModularXMLLoading() const override {
		fprintf(stderr, "[UTILITY STUB] isModularXMLLoading\n");
		return false;
	}

	// =========================================================================
	// Pitboss
	// =========================================================================

	bool IsPitbossHost() const override {
		fprintf(stderr, "[UTILITY STUB] IsPitbossHost\n");
		return false;
	}

	CvString GetPitbossSmtpHost() const override {
		fprintf(stderr, "[UTILITY STUB] GetPitbossSmtpHost\n");
		return "";
	}

	CvWString GetPitbossSmtpLogin() const override {
		fprintf(stderr, "[UTILITY STUB] GetPitbossSmtpLogin\n");
		return L"";
	}

	CvWString GetPitbossSmtpPassword() const override {
		fprintf(stderr, "[UTILITY STUB] GetPitbossSmtpPassword\n");
		return L"";
	}

	CvString GetPitbossEmail() const override {
		fprintf(stderr, "[UTILITY STUB] GetPitbossEmail\n");
		return "";
	}

	// =========================================================================
	// Send messages
	// =========================================================================

	void sendMessageData(CvMessageData* pData) override {
		fprintf(stderr, "[UTILITY STUB] sendMessageData\n");
	}

	void sendPlayerInfo(PlayerTypes eActivePlayer) override {
		fprintf(stderr, "[UTILITY STUB] sendPlayerInfo\n");
	}

	void sendGameInfo(const CvWString& szGameName, const CvWString& szAdminPassword) override {
		fprintf(stderr, "[UTILITY STUB] sendGameInfo\n");
	}

	void sendPlayerOption(PlayerOptionTypes eOption, bool bValue) override {
		fprintf(stderr, "[UTILITY STUB] sendPlayerOption\n");
	}

	void sendChat(const CvWString& szChatString, ChatTargetTypes eTarget) override {
		fprintf(stderr, "[UTILITY STUB] sendChat\n");
	}

	void sendPause(int iPauseID) override {
		fprintf(stderr, "[UTILITY STUB] sendPause(%d)\n", iPauseID);
	}

	void sendMPRetire() override {
		fprintf(stderr, "[UTILITY STUB] sendMPRetire\n");
	}

	void sendToggleTradeMessage(PlayerTypes eWho, TradeableItems eItemType, int iData, int iOtherWho, bool bAIOffer, bool bSendToAll) override {
		fprintf(stderr, "[UTILITY STUB] sendToggleTradeMessage\n");
	}

	void sendClearTableMessage(PlayerTypes eWhoTradingWith) override {
		fprintf(stderr, "[UTILITY STUB] sendClearTableMessage\n");
	}

	void sendImplementDealMessage(PlayerTypes eOtherWho, CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirList) override {
		fprintf(stderr, "[UTILITY STUB] sendImplementDealMessage\n");
	}

	void sendContactCiv(NetContactTypes eContactType, PlayerTypes eWho) override {
		fprintf(stderr, "[UTILITY STUB] sendContactCiv\n");
	}

	void sendOffer() override {
		fprintf(stderr, "[UTILITY STUB] sendOffer\n");
	}

	void sendDiploEvent(PlayerTypes eWhoTradingWith, DiploEventTypes eDiploEvent, int iData1, int iData2) override {
		fprintf(stderr, "[UTILITY STUB] sendDiploEvent\n");
	}

	void sendRenegotiate(PlayerTypes eWhoTradingWith) override {
		fprintf(stderr, "[UTILITY STUB] sendRenegotiate\n");
	}

	void sendRenegotiateThisItem(PlayerTypes ePlayer2, TradeableItems eItemType, int iData) override {
		fprintf(stderr, "[UTILITY STUB] sendRenegotiateThisItem\n");
	}

	void sendExitTrade() override {
		fprintf(stderr, "[UTILITY STUB] sendExitTrade\n");
	}

	void sendKillDeal(int iDealID, bool bFromDiplomacy) override {
		fprintf(stderr, "[UTILITY STUB] sendKillDeal(%d)\n", iDealID);
	}

	void sendDiplomacy(PlayerTypes ePlayer, CvDiploParameters* pParams) override {
		fprintf(stderr, "[UTILITY STUB] sendDiplomacy\n");
	}

	void sendPopup(PlayerTypes ePlayer, CvPopupInfo* pInfo) override {
		fprintf(stderr, "[UTILITY STUB] sendPopup\n");
	}

	// =========================================================================
	// Timing
	// =========================================================================

	int getMillisecsPerTurn() override {
		fprintf(stderr, "[UTILITY STUB] getMillisecsPerTurn\n");
		return 0;
	}

	float getSecsPerTurn() override {
		fprintf(stderr, "[UTILITY STUB] getSecsPerTurn\n");
		return 0.0f;
	}

	int getTurnsPerSecond() override {
		fprintf(stderr, "[UTILITY STUB] getTurnsPerSecond\n");
		return 0;
	}

	int getTurnsPerMinute() override {
		fprintf(stderr, "[UTILITY STUB] getTurnsPerMinute\n");
		return 0;
	}

	// =========================================================================
	// Slots
	// =========================================================================

	void openSlot(PlayerTypes eID) override {
		fprintf(stderr, "[UTILITY STUB] openSlot\n");
	}

	void closeSlot(PlayerTypes eID) override {
		fprintf(stderr, "[UTILITY STUB] closeSlot\n");
	}

	// =========================================================================
	// Map scripts
	// =========================================================================

	CvWString getMapScriptName() override {
		fprintf(stderr, "[UTILITY STUB] getMapScriptName\n");
		return L"";
	}

	bool getTransferredMap() override {
		fprintf(stderr, "[UTILITY STUB] getTransferredMap\n");
		return false;
	}

	bool isDescFileName(const char* szFileName) override {
		fprintf(stderr, "[UTILITY STUB] isDescFileName: %s\n", szFileName ? szFileName : "(null)");
		return false;
	}

	bool isWBMapScript() override {
		fprintf(stderr, "[UTILITY STUB] isWBMapScript\n");
		return false;
	}

	bool isWBMapNoPlayers() override {
		fprintf(stderr, "[UTILITY STUB] isWBMapNoPlayers\n");
		return false;
	}

	bool pythonMapExists(const char* szMapName) override {
		fprintf(stderr, "[UTILITY STUB] pythonMapExists: %s\n", szMapName ? szMapName : "(null)");
		return false;
	}

	// =========================================================================
	// String utilities
	// =========================================================================

	void stripSpecialCharacters(CvWString& szName) override {
		fprintf(stderr, "[UTILITY STUB] stripSpecialCharacters\n");
	}

	// =========================================================================
	// Globals init/uninit
	// =========================================================================

	void initGlobals() override {
		fprintf(stderr, "[UTILITY STUB] initGlobals\n");
	}

	void uninitGlobals() override {
		fprintf(stderr, "[UTILITY STUB] uninitGlobals\n");
	}

	// =========================================================================
	// Updater
	// =========================================================================

	void callUpdater() override {
		fprintf(stderr, "[UTILITY STUB] callUpdater\n");
	}

	// =========================================================================
	// Compression
	// =========================================================================

	bool Uncompress(byte** bufIn, unsigned long* bufLenIn, unsigned long maxBufLenOut, int offset) override {
		fprintf(stderr, "[UTILITY STUB] Uncompress\n");
		return false;
	}

	bool Compress(byte** bufIn, unsigned long* bufLenIn, int offset) override {
		fprintf(stderr, "[UTILITY STUB] Compress\n");
		return false;
	}

	// =========================================================================
	// UI / Application state
	// =========================================================================

	void NiTextOut(const TCHAR* szText) override {
		fprintf(stderr, "[UTILITY STUB] NiTextOut: %s\n", szText ? szText : "(null)");
	}

	void MessageBox(const TCHAR* szText, const TCHAR* szCaption) override {
		fprintf(stderr, "[UTILITY STUB] MessageBox: %s — %s\n",
			szCaption ? szCaption : "(null)",
			szText ? szText : "(null)");
	}

	void SetDone(bool bDone) override {
		fprintf(stderr, "[UTILITY STUB] SetDone(%d)\n", bDone);
	}

	bool GetDone() override {
		fprintf(stderr, "[UTILITY STUB] GetDone\n");
		return false;
	}

	bool GetAutorun() override {
		fprintf(stderr, "[UTILITY STUB] GetAutorun\n");
		return false;
	}

	// =========================================================================
	// Diplomacy
	// =========================================================================

	void beginDiplomacy(CvDiploParameters* pDiploParams, PlayerTypes ePlayer) override {
		fprintf(stderr, "[UTILITY STUB] beginDiplomacy\n");
	}

	void endDiplomacy() override {
		fprintf(stderr, "[UTILITY STUB] endDiplomacy\n");
	}

	bool isDiplomacy() override {
		fprintf(stderr, "[UTILITY STUB] isDiplomacy\n");
		return false;
	}

	int getDiplomacyPlayer() override {
		fprintf(stderr, "[UTILITY STUB] getDiplomacyPlayer\n");
		return 0;
	}

	void updateDiplomacyAttitude(bool bForce) override {
		fprintf(stderr, "[UTILITY STUB] updateDiplomacyAttitude\n");
	}

	bool isMPDiplomacy() override {
		fprintf(stderr, "[UTILITY STUB] isMPDiplomacy\n");
		return false;
	}

	bool isMPDiplomacyScreenUp() override {
		fprintf(stderr, "[UTILITY STUB] isMPDiplomacyScreenUp\n");
		return false;
	}

	int getMPDiplomacyPlayer() override {
		fprintf(stderr, "[UTILITY STUB] getMPDiplomacyPlayer\n");
		return 0;
	}

	void beginMPDiplomacy(PlayerTypes eWhoTalkingTo, bool bRenegotiate, bool bSimultaneous) override {
		fprintf(stderr, "[UTILITY STUB] beginMPDiplomacy\n");
	}

	void endMPDiplomacy() override {
		fprintf(stderr, "[UTILITY STUB] endMPDiplomacy\n");
	}

	// =========================================================================
	// Audio
	// =========================================================================

	bool getAudioDisabled() override {
		fprintf(stderr, "[UTILITY STUB] getAudioDisabled\n");
		return false;
	}

	int getAudioTagIndex(const TCHAR* szTag, int iScriptType) override {
		fprintf(stderr, "[UTILITY STUB] getAudioTagIndex: %s\n", szTag ? szTag : "(null)");
		return 0;
	}

	void DoSound(int iScriptId) override {
		fprintf(stderr, "[UTILITY STUB] DoSound(%d)\n", iScriptId);
	}

	void Do3DSound(int iScriptId, NiPoint3 vPosition) override {
		fprintf(stderr, "[UTILITY STUB] Do3DSound(%d)\n", iScriptId);
	}

	// =========================================================================
	// Data streams
	// =========================================================================

	FDataStreamBase* createFileStream() override {
		fprintf(stderr, "[UTILITY STUB] createFileStream\n");
		return nullptr;
	}

	void destroyDataStream(FDataStreamBase*& stream) override {
		fprintf(stderr, "[UTILITY STUB] destroyDataStream\n");
		stream = nullptr;
	}

	// =========================================================================
	// Cache objects
	// =========================================================================

	CvCacheObject* createGlobalTextCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createGlobalTextCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createGlobalDefinesCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createGlobalDefinesCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createTechInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createTechInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createBuildingInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createBuildingInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createUnitInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createUnitInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createLeaderHeadInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createLeaderHeadInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createCivilizationInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createCivilizationInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createPromotionInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createPromotionInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createDiplomacyInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createDiplomacyInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createEventInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createEventInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createEventTriggerInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createEventTriggerInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createCivicInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createCivicInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createHandicapInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createHandicapInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createBonusInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createBonusInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	CvCacheObject* createImprovementInfoCacheObject(const TCHAR* szCacheFileName) override {
		fprintf(stderr, "[UTILITY STUB] createImprovementInfoCacheObject: %s\n", szCacheFileName ? szCacheFileName : "(null)");
		return nullptr;
	}

	// =========================================================================
	// Cache read/write/destroy
	// =========================================================================

	bool cacheRead(CvCacheObject* pCache, const TCHAR* szSourceFileName) override {
		fprintf(stderr, "[UTILITY STUB] cacheRead: %s\n", szSourceFileName ? szSourceFileName : "(null)");
		return false;
	}

	bool cacheWrite(CvCacheObject* pCache) override {
		fprintf(stderr, "[UTILITY STUB] cacheWrite\n");
		return false;
	}

	void destroyCache(CvCacheObject*& pCache) override {
		fprintf(stderr, "[UTILITY STUB] destroyCache\n");
		pCache = nullptr;
	}

	// =========================================================================
	// File manager
	// =========================================================================

	bool fileManagerEnabled() override {
		fprintf(stderr, "[UTILITY STUB] fileManagerEnabled\n");
		return false;
	}

	// =========================================================================
	// Logging
	// =========================================================================

	void logMsg(const TCHAR* pLogFileName, const TCHAR* pBuf, bool bWriteToConsole, bool bTimeStamp) override {
		fprintf(stderr, "[DLL LOG] %s: %s\n",
			pLogFileName ? pLogFileName : "",
			pBuf ? pBuf : "");
	}

	void logMemState(const char* msg) override {
		fprintf(stderr, "[UTILITY STUB] logMemState: %s\n", msg ? msg : "(null)");
	}

	// =========================================================================
	// Symbol ID
	// =========================================================================

	int getSymbolID(int iID) override {
		fprintf(stderr, "[UTILITY STUB] getSymbolID(%d)\n", iID);
		return 0;
	}

	void setSymbolID(int iID, int iValue) override {
		fprintf(stderr, "[UTILITY STUB] setSymbolID(%d, %d)\n", iID, iValue);
	}

	// =========================================================================
	// Text / Localization
	// =========================================================================

	CvWString getText(CvWString szIDTag, ...) override {
		fprintf(stderr, "[UTILITY STUB] getText\n");
		return L"";
	}

	CvWString getObjectText(CvWString szIDTag, uint uiForm, bool bNoSubs) override {
		fprintf(stderr, "[UTILITY STUB] getObjectText\n");
		return L"";
	}

	void addText(const TCHAR* szIDTag, const wchar* szString, const wchar* szGender, const wchar* szPlural) override {
		fprintf(stderr, "[UTILITY STUB] addText: %s\n", szIDTag ? szIDTag : "(null)");
	}

	uint getNumForms(CvWString szIDTag) override {
		fprintf(stderr, "[UTILITY STUB] getNumForms\n");
		return 0;
	}

	// =========================================================================
	// World / Frame
	// =========================================================================

	WorldSizeTypes getWorldSize() override {
		fprintf(stderr, "[UTILITY STUB] getWorldSize\n");
		return WorldSizeTypes{};
	}

	uint getFrameCounter() const override {
		fprintf(stderr, "[UTILITY STUB] getFrameCounter\n");
		return 0;
	}

	// =========================================================================
	// Key state
	// =========================================================================

	bool altKey() override {
		fprintf(stderr, "[UTILITY STUB] altKey\n");
		return false;
	}

	bool shiftKey() override {
		fprintf(stderr, "[UTILITY STUB] shiftKey\n");
		return false;
	}

	bool ctrlKey() override {
		fprintf(stderr, "[UTILITY STUB] ctrlKey\n");
		return false;
	}

	bool scrollLock() override {
		fprintf(stderr, "[UTILITY STUB] scrollLock\n");
		return false;
	}

	bool capsLock() override {
		fprintf(stderr, "[UTILITY STUB] capsLock\n");
		return false;
	}

	bool numLock() override {
		fprintf(stderr, "[UTILITY STUB] numLock\n");
		return false;
	}

	// =========================================================================
	// Profiling
	// =========================================================================

	void ProfilerBegin() override {
		fprintf(stderr, "[UTILITY STUB] ProfilerBegin\n");
	}

	void ProfilerEnd() override {
		fprintf(stderr, "[UTILITY STUB] ProfilerEnd\n");
	}

	void BeginSample(ProfileSample* pSample) override {
		// Intentionally silent — profiling is high-frequency
	}

	void EndSample(ProfileSample* pSample) override {
		// Intentionally silent — profiling is high-frequency
	}

	bool isGameInitializing() override {
		fprintf(stderr, "[UTILITY STUB] isGameInitializing\n");
		return false;
	}

	// =========================================================================
	// File enumeration
	// =========================================================================

	void enumerateFiles(std::vector<CvString>& files, const char* szPattern) override {
		fprintf(stderr, "[UTILITY STUB] enumerateFiles: %s\n", szPattern ? szPattern : "(null)");
	}

	void enumerateModuleFiles(std::vector<CvString>& aszFiles, const CvString& refcstrRootDirectory, const CvString& refcstrModularDirectory, const CvString& refcstrExtension, bool bSearchSubdirectories) override {
		fprintf(stderr, "[UTILITY STUB] enumerateModuleFiles: %s / %s\n",
			refcstrRootDirectory.c_str(), refcstrExtension.c_str());
	}

	// =========================================================================
	// Save / Load
	// =========================================================================

	void SaveGame(SaveGameTypes eSaveGame) override {
		fprintf(stderr, "[UTILITY STUB] SaveGame\n");
	}

	void LoadGame() override {
		fprintf(stderr, "[UTILITY STUB] LoadGame\n");
	}

	int loadReplays(std::vector<CvReplayInfo*>& listReplays) override {
		fprintf(stderr, "[UTILITY STUB] loadReplays\n");
		return 0;
	}

	void QuickSave() override {
		fprintf(stderr, "[UTILITY STUB] QuickSave\n");
	}

	void QuickLoad() override {
		fprintf(stderr, "[UTILITY STUB] QuickLoad\n");
	}

	void sendPbemTurn(PlayerTypes ePlayer) override {
		fprintf(stderr, "[UTILITY STUB] sendPbemTurn\n");
	}

	void getPassword(PlayerTypes ePlayer) override {
		fprintf(stderr, "[UTILITY STUB] getPassword\n");
	}

	// =========================================================================
	// Options
	// =========================================================================

	bool getGraphicOption(GraphicOptionTypes eGraphicOption) override {
		fprintf(stderr, "[UTILITY STUB] getGraphicOption\n");
		return false;
	}

	bool getPlayerOption(PlayerOptionTypes ePlayerOption) override {
		fprintf(stderr, "[UTILITY STUB] getPlayerOption\n");
		return false;
	}

	int getMainMenu() override {
		fprintf(stderr, "[UTILITY STUB] getMainMenu\n");
		return 0;
	}

	// =========================================================================
	// FMP Manager
	// =========================================================================

	bool isFMPMgrHost() override {
		fprintf(stderr, "[UTILITY STUB] isFMPMgrHost\n");
		return false;
	}

	bool isFMPMgrPublic() override {
		fprintf(stderr, "[UTILITY STUB] isFMPMgrPublic\n");
		return false;
	}

	void handleRetirement(PlayerTypes ePlayer) override {
		fprintf(stderr, "[UTILITY STUB] handleRetirement\n");
	}

	PlayerTypes getFirstBadConnection() override {
		fprintf(stderr, "[UTILITY STUB] getFirstBadConnection\n");
		return NO_PLAYER;
	}

	int getConnState(PlayerTypes ePlayer) override {
		fprintf(stderr, "[UTILITY STUB] getConnState\n");
		return 0;
	}

	// =========================================================================
	// INI
	// =========================================================================

	bool ChangeINIKeyValue(const char* szGroupKey, const char* szKeyValue, const char* szOut) override {
		fprintf(stderr, "[UTILITY STUB] ChangeINIKeyValue: %s / %s\n",
			szGroupKey ? szGroupKey : "(null)",
			szKeyValue ? szKeyValue : "(null)");
		return false;
	}

	// =========================================================================
	// MD5
	// =========================================================================

	char* md5String(char* szString) override {
		fprintf(stderr, "[UTILITY STUB] md5String\n");
		return nullptr;
	}

	// =========================================================================
	// Mod info
	// =========================================================================

	const char* getModName(bool bFullPath) const override {
		fprintf(stderr, "[UTILITY STUB] getModName\n");
		return "";
	}

	bool hasSkippedSaveChecksum() const override {
		fprintf(stderr, "[UTILITY STUB] hasSkippedSaveChecksum\n");
		return false;
	}

	void reportStatistics() override {
		fprintf(stderr, "[UTILITY STUB] reportStatistics\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLUtilityIFaceImpl g_utilityIFaceInstance;
CvDLLUtilityIFaceBase* g_pUtilityIFace = &g_utilityIFaceInstance;
