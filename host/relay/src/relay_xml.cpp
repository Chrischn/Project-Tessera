// =============================================================================
// File:              relay_xml.cpp
// Author(s):         Chrischn89
// Description:
//   VS2003-compiled CvDLLXmlIFaceBase implementation. Translates between
//   VS2003 STL types (std::string, std::wstring) and C types (char*, wchar_t*),
//   forwarding all calls to the VS2022 host via HostCallbacks function pointers.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "relay_types.h"
#include "relay_api.h"
#include <cstdio>
#include <cstring>

// Access to global callbacks (defined in relay_main.cpp)
extern HostCallbacks* relay_get_callbacks();

// =============================================================================
// Implementation class
// =============================================================================

class CvDLLXmlIFaceImpl : public CvDLLXmlIFaceBase
{
public:
    // --- Creation / Destruction ---

    FXml* CreateFXml(FXmlSchemaCache* pSchemaCache) {
        HostCallbacks* cb = relay_get_callbacks();
        void* xml = cb->xml_create(static_cast<void*>(pSchemaCache));
        return static_cast<FXml*>(xml);
    }

    void DestroyFXml(FXml*& xml) {
        HostCallbacks* cb = relay_get_callbacks();
        if (xml) {
            cb->xml_destroy(static_cast<void*>(xml));
            xml = NULL;
        }
    }

    void DestroyFXmlSchemaCache(FXmlSchemaCache*& cache) {
        HostCallbacks* cb = relay_get_callbacks();
        if (cache) {
            cb->xml_destroy_schema_cache(static_cast<void*>(cache));
            cache = NULL;
        }
    }

    FXmlSchemaCache* CreateFXmlSchemaCache() {
        HostCallbacks* cb = relay_get_callbacks();
        return static_cast<FXmlSchemaCache*>(cb->xml_create_schema_cache());
    }

    // --- Load / Validate ---

