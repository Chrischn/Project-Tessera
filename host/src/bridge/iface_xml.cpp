// =============================================================================
// File:              iface_xml.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLXmlIFaceBase. All methods log and return safe
//   defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_xml.h"
#include <cstdio>

class CvDLLXmlIFaceImpl : public CvDLLXmlIFaceBase
{
public:
	// =========================================================================
	// Creation / Destruction
	// =========================================================================

	FXml* CreateFXml(FXmlSchemaCache* pSchemaCache) override {
		fprintf(stderr, "[XML STUB] CreateFXml\n");
		return nullptr;
	}

	void DestroyFXml(FXml*& xml) override {
		fprintf(stderr, "[XML STUB] DestroyFXml\n");
		xml = nullptr;
	}

	void DestroyFXmlSchemaCache(FXmlSchemaCache*& pCache) override {
		fprintf(stderr, "[XML STUB] DestroyFXmlSchemaCache\n");
		pCache = nullptr;
	}

	FXmlSchemaCache* CreateFXmlSchemaCache() override {
		fprintf(stderr, "[XML STUB] CreateFXmlSchemaCache\n");
		return nullptr;
	}

	// =========================================================================
	// Load / Validate / Write
	// =========================================================================

	bool LoadXml(FXml* xml, const TCHAR* pszXmlFile) override {
		fprintf(stderr, "[XML STUB] LoadXml: %s\n", pszXmlFile ? pszXmlFile : "(null)");
		return false;
	}

	bool Validate(FXml* xml, TCHAR* pszError) override {
		fprintf(stderr, "[XML STUB] Validate\n");
		return false;
	}

	bool WriteXml(FXml* xml, TCHAR* pszXmlFile) override {
		fprintf(stderr, "[XML STUB] WriteXml: %s\n", pszXmlFile ? pszXmlFile : "(null)");
		return false;
	}

	// =========================================================================
	// Navigation
	// =========================================================================

	bool LocateNode(FXml* xml, const TCHAR* pszXmlNode) override {
		fprintf(stderr, "[XML STUB] LocateNode: %s\n", pszXmlNode ? pszXmlNode : "(null)");
		return false;
	}

