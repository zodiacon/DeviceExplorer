#include "pch.h"
#include "CompoundFile.h"

using namespace std;
using namespace StructuredStorage;

CompoundFile CompoundFile::Create(const std::wstring& path, PSECURITY_DESCRIPTOR securityDescriptor) {
	CComPtr<IStorage> spStg;
	auto hr = ::StgCreateStorageEx(path.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		STGFMT_STORAGE, 0, nullptr, securityDescriptor, __uuidof(IStorage), reinterpret_cast<void**>(&spStg));

	return CompoundFile(spStg);
}

CompoundFile CompoundFile::Open(const std::wstring& path, CompoundFileMode mode) {
	CComPtr<IStorage> spStg;
	auto hr = ::StgOpenStorageEx(path.c_str(), (mode == CompoundFileMode::Read ? STGM_READ : STGM_READWRITE) | STGM_SHARE_EXCLUSIVE,
		STGFMT_STORAGE, 0, nullptr, nullptr, __uuidof(IStorage), reinterpret_cast<void**>(&spStg));

	return CompoundFile(spStg, mode);
}

StructuredDirectory StructuredDirectory::CreateStructuredDirectory(const std::wstring& name) {
	CheckNameLength(name);

	CComPtr<IStorage> spStg;
	auto hr = GetStorage()->CreateStorage(name.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStg);

	return StructuredDirectory(spStg);
}

StructuredFile StructuredDirectory::CreateStructuredFile(const std::wstring& name) {
	CheckNameLength(name);

	CComPtr<IStream> spStm;
	auto hr = GetStorage()->CreateStream(name.c_str(), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStm);

	return StructuredFile(spStm);
}

StructuredFile StructuredDirectory::OpenStructuredFile(const std::wstring& name) const {
	CheckNameLength(name);

	CComPtr<IStream> spStm;
	auto hr = GetStorage()->OpenStream(name.c_str(), nullptr,
		(GetMode() == CompoundFileMode::Read ? STGM_READ : STGM_READWRITE) | STGM_SHARE_EXCLUSIVE, 0, &spStm);

	return StructuredFile(spStm);
}

StructuredDirectory StructuredDirectory::OpenStructuredDirectory(const std::wstring& name) const {
	CheckNameLength(name);

	CComPtr<IStorage> spStg;
	auto hr = GetStorage()->OpenStorage(name.c_str(), nullptr,
		(GetMode() == CompoundFileMode::Read ? STGM_READ : STGM_READWRITE) | STGM_SHARE_EXCLUSIVE, 0, 0, &spStg);

	return StructuredDirectory(spStg, GetMode());
}

void StructuredDirectory::Close() {
	m_spStorage = nullptr;
}

bool StructuredDirectory::CheckNameLength(const std::wstring& name) const {
	return name.size() < 32;
}

HRESULT StructuredFile::Write(const void* buffer, uint32_t count) {
	return m_spStream->Write(buffer, count, nullptr);
}

HRESULT StructuredFile::Read(void* buffer, uint32_t count) const {
	return m_spStream->Read(buffer, count, nullptr);
}

HRESULT StructuredFile::Seek(uint32_t offset, SeekMode mode, uint32_t* newOffset) {
	LARGE_INTEGER li;
	li.QuadPart = offset;
	ULARGE_INTEGER newOffsetLocal;
	auto hr = m_spStream->Seek(li, static_cast<DWORD>(mode), &newOffsetLocal);
	if (SUCCEEDED(hr) && newOffset)
		*newOffset = newOffsetLocal.LowPart;
	return hr;
}

uint32_t StructuredFile::GetSize() const {
	STATSTG stat = { 0 };
	m_spStream->Stat(&stat, STATFLAG_NONAME);
	return stat.cbSize.LowPart;
}

void StructuredFile::Close() {
	m_spStream = nullptr;
}
