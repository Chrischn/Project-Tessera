// =============================================================================
// File:              iface_data_stream.h
// Author(s):         Chrischn89
// Description:
//   Clean-room FDataStreamBase definition. Method order matches SDK vtable
//   layout for ABI compatibility with unmodified CvGameCoreDLL.dll.
//
// License:
//   Released under the terms of the GNU General Public License version 3.0
// =============================================================================
#pragma once

#include "iface_types.h"

class FDataStreamBase
{
public:
	virtual void Rewind() = 0;
	virtual bool AtEnd() = 0;
	virtual void FastFwd() = 0;
	virtual unsigned int GetPosition() const = 0;
	virtual void SetPosition(unsigned int position) = 0;
	virtual void Truncate() = 0;
	virtual void Flush() = 0;
	virtual unsigned int GetEOF() const = 0;
	virtual unsigned int GetSizeLeft() const = 0;
	virtual void CopyToMem(void* mem) = 0;

	virtual unsigned int WriteString(const wchar* szName) = 0;
	virtual unsigned int WriteString(const char* szName) = 0;
	virtual unsigned int WriteString(const std::string& szName) = 0;
	virtual unsigned int WriteString(const std::wstring& szName) = 0;
	virtual unsigned int WriteString(int count, std::string values[]) = 0;
	virtual unsigned int WriteString(int count, std::wstring values[]) = 0;

	virtual unsigned int ReadString(char* szName) = 0;
	virtual unsigned int ReadString(wchar* szName) = 0;
	virtual unsigned int ReadString(std::string& szName) = 0;
	virtual unsigned int ReadString(std::wstring& szName) = 0;
	virtual unsigned int ReadString(int count, std::string values[]) = 0;
	virtual unsigned int ReadString(int count, std::wstring values[]) = 0;

	virtual char* ReadString() = 0;          // allocates memory
	virtual wchar* ReadWideString() = 0;     // allocates memory

	virtual void Read(char*) = 0;
	virtual void Read(byte*) = 0;
	virtual void Read(int count, char values[]) = 0;
	virtual void Read(int count, byte values[]) = 0;
	virtual void Read(bool*) = 0;
	virtual void Read(int count, bool values[]) = 0;
	virtual void Read(short* s) = 0;
	virtual void Read(unsigned short* s) = 0;
	virtual void Read(int count, short values[]) = 0;
	virtual void Read(int count, unsigned short values[]) = 0;
	virtual void Read(int* i) = 0;
	virtual void Read(unsigned int* i) = 0;
	virtual void Read(int count, int values[]) = 0;
	virtual void Read(int count, unsigned int values[]) = 0;

	virtual void Read(long* l) = 0;
	virtual void Read(unsigned long* l) = 0;
	virtual void Read(int count, long values[]) = 0;
	virtual void Read(int count, unsigned long values[]) = 0;

	virtual void Read(float* value) = 0;
	virtual void Read(int count, float values[]) = 0;

	virtual void Read(double* value) = 0;
	virtual void Read(int count, double values[]) = 0;

	virtual void Write(char value) = 0;
	virtual void Write(byte value) = 0;
	virtual void Write(int count, const char values[]) = 0;
	virtual void Write(int count, const byte values[]) = 0;

	virtual void Write(bool value) = 0;
	virtual void Write(int count, const bool values[]) = 0;

	virtual void Write(short value) = 0;
	virtual void Write(unsigned short value) = 0;
	virtual void Write(int count, const short values[]) = 0;
	virtual void Write(int count, const unsigned short values[]) = 0;

	virtual void Write(int value) = 0;
	virtual void Write(unsigned int value) = 0;
	virtual void Write(int count, const int values[]) = 0;
	virtual void Write(int count, const unsigned int values[]) = 0;

	virtual void Write(long value) = 0;
	virtual void Write(unsigned long value) = 0;
	virtual void Write(int count, const long values[]) = 0;
	virtual void Write(int count, const unsigned long values[]) = 0;

	virtual void Write(float value) = 0;
	virtual void Write(int count, const float values[]) = 0;

	virtual void Write(double value) = 0;
	virtual void Write(int count, const double values[]) = 0;
};
