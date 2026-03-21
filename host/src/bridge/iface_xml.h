// =============================================================================
// File:              iface_xml.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLXmlIFaceBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

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
	virtual bool SetInsertedNodeAttribute(FXml* xml, TCHAR* pszAttributeName, TCHAR* pszAttributeValue) = 0;
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
