// =============================================================================
// File:              iface_python.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of CvDLLPythonIFaceBase. All methods log and return
//   safe defaults. Real implementations replace stubs incrementally.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_python.h"
#include <cstdio>

class CvDLLPythonIFaceImpl : public CvDLLPythonIFaceBase
{
public:
	bool isInitialized() override {
		fprintf(stderr, "[PYTHON STUB] isInitialized\n");
		return false;
	}

	const char* getMapScriptModule() override {
		fprintf(stderr, "[PYTHON STUB] getMapScriptModule\n");
		return "";
	}

	PyObject* MakeFunctionArgs(void** args, int argc) override {
		fprintf(stderr, "[PYTHON STUB] MakeFunctionArgs\n");
		return nullptr;
	}

	bool moduleExists(const char* moduleName, bool bLoadIfNecessary) override {
		fprintf(stderr, "[PYTHON STUB] moduleExists: %s\n", moduleName ? moduleName : "(null)");
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(void): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, long* result) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(long): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		if (result) *result = 0;
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, CvString* result) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(CvString): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		if (result) result->clear();
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, CvWString* result) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(CvWString): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		if (result) result->clear();
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<byte>* pList) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(byte vec): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<int>* pIntList) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(int vec): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, int* pIntList, int* iListSize) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(int*): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		if (iListSize) *iListSize = 0;
		return false;
	}

	bool callFunction(const char* moduleName, const char* fxnName, void* fxnArg, std::vector<float>* pFloatList) override {
		fprintf(stderr, "[PYTHON STUB] callFunction(float vec): %s.%s\n",
			moduleName ? moduleName : "(null)", fxnName ? fxnName : "(null)");
		return false;
	}

	bool callPythonFunction(const char* szModName, const char* szFxnName, int iArg, long* result) override {
		fprintf(stderr, "[PYTHON STUB] callPythonFunction: %s.%s\n",
			szModName ? szModName : "(null)", szFxnName ? szFxnName : "(null)");
		if (result) *result = 0;
		return false;
	}

	bool pythonUsingDefaultImpl() override {
		fprintf(stderr, "[PYTHON STUB] pythonUsingDefaultImpl\n");
		return true;
	}
};

// =============================================================================
// Global singleton
// =============================================================================
static CvDLLPythonIFaceImpl g_pythonIFaceInstance;
CvDLLPythonIFaceBase* g_pPythonIFace = &g_pythonIFaceInstance;
