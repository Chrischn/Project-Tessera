// =============================================================================
// File:              iface_xml.cpp
// Author(s):         Chrischn89
// Description:
//   CvDLLXmlIFaceBase implementation using pugixml. Provides XML loading,
//   DOM navigation, and value reading for CvGameCoreDLL's XML loading pipeline.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

// Include windows.h BEFORE iface_xml.h to avoid POINT redefinition
// (iface_types.h has a conditional POINT struct that conflicts with windef.h).
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "iface_xml.h"

#include <pugixml.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// =============================================================================
// FXml / FXmlSchemaCache definitions
// =============================================================================

struct FXml {
	pugi::xml_document doc;
	pugi::xml_node current_node;   // The "cursor" — where navigation methods operate
	bool loaded = false;
};

struct FXmlSchemaCache {
	// Placeholder — schema validation not needed for MVP
	int dummy = 0;
};

// =============================================================================
// Globals from message_protocol.cpp
// =============================================================================

extern std::string g_basePath;  // e.g., "E:/Programming/Civ4"
extern std::string g_modName;   // e.g., "" for vanilla

// =============================================================================
// Helper: Normalize a DLL-provided path
//   - Replace backslashes with forward slashes
//   - Collapse multiple consecutive slashes into one
// =============================================================================

static std::string normalizePath(const char* raw) {
	if (!raw) return {};

	std::string path(raw);

	// Replace all backslashes with forward slashes
	std::replace(path.begin(), path.end(), '\\', '/');

	// Collapse consecutive slashes (but not the double slash after ":" in drive letters)
	std::string result;
	result.reserve(path.size());
	bool prevSlash = false;
	for (char c : path) {
		if (c == '/') {
			if (!prevSlash) {
				result += c;
			}
			prevSlash = true;
		} else {
			result += c;
			prevSlash = false;
		}
	}

	return result;
}

// =============================================================================
// Helper: Resolve a DLL-relative path to an absolute filesystem path
//
// The DLL passes paths like "xml/GameInfo/CIV4GameSpeedInfo.xml" or
// "Assets//xml\GlobalDefines.xml". We normalize and search in priority order:
//   1. <basePath>/Beyond the Sword/Mods/<mod>/Assets/ + relative  (if mod active)
//   2. <basePath>/Beyond the Sword/Assets/ + relative  (NOT "Beyond the Sword" twice)
//   3. <basePath>/Beyond the Sword/ + relative  (for paths that start with "Assets/")
//   4. <basePath>/ + relative  (base game fallback)
// =============================================================================

