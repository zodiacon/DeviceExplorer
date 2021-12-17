#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include "CompoundFile.h"

namespace StructuredStorage {
	class CompoundFileWriter {
	public:
		CompoundFileWriter(StructuredFile& file);
		CompoundFileWriter(StructuredDirectory& dir, const std::wstring& name);

		template<typename T>
		void Write(const T& value) {
			static_assert(std::is_trivially_copyable<T>(), "T must be POD");

			m_File.Write(&value, sizeof(value));
		}

		void Write(const std::wstring& value);
		void Write(PCWSTR value) {
			return Write(std::wstring(value));
		}
		void Write(CString const& value) {
			return Write(std::wstring((PCWSTR)value));
		}
		void Write(const std::string& value);

		template<typename T>
		void Write(const std::vector<T>& vec) {
			auto count = static_cast<uint32_t>(vec.size());
			Write(count);
			if (std::is_pod<T>::value) {
				m_File.Write(vec.data(), count * sizeof(T));
			}
			else {
				for (const auto& item : vec)
					Write(item);
			}
		}

		template<typename T1, typename T2>
		void Write(const std::pair<T1, T2>& pair) {
			Write(pair.first);
			Write(pair.second);
		}

		template<typename K, typename V>
		void Write(const std::map<K, V>& map) {
			Write(static_cast<uint32_t>(map.size()));
			for (const auto& pair : map)
				Write(pair);
		}

		template<typename K, typename V>
		void Write(const std::unordered_map<K, V>& map) {
			Write(static_cast<uint32_t>(map.size()));
			for (const auto& pair : map)
				Write(pair);
		}

	private:
		StructuredFile m_File;
	};

	class CompoundFileReader {
	public:
		CompoundFileReader(const StructuredFile& file);
		CompoundFileReader(const StructuredDirectory& dir, const std::wstring& name);


		template<typename T>
		void Read(T& value) const {
			static_assert(std::is_trivially_copyable<T>(), "T must be POD");

			m_File.Read(&value, sizeof(value));
		}

		void Read(std::wstring& value) const;
		void Read(std::string& value) const;

		template<typename T>
		void Read(std::vector<T>& vec) const {
			uint32_t count;
			Read(count);
			if (count == 0) {
				vec.clear();
				return;
			}

			vec.resize(count);
			if (std::is_pod<T>::value) {
				m_File.Read(vec.data(), count * sizeof(T));
			}
			else {
				for (uint32_t i = 0; i < count; ++i) {
					T value;
					Read(value);
					vec[i] = value;
				}
			}
		}

		template<typename T1, typename T2>
		void Read(std::pair<T1, T2>& pair) const {
			Read(pair.first);
			Read(pair.second);
		}

		template<typename K, typename V>
		void Read(std::map<K, V>& map) const {
			uint32_t count;
			Read(count);
			for (uint32_t i = 0; i < count; i++) {
				std::pair<K, V> pair;
				Read(pair);
				map.insert(pair);
			}
		}

		template<typename K, typename V>
		void Read(std::unordered_map<K, V>& map) const {
			uint32_t count;
			Read(count);
			for (uint32_t i = 0; i < count; i++) {
				std::pair<K, V> pair;
				Read(pair);
				map.insert(pair);
			}
		}

	private:
		const StructuredFile m_File;
	};

	template<typename T>
	bool CreateFileAndWrite(StructuredDirectory& dir, const std::wstring& name, const T& value) {
		auto file = dir.CreateStructuredFile(name);
		if (!file)
			return false;
		CompoundFileWriter writer(file);
		writer.Write(value);
		return true;
	}

	template<typename T>
	bool OpenFileAndRead(const StructuredDirectory& dir, const std::wstring& name, T& value) {
		auto file = dir.OpenStructuredFile(name);
		if (!file)
			return false;
		CompoundFileReader reader(file);
		reader.Read(value);
		return true;
	}

}
