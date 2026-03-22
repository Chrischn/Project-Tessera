// =============================================================================
// File:              iface_ini_parser.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLIniParserIFaceBase definition. Method order matches SDK
//   vtable layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLIniParserIFaceBase
{
public:
	virtual FIniParser* create(const char* szFile) = 0;
	virtual void destroy(FIniParser*& pParser, bool bSafeDelete = true) = 0;
	virtual bool SetGroupKey(FIniParser* pParser, const LPCTSTR pGroupKey) = 0;
	virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, bool* iValue) = 0;
	virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, short* iValue) = 0;
	virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, int* iValue) = 0;
	virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, float* fValue) = 0;
	virtual bool GetKeyValue(FIniParser* pParser, const LPCTSTR szKey, LPTSTR szValue) = 0;
};