	bool LocateFirstSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) override {
		fprintf(stderr, "[XML STUB] LocateFirstSiblingNodeByTagName: %s\n", pszTagName ? pszTagName : "(null)");
		return false;
	}

	bool LocateNextSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) override {
		fprintf(stderr, "[XML STUB] LocateNextSiblingNodeByTagName: %s\n", pszTagName ? pszTagName : "(null)");
		return false;
	}

	bool NextSibling(FXml* xml) override {
		fprintf(stderr, "[XML STUB] NextSibling\n");
		return false;
	}

	bool PrevSibling(FXml* xml) override {
		fprintf(stderr, "[XML STUB] PrevSibling\n");
		return false;
	}

	bool SetToChild(FXml* xml) override {
		fprintf(stderr, "[XML STUB] SetToChild\n");
		return false;
	}

	bool SetToChildByTagName(FXml* xml, const TCHAR* szTagName) override {
		fprintf(stderr, "[XML STUB] SetToChildByTagName: %s\n", szTagName ? szTagName : "(null)");
		return false;
	}

	bool SetToParent(FXml* xml) override {
		fprintf(stderr, "[XML STUB] SetToParent\n");
		return false;
	}

	// =========================================================================
	// Insertion
	// =========================================================================

	bool AddChildNode(FXml* xml, TCHAR* pszNewNode) override {
		fprintf(stderr, "[XML STUB] AddChildNode\n");
		return false;
	}

	bool AddSiblingNodeBefore(FXml* xml, TCHAR* pszNewNode) override {
		fprintf(stderr, "[XML STUB] AddSiblingNodeBefore\n");
		return false;
	}

	bool AddSiblingNodeAfter(FXml* xml, TCHAR* pszNewNode) override {
		fprintf(stderr, "[XML STUB] AddSiblingNodeAfter\n");
		return false;
	}

	bool SetInsertedNodeAttribute(FXml* xml, TCHAR* pszAttributeName, TCHAR* pszAttributeValue) override {
		fprintf(stderr, "[XML STUB] SetInsertedNodeAttribute\n");
		return false;
	}

	// =========================================================================
	// Text / Value Getters
	// =========================================================================

	int GetLastNodeTextSize(FXml* xml) override {
		fprintf(stderr, "[XML STUB] GetLastNodeTextSize\n");
		return 0;
	}

	bool GetLastNodeText(FXml* xml, TCHAR* pszText) override {
		fprintf(stderr, "[XML STUB] GetLastNodeText\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, std::string& pszText) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(std::string)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, std::wstring& pszText) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(std::wstring)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, char* pszText) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(char*)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, wchar* pszText) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(wchar*)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, bool* pbVal) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(bool*)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, int* piVal) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(int*)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, float* pfVal) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(float*)\n");
		return false;
	}

	bool GetLastNodeValue(FXml* xml, unsigned int* puiVal) override {
		fprintf(stderr, "[XML STUB] GetLastNodeValue(uint*)\n");
		return false;
	}

	// =========================================================================
	// Inserted Node Text
	// =========================================================================

	int GetInsertedNodeTextSize(FXml* xml) override {
		fprintf(stderr, "[XML STUB] GetInsertedNodeTextSize\n");
		return 0;
	}

	bool GetInsertedNodeText(FXml* xml, TCHAR* pszText) override {
		fprintf(stderr, "[XML STUB] GetInsertedNodeText\n");
		return false;
	}

	bool SetInsertedNodeText(FXml* xml, TCHAR* pszText) override {
		fprintf(stderr, "[XML STUB] SetInsertedNodeText\n");
		return false;
	}

	// =========================================================================
	// Node Type Queries
	// =========================================================================

	bool GetLastLocatedNodeType(FXml* xml, TCHAR* pszType) override {
		fprintf(stderr, "[XML STUB] GetLastLocatedNodeType\n");
		return false;
	}

	bool GetLastInsertedNodeType(FXml* xml, TCHAR* pszType) override {
		fprintf(stderr, "[XML STUB] GetLastInsertedNodeType\n");
		return false;
	}

	bool IsLastLocatedNodeCommentNode(FXml* xml) override {
		fprintf(stderr, "[XML STUB] IsLastLocatedNodeCommentNode\n");
		return false;
	}

	// =========================================================================
	// Counting / Enumeration
	// =========================================================================

	int NumOfElementsByTagName(FXml* xml, TCHAR* pszTagName) override {
		fprintf(stderr, "[XML STUB] NumOfElementsByTagName: %s\n", pszTagName ? pszTagName : "(null)");
		return 0;
	}

	int NumOfChildrenByTagName(FXml* xml, const TCHAR* pszTagName) override {
		fprintf(stderr, "[XML STUB] NumOfChildrenByTagName: %s\n", pszTagName ? pszTagName : "(null)");
		return 0;
	}

	int GetNumSiblings(FXml* xml) override {
		fprintf(stderr, "[XML STUB] GetNumSiblings\n");
		return 0;
	}

	int GetNumChildren(FXml* xml) override {
		fprintf(stderr, "[XML STUB] GetNumChildren\n");
		return 0;
	}

	// =========================================================================
	// Misc
	// =========================================================================

	bool GetLastLocatedNodeTagName(FXml* xml, TCHAR* pszTagName) override {
		fprintf(stderr, "[XML STUB] GetLastLocatedNodeTagName\n");
		return false;
	}

	bool IsAllowXMLCaching() override {
		fprintf(stderr, "[XML STUB] IsAllowXMLCaching\n");
		return false;
	}

	void MapChildren(FXml* xml) override {
		fprintf(stderr, "[XML STUB] MapChildren\n");
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLXmlIFaceImpl g_xmlIFaceInstance;
CvDLLXmlIFaceBase* g_pXmlIFace = &g_xmlIFaceInstance;
