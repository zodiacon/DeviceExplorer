#pragma once

#include <objbase.h>
#include <string>
#include <memory>

namespace StructuredStorage {
	enum class CompoundFileMode {
		Read,
		ReadWrite
	};

	enum class SeekMode {
		Set,
		Current,
		End
	};

	class StructuredFile {
		friend class StructuredDirectory;

	public:
		bool IsValid() const {
			return m_spStream != nullptr;
		}

		operator bool() const {
			return IsValid();
		}

		HRESULT Write(const void* buffer, uint32_t count);
		HRESULT Read(void* buffer, uint32_t count) const;

		HRESULT Seek(uint32_t offset, SeekMode mode = SeekMode::Set, uint32_t* newOffset = nullptr);

		uint32_t GetSize() const;

		void Close();

		operator IStream* () const {
			return m_spStream.p;
		}

	private:
		StructuredFile(IStream* pStm) : m_spStream(pStm) {}

		CComPtr<IStream> m_spStream;
	};

	class StructuredDirectory {
	public:
		StructuredDirectory CreateStructuredDirectory(const std::wstring& name);
		StructuredFile CreateStructuredFile(const std::wstring& name);
		StructuredDirectory OpenStructuredDirectory(const std::wstring& name) const;
		StructuredFile OpenStructuredFile(const std::wstring& name) const;

		bool IsValid() const {
			return m_spStorage.p;
		}

		operator bool() const {
			return IsValid();
		}

		void Close();

		~StructuredDirectory() {
			Close();
		}

		CompoundFileMode GetMode() const {
			return m_FileMode;
		}

	protected:
		IStorage* GetStorage() const {
			return m_spStorage.p;
		}

		StructuredDirectory(IStorage* pStg, CompoundFileMode mode = CompoundFileMode::ReadWrite) : m_spStorage(pStg), m_FileMode(mode) {
		}

		bool CheckNameLength(const std::wstring& name) const;

	private:
		CComPtr<IStorage> m_spStorage;
		CompoundFileMode m_FileMode;
	};

	class CompoundFile : public StructuredDirectory {
	public:
		// factory methods

		static CompoundFile Create(const std::wstring& path, PSECURITY_DESCRIPTOR securityDescriptor = nullptr);
		static CompoundFile Open(const std::wstring& path, CompoundFileMode mode = CompoundFileMode::Read);

		// ctors

		CompoundFile(const CompoundFile&) = delete;
		CompoundFile& operator=(const CompoundFile&) = delete;

		CompoundFile(CompoundFile&&) = default;
		CompoundFile& operator=(CompoundFile&&) = default;

	private:
		CompoundFile(IStorage* pStg, CompoundFileMode mode = CompoundFileMode::ReadWrite) : StructuredDirectory(pStg, mode) {}
	};


}
