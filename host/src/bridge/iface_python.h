// =============================================================================
// File:              iface_python.h
// Author(s):         Chrischn89
// Description:
//   Clean-room CvDLLPythonIFaceBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
//   NOTE: Template helper methods (setSeqFromArray, putSeqInArray, etc.) are
//   non-virtual and implemented inline in the SDK. They do NOT occupy vtable
//   slots. Only the virtual methods matter for ABI compatibility.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class CvDLLPythonIFaceBase
{
public:
	virtual bool isInitialized() = 0;

	virtual const char* getMapScriptModule() = 0;

	// Non-virtual template helpers omitted — they are inline in the SDK and
	// do NOT occupy vtable slots.

	virtual PyObject* MakeFunctionArgs(void** args, int argc) = 0;

	virtual bool moduleExists(const char* moduleName, bool bLoadIfNecessary) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg = NULL) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, long* result) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, CvString* result) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, CvWString* result) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<byte>* pList) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<int>* pIntList) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, int* pIntList, int* iListSize) = 0;
	virtual bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<float>* pFloatList) = 0;
	virtual bool callPythonFunction(const char* szModName, const char* szFxnName, int iArg, long* result) = 0;

	virtual bool pythonUsingDefaultImpl() = 0;
};
