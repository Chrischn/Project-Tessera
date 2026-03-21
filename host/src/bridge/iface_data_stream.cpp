// =============================================================================
// File:              iface_data_stream.cpp
// Author(s):         Chrischn89
// Description:
//   Stub implementation of FDataStreamBase. All methods log (selectively) and
//   return safe defaults. Real implementations replace stubs incrementally.
//
//   Read/Write methods skip logging to avoid high-frequency spam.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================

#include "iface_data_stream.h"
#include <cstdio>
#include <cstring>

class FDataStreamImpl : public FDataStreamBase
{
public:
	// =========================================================================
	// Stream positioning
	// =========================================================================

	void Rewind() override {
		fprintf(stderr, "[STREAM STUB] Rewind\n");
	}

	bool AtEnd() override {
		fprintf(stderr, "[STREAM STUB] AtEnd\n");
		return true;
	}

	void FastFwd() override {
		fprintf(stderr, "[STREAM STUB] FastFwd\n");
	}

	unsigned int GetPosition() const override {
		return 0;
	}

	void SetPosition(unsigned int position) override {
		fprintf(stderr, "[STREAM STUB] SetPosition\n");
	}

	void Truncate() override {
		fprintf(stderr, "[STREAM STUB] Truncate\n");
	}

	void Flush() override {
		fprintf(stderr, "[STREAM STUB] Flush\n");
	}

	unsigned int GetEOF() const override {
		return 0;
	}

	unsigned int GetSizeLeft() const override {
		return 0;
	}

	void CopyToMem(void* mem) override {
		fprintf(stderr, "[STREAM STUB] CopyToMem\n");
	}

	// =========================================================================
	// WriteString overloads
	// =========================================================================

	unsigned int WriteString(const wchar* szName) override {
		return 0;
	}

	unsigned int WriteString(const char* szName) override {
		return 0;
	}

	unsigned int WriteString(const std::string& szName) override {
		return 0;
	}

	unsigned int WriteString(const std::wstring& szName) override {
		return 0;
	}

	unsigned int WriteString(int count, std::string values[]) override {
		return 0;
	}

	unsigned int WriteString(int count, std::wstring values[]) override {
		return 0;
	}

	// =========================================================================
	// ReadString overloads
	// =========================================================================

	unsigned int ReadString(char* szName) override {
		if (szName) szName[0] = '\0';
		return 0;
	}

	unsigned int ReadString(wchar* szName) override {
		if (szName) szName[0] = L'\0';
		return 0;
	}

	unsigned int ReadString(std::string& szName) override {
		szName.clear();
		return 0;
	}

	unsigned int ReadString(std::wstring& szName) override {
		szName.clear();
		return 0;
	}

	unsigned int ReadString(int count, std::string values[]) override {
		return 0;
	}

	unsigned int ReadString(int count, std::wstring values[]) override {
		return 0;
	}

	char* ReadString() override {
		return nullptr;
	}

	wchar* ReadWideString() override {
		return nullptr;
	}

	// =========================================================================
	// Read overloads (high-frequency — no logging)
	// =========================================================================

	void Read(char* v) override {
		if (v) *v = 0;
	}

	void Read(byte* v) override {
		if (v) *v = 0;
	}

	void Read(int count, char values[]) override {
		if (values) memset(values, 0, count * sizeof(char));
	}

	void Read(int count, byte values[]) override {
		if (values) memset(values, 0, count * sizeof(byte));
	}

	void Read(bool* v) override {
		if (v) *v = false;
	}

	void Read(int count, bool values[]) override {
		if (values) memset(values, 0, count * sizeof(bool));
	}

	void Read(short* s) override {
		if (s) *s = 0;
	}

	void Read(unsigned short* s) override {
		if (s) *s = 0;
	}

	void Read(int count, short values[]) override {
		if (values) memset(values, 0, count * sizeof(short));
	}

	void Read(int count, unsigned short values[]) override {
		if (values) memset(values, 0, count * sizeof(unsigned short));
	}

	void Read(int* i) override {
		if (i) *i = 0;
	}

	void Read(unsigned int* i) override {
		if (i) *i = 0;
	}

	void Read(int count, int values[]) override {
		if (values) memset(values, 0, count * sizeof(int));
	}

	void Read(int count, unsigned int values[]) override {
		if (values) memset(values, 0, count * sizeof(unsigned int));
	}

	void Read(long* l) override {
		if (l) *l = 0;
	}

	void Read(unsigned long* l) override {
		if (l) *l = 0;
	}

	void Read(int count, long values[]) override {
		if (values) memset(values, 0, count * sizeof(long));
	}

	void Read(int count, unsigned long values[]) override {
		if (values) memset(values, 0, count * sizeof(unsigned long));
	}

	void Read(float* value) override {
		if (value) *value = 0.0f;
	}

	void Read(int count, float values[]) override {
		if (values) memset(values, 0, count * sizeof(float));
	}

	void Read(double* value) override {
		if (value) *value = 0.0;
	}

	void Read(int count, double values[]) override {
		if (values) memset(values, 0, count * sizeof(double));
	}

	// =========================================================================
	// Write overloads (high-frequency — no logging)
	// =========================================================================

	void Write(char value) override {}
	void Write(byte value) override {}
	void Write(int count, const char values[]) override {}
	void Write(int count, const byte values[]) override {}

	void Write(bool value) override {}
	void Write(int count, const bool values[]) override {}

	void Write(short value) override {}
	void Write(unsigned short value) override {}
	void Write(int count, const short values[]) override {}
	void Write(int count, const unsigned short values[]) override {}

	void Write(int value) override {}
	void Write(unsigned int value) override {}
	void Write(int count, const int values[]) override {}
	void Write(int count, const unsigned int values[]) override {}

	void Write(long value) override {}
	void Write(unsigned long value) override {}
	void Write(int count, const long values[]) override {}
	void Write(int count, const unsigned long values[]) override {}

	void Write(float value) override {}
	void Write(int count, const float values[]) override {}

	void Write(double value) override {}
	void Write(int count, const double values[]) override {}
};

// =============================================================================
// Global singleton
// =============================================================================
static FDataStreamImpl g_dataStreamInstance;
FDataStreamBase* g_pDataStream = &g_dataStreamInstance;