    bool LoadXml(FXml* xml, const TCHAR* pszXmlFile) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_load(static_cast<void*>(xml), pszXmlFile) != 0;
    }

    bool Validate(FXml* xml, TCHAR* pszError) {
        return true;  // No validation for MVP
    }

    // --- Navigation ---

    bool LocateNode(FXml* xml, const TCHAR* pszXmlNode) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_locate_node(static_cast<void*>(xml), pszXmlNode) != 0;
    }

    bool LocateFirstSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_locate_first_sibling_by_tag(static_cast<void*>(xml), pszTagName) != 0;
    }

    bool LocateNextSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_locate_next_sibling_by_tag(static_cast<void*>(xml), pszTagName) != 0;
    }

    bool NextSibling(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_next_sibling(static_cast<void*>(xml)) != 0;
    }

    bool PrevSibling(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_prev_sibling(static_cast<void*>(xml)) != 0;
    }

    bool SetToChild(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_set_to_child(static_cast<void*>(xml)) != 0;
    }

    bool SetToChildByTagName(FXml* xml, const TCHAR* szTagName) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_set_to_child_by_tag(static_cast<void*>(xml), szTagName) != 0;
    }

    bool SetToParent(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_set_to_parent(static_cast<void*>(xml)) != 0;
    }

    // --- Mutation stubs (not needed for MVP) ---

    bool AddChildNode(FXml* xml, TCHAR* pszNewNode) {
        return false;
    }
    bool AddSiblingNodeBefore(FXml* xml, TCHAR* pszNewNode) {
        return false;
    }
    bool AddSiblingNodeAfter(FXml* xml, TCHAR* pszNewNode) {
        return false;
    }
    bool WriteXml(FXml* xml, TCHAR* pszXmlFile) {
        return false;
    }
    bool SetInsertedNodeAttribute(FXml* xml, TCHAR* a, TCHAR* b) {
        return false;
    }

    // --- Value reading: THE CORE STL TRANSLATION ---

    int GetLastNodeTextSize(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_get_last_node_text_size(static_cast<void*>(xml));
    }

    bool GetLastNodeText(FXml* xml, TCHAR* pszText) {
        HostCallbacks* cb = relay_get_callbacks();
        char buf[1024];
        if (!cb->xml_get_last_node_text(static_cast<void*>(xml), buf, sizeof(buf)))
            return false;
        strcpy(pszText, buf);
        return true;
    }

    // *** std::string — this is the key STL isolation method ***
    bool GetLastNodeValue(FXml* xml, std::string& text) {
        HostCallbacks* cb = relay_get_callbacks();
        char buf[1024];
        if (!cb->xml_get_value_string(static_cast<void*>(xml), buf, sizeof(buf)))
            return false;
        text = buf;  // VS2003 native std::string assignment — SAFE!
        return true;
    }

    // *** std::wstring — same pattern ***
    bool GetLastNodeValue(FXml* xml, std::wstring& text) {
        HostCallbacks* cb = relay_get_callbacks();
        wchar_t wbuf[1024];
        if (!cb->xml_get_value_wstring(static_cast<void*>(xml), wbuf, sizeof(wbuf)/sizeof(wchar_t)))
            return false;
        text = wbuf;  // VS2003 native std::wstring assignment — SAFE!
        return true;
    }

    bool GetLastNodeValue(FXml* xml, char* pszText) {
        HostCallbacks* cb = relay_get_callbacks();
        // Use our own buffer — we don't know the caller's buffer size
        char buf[1024];
        if (!cb->xml_get_value_string(static_cast<void*>(xml), buf, sizeof(buf)))
            return false;
        strcpy(pszText, buf);  // Copy to caller's buffer
        return true;
    }

    bool GetLastNodeValue(FXml* xml, wchar* pszText) {
        HostCallbacks* cb = relay_get_callbacks();
        // Use our own buffer — we don't know the caller's buffer size
        wchar_t wbuf[1024];
        if (!cb->xml_get_value_wstring(static_cast<void*>(xml), wbuf, sizeof(wbuf)/sizeof(wchar_t)))
            return false;
        wcscpy(pszText, wbuf);  // Copy to caller's buffer
        return true;
    }

    bool GetLastNodeValue(FXml* xml, bool* pbVal) {
        HostCallbacks* cb = relay_get_callbacks();
        int val = 0;
        if (!cb->xml_get_value_bool(static_cast<void*>(xml), &val))
            return false;
        *pbVal = (val != 0);
        return true;
    }

    bool GetLastNodeValue(FXml* xml, int* piVal) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_get_value_int(static_cast<void*>(xml), piVal) != 0;
    }

    bool GetLastNodeValue(FXml* xml, float* pfVal) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_get_value_float(static_cast<void*>(xml), pfVal) != 0;
    }

    bool GetLastNodeValue(FXml* xml, unsigned int* puiVal) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_get_value_uint(static_cast<void*>(xml), puiVal) != 0;
    }

    // --- Inserted node stubs ---

    int GetInsertedNodeTextSize(FXml* xml) { return 0; }
    bool GetInsertedNodeText(FXml* xml, TCHAR* pszText) { return false; }
    bool SetInsertedNodeText(FXml* xml, TCHAR* pszText) { return false; }
    bool GetLastLocatedNodeType(FXml* xml, TCHAR* pszType) { return false; }
    bool GetLastInsertedNodeType(FXml* xml, TCHAR* pszType) { return false; }

    // --- Query methods ---

    bool IsLastLocatedNodeCommentNode(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_is_comment_node(static_cast<void*>(xml)) != 0;
    }

    int NumOfElementsByTagName(FXml* xml, TCHAR* pszTagName) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_num_elements_by_tag(static_cast<void*>(xml), pszTagName);
    }

    int NumOfChildrenByTagName(FXml* xml, const TCHAR* pszTagName) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_num_children_by_tag(static_cast<void*>(xml), pszTagName);
    }

    int GetNumSiblings(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_get_num_siblings(static_cast<void*>(xml));
    }

    int GetNumChildren(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_get_num_children(static_cast<void*>(xml));
    }

    bool GetLastLocatedNodeTagName(FXml* xml, TCHAR* pszTagName) {
        HostCallbacks* cb = relay_get_callbacks();
        // Use our own buffer — we don't know the caller's buffer size
        char buf[1024];
        if (!cb->xml_get_tag_name(static_cast<void*>(xml), buf, sizeof(buf)))
            return false;
        strcpy(pszTagName, buf);
        return true;
    }

    bool IsAllowXMLCaching() {
        HostCallbacks* cb = relay_get_callbacks();
        return cb->xml_is_allow_caching() != 0;
    }

    void MapChildren(FXml* xml) {
        HostCallbacks* cb = relay_get_callbacks();
        cb->xml_map_children(static_cast<void*>(xml));
    }
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLXmlIFaceImpl g_relayXmlIFaceInstance;
CvDLLXmlIFaceBase* g_pRelayXmlIFace = &g_relayXmlIFaceInstance;
