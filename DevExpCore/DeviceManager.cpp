#include "pch.h"
#include "DeviceManager.h"
#include <RegStr.h>

#pragma comment(lib, "setupapi")
#pragma comment(lib, "cfgmgr32")

SP_CLASSIMAGELIST_DATA g_ClassImageList;

std::unique_ptr<DeviceManager> DeviceManager::Create(const wchar_t* computerName, const GUID* classGuid, const wchar_t* enumerator, InfoSetOptions options) {
	auto dm = new DeviceManager(computerName, classGuid, enumerator, options);
	if (dm->_hInfoSet)
		return std::unique_ptr<DeviceManager>(dm);
	delete dm;
	return nullptr;
}

std::wstring DeviceManager::GetDeviceClassDescription(GUID const& guid, const wchar_t* computerName) {
	wchar_t desc[256];
	if (::SetupDiGetClassDescriptionEx(&guid, desc, _countof(desc), nullptr, computerName, nullptr)) {
		return desc;
	}
	return L"";
}

HIMAGELIST DeviceManager::GetClassImageList() {
	if (g_ClassImageList.ImageList == nullptr) {
		g_ClassImageList.cbSize = sizeof(g_ClassImageList);
		::SetupDiGetClassImageList(&g_ClassImageList);
	}
	return g_ClassImageList.ImageList;
}

int DeviceManager::GetClassImageIndex(const GUID* guid) {
	int index = -1;
	::SetupDiGetClassImageIndex(&g_ClassImageList, guid, &index);
	return index;
}

std::vector<DEVPROPKEY> DeviceManager::GetClassPropertyKeys(GUID const& guid) {
	std::vector<DEVPROPKEY> keys;
	DWORD count = 0;
	::SetupDiGetClassPropertyKeys(&guid, nullptr, 0, &count, DICLASSPROP_INSTALLER);
	if (count) {
		keys.resize(count);
		::SetupDiGetClassPropertyKeys(&guid, keys.data(), count, nullptr, DICLASSPROP_INSTALLER);
	}
	return keys;
}

std::vector<HardwareProfile> DeviceManager::EnumHardwareProfiles(PCWSTR computerName) {
	std::vector<HardwareProfile> hwprofiles;
	DWORD size;
	if (!::SetupDiGetHwProfileListEx(nullptr, 0, &size, nullptr, computerName, nullptr) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		auto profiles = std::make_unique<DWORD[]>(size);
		if (::SetupDiGetHwProfileListEx(profiles.get(), size, &size, nullptr, computerName, nullptr)) {
			WCHAR name[256];
			hwprofiles.reserve(size);
			for (DWORD i = 0; i < size; i++) {
				HardwareProfile profile;
				profile.Index = i;
				if (::SetupDiGetHwProfileFriendlyNameEx(i, name, _countof(name), nullptr, computerName, nullptr))
					profile.FriendlyName = name;
				hwprofiles.push_back(std::move(profile));
			}
		}
	}
	return hwprofiles;
}

DeviceNode DeviceManager::GetRootDeviceNode() {
	DEVINST inst;
	return CR_SUCCESS == ::CM_Locate_DevNode(&inst, nullptr, CM_LOCATE_DEVNODE_NORMAL) ? inst : 0;
}

std::wstring DeviceManager::GetDeviceRegistryPropertyString(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	std::wstring result;
	result.resize(256);

	DWORD regType;
	if (::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type), &regType,
		(BYTE*)result.data(), DWORD(result.size() * sizeof(wchar_t)), nullptr)) {
		assert(regType == REG_SZ);
		return result;
	}
	return L"";
}

std::vector<std::wstring> DeviceManager::GetDeviceRegistryPropertyMultiString(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	std::vector<std::wstring> result;
	WCHAR buffer[1 << 11];
	DWORD regType;
	auto ok = ::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type), &regType, (BYTE*)buffer, sizeof(buffer), nullptr);
	if (!ok)
		return result;

	assert(regType == REG_MULTI_SZ);
	for (auto p = buffer; *p;) {
		result.push_back(p);
		p += ::wcslen(p) + 1;
	}
	return result;
}

HICON DeviceManager::GetDeviceIcon(const DeviceInfo& di, bool big) const {
	HICON hIcon = nullptr;
	auto size = big ? 32 : 16;
	::SetupDiLoadDeviceIcon(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, size, size, 0, &hIcon);
	return hIcon;
}

std::wstring DeviceManager::GetDeviceClassRegistryPropertyString(const GUID* guid, DeviceClassRegistryPropertyType type) {
	DWORD regType;
	std::wstring result;
	result.resize(256);
	if (::SetupDiGetClassRegistryProperty(guid, static_cast<DWORD>(type), &regType, 
		(BYTE*)result.data(), DWORD(result.size() * sizeof(WCHAR)), nullptr, nullptr, nullptr)) {
		assert(regType == REG_SZ);
		return result;
	}

	return L"";
}

