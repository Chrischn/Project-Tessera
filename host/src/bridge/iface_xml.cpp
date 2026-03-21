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
// VS2003 string/wstring assign — call the VS2003 CRT's own string::assign
// method from msvcp71.dll. This lets the VS2003 string handle its own memory
// management, completely avoiding cross-CRT allocator conflicts.
// =============================================================================

// std::string::assign(const char*) — __thiscall, returns string&
typedef void* (__thiscall *StringAssignFn)(void* thisPtr, const char* str);
// std::wstring::assign(const wchar_t*) — __thiscall, returns wstring&
typedef void* (__thiscall *WStringAssignFn)(void* thisPtr, const wchar_t* str);

static StringAssignFn g_vs2003_string_assign = nullptr;
static WStringAssignFn g_vs2003_wstring_assign = nullptr;
static bool g_vs2003_stl_initialized = false;

static void initVS2003STL() {
	if (g_vs2003_stl_initialized) return;
	g_vs2003_stl_initialized = true;

	// msvcp71.dll is the VS2003 C++ Standard Library (already loaded by CvGameCoreDLL.dll)
	HMODULE hStl = GetModuleHandleA("msvcp71.dll");
	if (!hStl) {
		fprintf(stderr, "[XML ERROR] msvcp71.dll not found!\n");
		return;
	}

	// std::string::assign(const char*)
	g_vs2003_string_assign = (StringAssignFn)GetProcAddress(hStl,
		"?assign@?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@QAEAAV12@PBD@Z");

	// std::wstring::assign(const wchar_t*)
	g_vs2003_wstring_assign = (WStringAssignFn)GetProcAddress(hStl,
		"?assign@?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@QAEAAV12@PBG@Z");

	fprintf(stderr, "[XML] VS2003 STL (msvcp71.dll): string::assign=%p wstring::assign=%p\n",
		g_vs2003_string_assign, g_vs2003_wstring_assign);
}

// =============================================================================
// VS2003 std::string raw memory writer
//
// MSVC 7.1 (VS2003) std::string layout on 32-bit:
//   _String_val base class contains _Alval (allocator, 4 bytes padded)
//   Then:
//     offset  0: _Alval  (4 bytes — padded empty allocator from base)
//     offset  4: _Bx     (16 bytes — union { char _Buf[16]; char* _Ptr; })
//     offset 20: _Mysize (4 bytes)
//     offset 24: _Myres  (4 bytes)
//     Total: 28 bytes
//
// SSO mode (_Myres <= 15): string data in _Bx._Buf (offset 4), null-terminated
// Heap mode (_Myres > 15): _Bx._Ptr (offset 4) points to heap buffer
//
// We CANNOT use operator= because our VS2022 allocator would try to
// free/realloc buffers allocated by VS2003's msvcr71.dll.
// =============================================================================

// These offsets are determined empirically. If the DLL was compiled with
// VS2003 using Dinkumware STL without allocator padding, the layout is:
//   offset 0:  _Bx (16 bytes)
//   offset 16: _Mysize (4 bytes)
//   offset 20: _Myres (4 bytes)
// If the allocator adds 4 bytes at the beginning, shift all by 4.
// The correct offsets are validated by checking that _Myres = 15 for a
// default-constructed string passed from the DLL.
// Raw byte dump confirmed: _Myres=15 at offset 24, _Mysize=0 at offset 20.
// This is the 28-byte VS2003 layout with allocator at offset 0.
static const size_t VS2003_BX_OFFSET = 4;
static const size_t VS2003_MYSIZE_OFFSET = 20;
static const size_t VS2003_MYRES_OFFSET = 24;

static void writeVS2003String(char* raw, const char* text) {
	size_t len = text ? strlen(text) : 0;
	char* bx = raw + VS2003_BX_OFFSET;
	size_t* pMysize = reinterpret_cast<size_t*>(raw + VS2003_MYSIZE_OFFSET);
	size_t* pMyres = reinterpret_cast<size_t*>(raw + VS2003_MYRES_OFFSET);
	size_t myres = *pMyres;

	if (len <= 15) {
		if (myres > 15) {
			// Currently heap mode — write to heap buffer
			char* ptr = *reinterpret_cast<char**>(bx);
			memcpy(ptr, text, len);
			ptr[len] = '\0';
		} else {
			// SSO mode — write to inline buffer at _Bx offset
			memcpy(bx, text, len);
			bx[len] = '\0';
		}
		*pMysize = len;
	} else if (len <= myres) {
		// Fits in existing heap buffer
		char* ptr = *reinterpret_cast<char**>(bx);
		memcpy(ptr, text, len);
		ptr[len] = '\0';
		*pMysize = len;
	} else {
		// Need bigger buffer than current _Myres allows.
		// Allocate via VS2003's CRT (msvcr71.dll::malloc) so the
		// VS2003 string destructor can safely free it.
		size_t newres = len + 16;
		char* newbuf = static_cast<char*>(malloc(newres + 1));
		if (!newbuf) {
			fprintf(stderr, "[XML ERROR] malloc failed for string of size %zu\n", len);
			return;
		}
		memcpy(newbuf, text, len);
		newbuf[len] = '\0';

		// If switching from SSO to heap, the old SSO buffer is just stack memory
		// (part of the string struct itself) — no need to free it.
		// If already in heap mode, the old _Ptr buffer was allocated by VS2003's
		// allocator — we must NOT free it (let the DLL handle it or leak it).
		// Just overwrite the pointer.
		fprintf(stderr, "[XML] HEAP ALLOC: switching SSO->heap, len=%zu newres=%zu buf=%p\n", len, newres, newbuf);
		*reinterpret_cast<char**>(bx) = newbuf;
		*pMysize = len;
		*pMyres = newres;
		fprintf(stderr, "[XML] HEAP ALLOC: pointer written to bx(+%zu), _Mysize=%zu, _Myres=%zu\n",
			VS2003_BX_OFFSET, *pMysize, *pMyres);
	}
}

