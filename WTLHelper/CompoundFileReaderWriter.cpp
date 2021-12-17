#include "pch.h"
#include "CompoundFileReaderWriter.h"

using namespace StructuredStorage;

CompoundFileWriter::CompoundFileWriter(StructuredFile& file) : m_File(file) {
}

StructuredStorage::CompoundFileWriter::CompoundFileWriter(StructuredDirectory& dir, const std::wstring& name) 
	: m_File(dir.CreateStructuredFile(name)) {
}

CompoundFileReader::CompoundFileReader(const StructuredFile& file) : m_File(file) {
}

void CompoundFileWriter::Write(const std::wstring& value) {
	auto len = static_cast<uint32_t>(value.size());
	m_File.Write(&len, sizeof(len));
	m_File.Write(value.c_str(), len * sizeof(wchar_t));
}

void CompoundFileReader::Read(std::wstring& value) const {
	uint32_t len;
	m_File.Read(&len, sizeof(len));
	value.clear();
	if (len) {
		auto buffer = std::make_unique<wchar_t[]>(len);
		m_File.Read(buffer.get(), len * sizeof(WCHAR));
		value.assign(buffer.get(), len);
	}
}

void CompoundFileWriter::Write(const std::string& value) {
	auto len = static_cast<uint32_t>(value.size());
	m_File.Write(&len, sizeof(len));
	m_File.Write(value.c_str(), len * sizeof(char));
}

void CompoundFileReader::Read(std::string& value) const {
	uint32_t len;
	m_File.Read(&len, sizeof(len));
	auto buffer = std::make_unique<char[]>(len);
	m_File.Read(buffer.get(), len);
	value.assign(buffer.get(), len);
}