static bool fileExists(const std::string& path) {
	DWORD attrs = GetFileAttributesA(path.c_str());
	return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static std::string resolveXmlPath(const char* rawPath) {
	std::string relPath = normalizePath(rawPath);

	// Strip leading slash if present
	if (!relPath.empty() && relPath[0] == '/') {
		relPath = relPath.substr(1);
	}

	// Normalize basePath
	std::string base = normalizePath(g_basePath.c_str());
	if (!base.empty() && base.back() == '/') {
		base.pop_back();
	}

	// Build candidate paths in priority order
	std::vector<std::string> candidates;

	// 1. Mod override (if mod is active)
	if (!g_modName.empty()) {
		candidates.push_back(base + "/Beyond the Sword/Mods/" + g_modName + "/" + relPath);
	}

	// 2. BTS directory + relative path directly
	candidates.push_back(base + "/Beyond the Sword/" + relPath);

	// 3. Base game root + relative path
	candidates.push_back(base + "/" + relPath);

	for (const auto& candidate : candidates) {
		if (fileExists(candidate)) {
			return candidate;
		}
	}

	// Not found — return the most likely path for error reporting
	if (!candidates.empty()) {
		return candidates[0];
	}
	return relPath;
}

// =============================================================================
// Helper: Skip to the first non-comment/non-declaration element node
// =============================================================================

static pugi::xml_node skipNonElements(pugi::xml_node node) {
	while (node && node.type() != pugi::node_element) {
		node = node.next_sibling();
	}
	return node;
}

static pugi::xml_node skipNonElementsPrev(pugi::xml_node node) {
	while (node && node.type() != pugi::node_element) {
		node = node.previous_sibling();
	}
	return node;
}

// =============================================================================
// Helper: Get text content of a node (handles both child_value and text())
// =============================================================================

static const char* getNodeText(const pugi::xml_node& node) {
	// child_value() returns the text of the first PCDATA/CDATA child
	const char* val = node.child_value();
	if (val && val[0] != '\0') return val;

	// Fallback: try text() accessor (works with <tag>text</tag>)
	val = node.text().get();
	if (val && val[0] != '\0') return val;

	return "";
}

// =============================================================================
// Implementation class
// =============================================================================

class CvDLLXmlIFaceImpl : public CvDLLXmlIFaceBase
{
public:
	// =========================================================================
	// Creation / Destruction
	// =========================================================================

	FXml* CreateFXml(FXmlSchemaCache* pSchemaCache) override {
		fprintf(stderr, "[XML] CreateFXml\n");
		return new FXml();
	}

	void DestroyFXml(FXml*& xml) override {
		fprintf(stderr, "[XML] DestroyFXml\n");
		if (xml) {
			delete xml;
			xml = nullptr;
		}
	}

	void DestroyFXmlSchemaCache(FXmlSchemaCache*& pCache) override {
		fprintf(stderr, "[XML] DestroyFXmlSchemaCache\n");
		if (pCache) {
			delete pCache;
			pCache = nullptr;
		}
	}

	FXmlSchemaCache* CreateFXmlSchemaCache() override {
		fprintf(stderr, "[XML] CreateFXmlSchemaCache\n");
		return new FXmlSchemaCache();
	}

	// =========================================================================
	// Load / Validate / Write
	// =========================================================================

	bool LoadXml(FXml* xml, const TCHAR* pszXmlFile) override {
		if (!xml || !pszXmlFile) {
			fprintf(stderr, "[XML] LoadXml: null argument\n");
			return false;
		}

		std::string absPath = resolveXmlPath(pszXmlFile);
		fprintf(stderr, "[XML] LoadXml: %s -> %s\n", pszXmlFile, absPath.c_str());

		pugi::xml_parse_result result = xml->doc.load_file(absPath.c_str());
		if (!result) {
			fprintf(stderr, "[XML] LoadXml FAILED: %s (offset %td)\n",
				result.description(), result.offset);
			xml->loaded = false;
			return false;
		}

		xml->current_node = xml->doc.document_element();
		xml->loaded = true;

		fprintf(stderr, "[XML] LoadXml OK: root=<%s>\n", xml->current_node.name());
		return true;
	}

	bool Validate(FXml* xml, TCHAR* pszError) override {
		// Skip validation — not needed for MVP
		return true;
	}

	bool WriteXml(FXml* xml, TCHAR* pszXmlFile) override {
		fprintf(stderr, "[XML STUB] WriteXml: %s\n", pszXmlFile ? pszXmlFile : "(null)");
		return false;
	}

	// =========================================================================
	// Navigation
	// =========================================================================

	bool LocateNode(FXml* xml, const TCHAR* pszXmlNode) override {
		if (!xml || !pszXmlNode || !xml->loaded) {
			fprintf(stderr, "[XML] LocateNode: null/unloaded\n");
			return false;
		}

		fprintf(stderr, "[XML] LocateNode: %s\n", pszXmlNode);

		// Split path on '/' and navigate step by step from document root
		std::string path(pszXmlNode);
		pugi::xml_node node = xml->doc.document_element();

		// Check if the first segment matches the document element name
		size_t pos = 0;
		size_t slash = path.find('/');
		std::string segment = (slash != std::string::npos)
			? path.substr(0, slash)
			: path;

		if (segment == node.name()) {
			// First segment is the root — start navigating from root's children
			pos = (slash != std::string::npos) ? slash + 1 : std::string::npos;
		} else {
			// First segment is not the root — try to find it as a child of root
			pugi::xml_node child = node.child(segment.c_str());
			if (!child) {
				fprintf(stderr, "[XML] LocateNode FAILED at segment: %s\n", segment.c_str());
				return false;
			}
			node = child;
			pos = (slash != std::string::npos) ? slash + 1 : std::string::npos;
		}

		// Navigate remaining segments
		while (pos != std::string::npos && pos < path.size()) {
			slash = path.find('/', pos);
			segment = (slash != std::string::npos)
				? path.substr(pos, slash - pos)
				: path.substr(pos);

			if (segment.empty()) {
				pos = (slash != std::string::npos) ? slash + 1 : std::string::npos;
				continue;
			}

			pugi::xml_node child = node.child(segment.c_str());
			if (!child) {
				fprintf(stderr, "[XML] LocateNode FAILED at segment: %s\n", segment.c_str());
				return false;
			}
			node = child;
			pos = (slash != std::string::npos) ? slash + 1 : std::string::npos;
		}

		xml->current_node = node;
		fprintf(stderr, "[XML] LocateNode OK: <%s>\n", node.name());
		return true;
	}

	bool LocateFirstSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) override {
		if (!xml || !pszTagName || !xml->loaded) return false;

		fprintf(stderr, "[XML] LocateFirstSiblingNodeByTagName: %s\n", pszTagName);

		// Go to parent, then find first child with matching tag
		pugi::xml_node parent = xml->current_node.parent();
		if (!parent) return false;

		pugi::xml_node found = parent.child(pszTagName);
		if (!found) return false;

		xml->current_node = found;
		return true;
	}

	bool LocateNextSiblingNodeByTagName(FXml* xml, TCHAR* pszTagName) override {
		if (!xml || !pszTagName || !xml->loaded) return false;

		fprintf(stderr, "[XML] LocateNextSiblingNodeByTagName: %s\n", pszTagName);

		// From current node, find next sibling with matching tag
		pugi::xml_node found = xml->current_node.next_sibling(pszTagName);
		if (!found) return false;

		xml->current_node = found;
		return true;
	}

	bool NextSibling(FXml* xml) override {
		if (!xml || !xml->loaded) return false;

		pugi::xml_node next = xml->current_node.next_sibling();
		// Skip non-element nodes (comments, processing instructions, etc.)
		next = skipNonElements(next);

		if (!next) return false;

		xml->current_node = next;
		return true;
	}

	bool PrevSibling(FXml* xml) override {
		if (!xml || !xml->loaded) return false;

		pugi::xml_node prev = xml->current_node.previous_sibling();
		prev = skipNonElementsPrev(prev);

		if (!prev) return false;

		xml->current_node = prev;
		return true;
	}

	bool SetToChild(FXml* xml) override {
		if (!xml || !xml->loaded) return false;

		pugi::xml_node child = xml->current_node.first_child();
		// Skip non-element nodes (text nodes, comments, etc.)
		child = skipNonElements(child);

		if (!child) return false;

		xml->current_node = child;
		return true;
	}

	bool SetToChildByTagName(FXml* xml, const TCHAR* szTagName) override {
		if (!xml || !szTagName || !xml->loaded) return false;

		pugi::xml_node child = xml->current_node.child(szTagName);
		if (!child) return false;

		xml->current_node = child;
		return true;
	}

	bool SetToParent(FXml* xml) override {
		if (!xml || !xml->loaded) return false;

		pugi::xml_node parent = xml->current_node.parent();
		if (!parent || parent.type() == pugi::node_document) {
			// Already at document root — going higher would leave DOM
			// Still set to document element as safe fallback
			return false;
		}

		xml->current_node = parent;
		return true;
	}

	// =========================================================================
	// Insertion (stubs — not needed for MVP)
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
		if (!xml || !xml->loaded) return 0;
		const char* text = getNodeText(xml->current_node);
		return static_cast<int>(strlen(text));
	}

	bool GetLastNodeText(FXml* xml, TCHAR* pszText) override {
		if (!xml || !xml->loaded || !pszText) return false;
		const char* text = getNodeText(xml->current_node);
		strcpy(pszText, text);
		return true;
	}

	bool GetLastNodeValue(FXml* xml, std::string& pszText) override {
		if (!xml || !xml->loaded) return false;
		const char* text = getNodeText(xml->current_node);
		pszText = text;
		return true;
	}

	bool GetLastNodeValue(FXml* xml, std::wstring& pszText) override {
		if (!xml || !xml->loaded) return false;
		const char* text = getNodeText(xml->current_node);

		// Convert UTF-8 to wide string using Windows API
		int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
		if (len <= 0) {
			pszText.clear();
			return true;
		}
		std::vector<wchar_t> buf(len);
		MultiByteToWideChar(CP_UTF8, 0, text, -1, buf.data(), len);
		pszText = buf.data();
		return true;
	}

	bool GetLastNodeValue(FXml* xml, char* pszText) override {
		if (!xml || !xml->loaded || !pszText) return false;
		const char* text = getNodeText(xml->current_node);
		strcpy(pszText, text);
		return true;
	}

	bool GetLastNodeValue(FXml* xml, wchar* pszText) override {
		if (!xml || !xml->loaded || !pszText) return false;
		const char* text = getNodeText(xml->current_node);

		int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
		if (len <= 0) {
			pszText[0] = L'\0';
			return true;
		}
		MultiByteToWideChar(CP_UTF8, 0, text, -1, pszText, len);
		return true;
	}

	bool GetLastNodeValue(FXml* xml, bool* pbVal) override {
		if (!xml || !xml->loaded || !pbVal) return false;
		const char* text = getNodeText(xml->current_node);
		if (!text || text[0] == '\0') return false;

		// Handle "1"/"0", "true"/"false"
		if (strcmp(text, "1") == 0 || _stricmp(text, "true") == 0) {
			*pbVal = true;
		} else if (strcmp(text, "0") == 0 || _stricmp(text, "false") == 0) {
			*pbVal = false;
		} else {
			*pbVal = xml->current_node.text().as_bool();
		}
		return true;
	}

	bool GetLastNodeValue(FXml* xml, int* piVal) override {
		if (!xml || !xml->loaded || !piVal) return false;
		const char* text = getNodeText(xml->current_node);
		if (!text || text[0] == '\0') return false;
		*piVal = xml->current_node.text().as_int();
		return true;
	}

	bool GetLastNodeValue(FXml* xml, float* pfVal) override {
		if (!xml || !xml->loaded || !pfVal) return false;
		const char* text = getNodeText(xml->current_node);
		if (!text || text[0] == '\0') return false;
		*pfVal = xml->current_node.text().as_float();
		return true;
	}

	bool GetLastNodeValue(FXml* xml, unsigned int* puiVal) override {
		if (!xml || !xml->loaded || !puiVal) return false;
		const char* text = getNodeText(xml->current_node);
		if (!text || text[0] == '\0') return false;
		*puiVal = xml->current_node.text().as_uint();
		return true;
	}

	// =========================================================================
	// Inserted Node Text (stubs — write support not needed for MVP)
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
		if (!xml || !xml->loaded || !pszType) return false;

		switch (xml->current_node.type()) {
		case pugi::node_element:    strcpy(pszType, "element"); break;
		case pugi::node_pcdata:     strcpy(pszType, "text"); break;
		case pugi::node_comment:    strcpy(pszType, "comment"); break;
		case pugi::node_cdata:      strcpy(pszType, "cdata"); break;
		case pugi::node_pi:         strcpy(pszType, "processing_instruction"); break;
		default:                    strcpy(pszType, "unknown"); break;
		}
		return true;
	}

	bool GetLastInsertedNodeType(FXml* xml, TCHAR* pszType) override {
		fprintf(stderr, "[XML STUB] GetLastInsertedNodeType\n");
		return false;
	}

	bool IsLastLocatedNodeCommentNode(FXml* xml) override {
		if (!xml || !xml->loaded) return false;
		return xml->current_node.type() == pugi::node_comment;
	}

	// =========================================================================
	// Counting / Enumeration
	// =========================================================================

	int NumOfElementsByTagName(FXml* xml, TCHAR* pszTagName) override {
		if (!xml || !xml->loaded || !pszTagName) return 0;

		// Count siblings (including current) matching the tag name.
		// Start from parent's first child to count all siblings.
		pugi::xml_node parent = xml->current_node.parent();
		if (!parent) return 0;

		int count = 0;
		for (pugi::xml_node child = parent.child(pszTagName); child;
			 child = child.next_sibling(pszTagName)) {
			count++;
		}
		return count;
	}

	int NumOfChildrenByTagName(FXml* xml, const TCHAR* pszTagName) override {
		if (!xml || !xml->loaded || !pszTagName) return 0;

		int count = 0;
		for (pugi::xml_node child = xml->current_node.child(pszTagName); child;
			 child = child.next_sibling(pszTagName)) {
			count++;
		}
		return count;
	}

	int GetNumSiblings(FXml* xml) override {
		if (!xml || !xml->loaded) return 0;

		pugi::xml_node parent = xml->current_node.parent();
		if (!parent) return 1;  // Only current node

		int count = 0;
		for (pugi::xml_node sibling = parent.first_child(); sibling;
			 sibling = sibling.next_sibling()) {
			if (sibling.type() == pugi::node_element) {
				count++;
			}
		}
		return count;
	}

	int GetNumChildren(FXml* xml) override {
		if (!xml || !xml->loaded) return 0;

		int count = 0;
		for (pugi::xml_node child = xml->current_node.first_child(); child;
			 child = child.next_sibling()) {
			if (child.type() == pugi::node_element) {
				count++;
			}
		}
		return count;
	}

	// =========================================================================
	// Misc
	// =========================================================================

	bool GetLastLocatedNodeTagName(FXml* xml, TCHAR* pszTagName) override {
		if (!xml || !xml->loaded || !pszTagName) return false;
		strcpy(pszTagName, xml->current_node.name());
		return true;
	}

	bool IsAllowXMLCaching() override {
		return true;
	}

	void MapChildren(FXml* xml) override {
		// No-op — optimization hint, not needed for correctness
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLXmlIFaceImpl g_xmlIFaceInstance;
CvDLLXmlIFaceBase* g_pXmlIFace = &g_xmlIFaceInstance;