static void writeVS2003WString(char* raw, const wchar_t* text) {
	size_t len = text ? wcslen(text) : 0;
	// VS2003 std::wstring layout (32-bit):
	//   offset  0: _Alval  (4 bytes — padded allocator from base)
	//   offset  4: _Bx     (16 bytes — union { wchar_t _Buf[8]; wchar_t* _Ptr; })
	//   offset 20: _Mysize (4 bytes)
	//   offset 24: _Myres  (4 bytes)
	// SSO threshold: 7 wchar_t characters (8 * 2 = 16 bytes, minus null)
	char* bx = raw + VS2003_BX_OFFSET;
	size_t* pMysize = reinterpret_cast<size_t*>(raw + VS2003_MYSIZE_OFFSET);
	size_t* pMyres = reinterpret_cast<size_t*>(raw + VS2003_MYRES_OFFSET);
	size_t myres = *pMyres;

	if (len <= 7) {
		if (myres > 7) {
			wchar_t* ptr = *reinterpret_cast<wchar_t**>(bx);
			memcpy(ptr, text, len * sizeof(wchar_t));
			ptr[len] = L'\0';
		} else {
			wchar_t* buf = reinterpret_cast<wchar_t*>(bx);
			memcpy(buf, text, len * sizeof(wchar_t));
			buf[len] = L'\0';
		}
		*pMysize = len;
	} else if (len <= myres) {
		wchar_t* ptr = *reinterpret_cast<wchar_t**>(bx);
		memcpy(ptr, text, len * sizeof(wchar_t));
		ptr[len] = L'\0';
		*pMysize = len;
	} else {
		size_t newres = len + 16;
		wchar_t* newbuf = static_cast<wchar_t*>(malloc((newres + 1) * sizeof(wchar_t)));
		if (!newbuf) {
			fprintf(stderr, "[XML ERROR] malloc failed for wstring of size %zu\n", len);
			return;
		}
		memcpy(newbuf, text, len * sizeof(wchar_t));
		newbuf[len] = L'\0';
		*reinterpret_cast<wchar_t**>(bx) = newbuf;
		*pMysize = len;
		*pMyres = newres;
	}
}

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
		next = skipNonElements(next);

		if (!next) {
			fprintf(stderr, "[XML] NextSibling: no more siblings after <%s>\n", xml->current_node.name());
			return false;
		}

		xml->current_node = next;
		fprintf(stderr, "[XML] NextSibling -> <%s>\n", next.name());
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
		child = skipNonElements(child);

		if (!child) {
			fprintf(stderr, "[XML] SetToChild: no element children in <%s>\n", xml->current_node.name());
			return false;
		}

		xml->current_node = child;
		fprintf(stderr, "[XML] SetToChild -> <%s>\n", child.name());
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
			fprintf(stderr, "[XML] SetToParent: already at root from <%s>\n", xml->current_node.name());
			return false;
		}
		fprintf(stderr, "[XML] SetToParent <%s> -> <%s>\n", xml->current_node.name(), parent.name());

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
		fprintf(stderr, "[XML] GetLastNodeValue(string) <%s> = \"%s\"\n", xml->current_node.name(), text);

		// Use VS2003's own string::assign method from msvcp71.dll.
		// This avoids all cross-CRT memory management issues.
		initVS2003STL();
		if (g_vs2003_string_assign) {
			g_vs2003_string_assign(&pszText, text);
		} else {
			// Fallback: raw write (may crash on heap-allocated strings)
			writeVS2003String(reinterpret_cast<char*>(&pszText), text);
		}
		return true;
	}

	bool GetLastNodeValue(FXml* xml, std::wstring& pszText) override {
		if (!xml || !xml->loaded) return false;
		const char* text = getNodeText(xml->current_node);

		// Convert UTF-8 to wide string
		int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
		if (wlen <= 0) {
			writeVS2003WString(reinterpret_cast<char*>(&pszText), L"");
			return true;
		}
		std::vector<wchar_t> buf(wlen);
		MultiByteToWideChar(CP_UTF8, 0, text, -1, buf.data(), wlen);

		// Use VS2003's own wstring::assign from msvcp71.dll
		initVS2003STL();
		if (g_vs2003_wstring_assign) {
			g_vs2003_wstring_assign(&pszText, buf.data());
		} else {
			writeVS2003WString(reinterpret_cast<char*>(&pszText), buf.data());
		}
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
		fprintf(stderr, "[XML] GetNumChildren(<%s>) = %d\n", xml->current_node.name(), count);
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