std::vector<std::wstring> DeviceManager::GetDeviceClassRegistryPropertyMultiString(const GUID* guid, DeviceClassRegistryPropertyType type) {
	std::vector<std::wstring> result;
	WCHAR buffer[1 << 12];
	DWORD regType;
	if (::SetupDiGetClassRegistryProperty(guid, static_cast<DWORD>(type), &regType, (BYTE*)buffer, sizeof(buffer), nullptr, nullptr, nullptr)) {
		assert(regType == REG_MULTI_SZ);
		for (auto p = buffer; *p;) {
			result.push_back(p);
			p += ::wcslen(p) + 1;
		}
	}
	return result;
}

std::vector<std::wstring> DeviceManager::EnumDeviceClasses() {
	CRegKey key;
	std::vector<std::wstring> classes;
	classes.reserve(128);
	WCHAR name[128];
	if (ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, REGSTR_PATH_CURRENTCONTROLSET L"\\Control\\Class", KEY_READ)) {
		for (DWORD i = 0;; i++) {
			if (ERROR_SUCCESS != ::RegEnumKey(key, i, name, _countof(name)))
				break;
			classes.push_back(name);
		}
	}
	return classes;
}

bool DeviceManager::EnumDeviceInterfaces(GUID const& guid, std::vector<DeviceInterfaceInfo>& vec) {
	SP_DEVICE_INTERFACE_DATA data = { sizeof(data) };
	auto buffer = std::make_unique<BYTE[]>(2048);
	auto detail = reinterpret_cast<SP_INTERFACE_DEVICE_DETAIL_DATA*>(buffer.get());
	SP_DEVINFO_DATA devData = { sizeof(devData) };
	WCHAR name[256];

	DWORD i = 0;
	for (;; i++) {
		if (!::SetupDiEnumDeviceInterfaces(_hInfoSet.get(), nullptr, &guid, i, &data))
			break;

		DeviceInterfaceInfo info;
		info.Guid = data.InterfaceClassGuid;
		detail->cbSize = sizeof(*detail);
		if (::SetupDiGetDeviceInterfaceDetail(_hInfoSet.get(), &data, detail, 2048, nullptr, &devData)
			|| ::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			auto hKey = ::SetupDiOpenDeviceInterfaceRegKey(_hInfoSet.get(), &data, 0, KEY_READ);
			if (hKey != INVALID_HANDLE_VALUE) {
				CRegKey key(hKey);
				ULONG chars = _countof(name);
				if (ERROR_SUCCESS == key.QueryStringValue(L"FriendlyName", name, &chars))
					info.FriendlyName = name;
			}
			DeviceInfo di;
			di.Data = devData;
			info.DeviceFriendlyName = GetDeviceRegistryPropertyString(di, DeviceRegistryPropertyType::FriendlyName);
			info.DeviceDescription = GetDeviceRegistryPropertyString(di, DeviceRegistryPropertyType::Description);
			info.SymbolicLink = detail->DevicePath;
		}
		vec.push_back(std::move(info));
	}
	return i > 0;
}

std::vector<DeviceInterfaceInfo> DeviceManager::EnumDeviceInterfaces() {
	CRegKey key;
	std::vector<DeviceInterfaceInfo> data;
	WCHAR name[256];
	if (ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, REGSTR_PATH_DEVICE_CLASSES, KEY_READ)) {
		data.reserve(128);
		GUID guid;
		for (DWORD i = 0; ::RegEnumKey(key, i, name, _countof(name)) == ERROR_SUCCESS; i++) {
			if (FAILED(::CLSIDFromString(name, &guid)))
				continue;
			DeviceManager dm(nullptr, &guid, nullptr, InfoSetOptions::DeviceInterface | InfoSetOptions::Present);
			dm.EnumDeviceInterfaces(guid, data);
		}
	}
	return data;
}

DeviceManager::DeviceManager(const wchar_t* computerName, const GUID* classGuid, const wchar_t* enumerator, InfoSetOptions options) {
	_hInfoSet.reset(::SetupDiGetClassDevsEx(classGuid, enumerator, nullptr, static_cast<DWORD>(options), nullptr, computerName, nullptr));
}

std::vector<DeviceInfo> DeviceManager::EnumDevices() {
	std::vector<DeviceInfo> devices;
	SP_DEVINFO_DATA data = { sizeof(data) };
	wchar_t name[512];

	for (DWORD i = 0; ; i++) {
		if (!::SetupDiEnumDeviceInfo(_hInfoSet.get(), i, &data))
			break;

		DeviceInfo di;
		di.Data = data;
		if (::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), &data, SPDRP_FRIENDLYNAME, nullptr, (BYTE*)name, sizeof(name), nullptr)) {
			di.Description = name;
		}
		if (di.Description.empty()) {
			if (::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), &data, SPDRP_DEVICEDESC, nullptr, (BYTE*)name, sizeof(name), nullptr)) {
				di.Description = name;
			}
		}
		devices.push_back(std::move(di));
	}
	return devices;
}
