// =============================================================================
// File:              iface_ini_parser.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLIniParserIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_ini_parser.h"
#include <cstdio>

class CvDLLIniParserIFaceImpl : public CvDLLIniParserIFaceBase
{
public:
	FIniParser* create(const char* szFile) override {
		fprintf(stderr, "[INI STUB] create: %s\n", szFile ? szFile : "(null)");
		return nullptr;
	}

	void destroy(FIniParser*& pParser, bool bSafeDelete) override {
		fprintf(stderr, "[INI STUB] destroy\n");
		pParser = nullptr;
	}

	bool SetGroupKey(FIniParser* pParser, const LPCTSTR pGroupKey) override {
		fprintf(stderr, "[INI STUB] SetGroupKey: %s\n", pGroupKey ? pGroupKey : "(null)");
		return false;
	}

	bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, bool* iValue) override {
		fprintf(stderr, "[INI STUB] GetKeyValue(bool): %s\n", szKey ? szKey : "(null)");
		if (iValue) *iValue = false;
		return false;
	}

	bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, short* iValue) override {
		fprintf(stderr, "[INI STUB] GetKeyValue(short): %s\n", szKey ? szKey : "(null)");
		if (iValue) *iValue = 0;
		return false;
	}

	bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, int* iValue) override {
		fprintf(stderr, "[INI STUB] GetKeyValue(int): %s\n", szKey ? szKey : "(null)");
		if (iValue) *iValue = 0;
		return false;
	}

	bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, float* fValue) override {
		fprintf(stderr, "[INI STUB] GetKeyValue(float): %s\n", szKey ? szKey : "(null)");
		if (fValue) *fValue = 0.0f;
		return false;
	}

	bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, LPTSTR szValue) override {
		fprintf(stderr, "[INI STUB] GetKeyValue(str): %s\n", szKey ? szKey : "(null)");
		if (szValue) szValue[0] = '\0';
		return false;
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLIniParserIFaceImpl g_iniParserIFaceInstance;
CvDLLIniParserIFaceBase* g_pIniParserIFace = &g_iniParserIFaceInstance;
