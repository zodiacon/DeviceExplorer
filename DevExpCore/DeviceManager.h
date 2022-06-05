#pragma once

#include <cfgmgr32.h>
#include "DeviceNode.h"

namespace wil {
	using unique_hinfoset = unique_any_handle_invalid<decltype(&::SetupDiDestroyDeviceInfoList), ::SetupDiDestroyDeviceInfoList>;
}

template<>
struct std::hash<GUID> {
	size_t operator()(const GUID& key) const {
		return key.Data1 ^ ((key.Data3 << 16) | key.Data2);
	}
};

struct DeviceInterfaceInfo {
	std::wstring FriendlyName;
	std::wstring SymbolicLink;
	std::wstring DeviceFriendlyName;
	std::wstring DeviceDescription;
	GUID Guid;
};

struct DeviceClassInfo {
	GUID Guid;
	std::wstring Name;
};

struct HardwareProfile {
	uint32_t Index;
	std::wstring FriendlyName;
};

enum class DeviceClassRegistryPropertyType {
	Characteristics = SPCRP_CHARACTERISTICS,
	DeviceType = SPCRP_DEVTYPE,
	UpperFilters = SPCRP_UPPERFILTERS,
	LowerFilters = SPCRP_LOWERFILTERS,
	Exclusive = SPCRP_EXCLUSIVE,
	SecurityDescriptor = SPCRP_SECURITY,
	SecurityDescriptionString = SPCRP_SECURITY_SDS
};

enum class DeviceRegistryPropertyType {
	Description = SPDRP_DEVICEDESC,
	HardwareId = SPDRP_HARDWAREID,
	CompatibleIds = SPDRP_COMPATIBLEIDS,
	Service = SPDRP_SERVICE,
	Class = SPDRP_CLASS,
	ClassGuid = SPDRP_CLASSGUID,
	Driver = SPDRP_DRIVER,
	ConfigFlags = SPDRP_CONFIGFLAGS,
	Mfg = SPDRP_MFG,
	FriendlyName = SPDRP_FRIENDLYNAME,
	UpperFilters = SPDRP_UPPERFILTERS,
	LowerFilters = SPDRP_LOWERFILTERS,
	Capabilities = SPDRP_CAPABILITIES,
	PdoName = SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
	Enumerator = SPDRP_ENUMERATOR_NAME,
	Location = SPDRP_LOCATION_INFORMATION,
	UINumber = SPDRP_UI_NUMBER,
	BusTypeGuid = SPDRP_BUSTYPEGUID,
	Exclusive = SPDRP_EXCLUSIVE,
	DeviceType = SPDRP_DEVTYPE,
	BusNumber = SPDRP_BUSNUMBER,
	Address = SPDRP_ADDRESS,
	Characteristics = SPDRP_CHARACTERISTICS,
	LegacyBusType = SPDRP_LEGACYBUSTYPE,
	InstallState = SPDRP_INSTALL_STATE,
	PowerData = SPDRP_DEVICE_POWER_DATA,
	SecurityDescriptor = SPDRP_SECURITY,
	SecurityDescriptorString = SPDRP_SECURITY_SDS,
	LocationPaths = SPDRP_LOCATION_PATHS,
	UINumberDescFormat = SPDRP_UI_NUMBER_DESC_FORMAT,
	RemovalPolicy = SPDRP_REMOVAL_POLICY,
	RemovalPolicyHwDefault = SPDRP_REMOVAL_POLICY_HW_DEFAULT,
	RemovalPolicyOverride = SPDRP_REMOVAL_POLICY_OVERRIDE,
	BaseContainerId = SPDRP_BASE_CONTAINERID
};

enum class InfoSetOptions {
	AllClasses = DIGCF_ALLCLASSES,
	Profile = DIGCF_PROFILE,
	Present = DIGCF_PRESENT,
	Default = DIGCF_DEFAULT,
	DeviceInterface = DIGCF_DEVICEINTERFACE
};
DEFINE_ENUM_FLAG_OPERATORS(InfoSetOptions);

struct DeviceInfo {
	std::wstring Description;
	SP_DEVINFO_DATA Data;
};

struct DeviceDriverInfo {
	std::wstring Description;
	std::wstring ManufactorName;
	std::wstring ProviderName;
	FILETIME  DriverDate;
	DWORDLONG DriverVersion;
	std::wstring DriverDesc;
	std::wstring InfFile;
	DWORD Type;
};

class DeviceManager final {
public:
	static std::unique_ptr<DeviceManager> Create(const wchar_t* computerName = nullptr, const GUID* classGuid = nullptr, const wchar_t* enumerator = nullptr,
		InfoSetOptions options = InfoSetOptions::Present | InfoSetOptions::AllClasses);

	template<typename T = DeviceInfo> requires (std::is_base_of_v<DeviceInfo, T>)
	std::vector<T> EnumDevices(bool includeHidden = false);
	static std::wstring GetDeviceClassDescription(GUID const& guid, const wchar_t* computerName = nullptr);
	static HIMAGELIST GetClassImageList();
	static int GetClassImageIndex(GUID const& guid);
	static std::vector<DEVPROPKEY> GetClassPropertyKeys(GUID const& guid);
	static std::vector<HardwareProfile> EnumHardwareProfiles(PCWSTR computerName = nullptr);
	static std::vector<DEVPROPKEY> GetDeviceClassPropertyKeys(GUID const& guid);
	static std::vector<DEVPROPKEY> GetDeviceInterfacePropertyKeys(GUID const& guid);
	static std::unique_ptr<BYTE[]> GetClassPropertyValue(GUID const& guid, DEVPROPKEY const& key, DEVPROPTYPE& type, ULONG* len, bool iface = false);
	static std::vector<GUID> BuildClassInfoList(DWORD flags = DIBCI_NOINSTALLCLASS | DIBCI_NODISPLAYCLASS);
	static std::wstring GetSetupClassDescription(GUID const& guid);

	static DeviceNode GetRootDeviceNode();

	HDEVINFO InfoSet() {
		return m_hInfoSet.get();
	}

	// device
	std::wstring GetDeviceRegistryPropertyString(const DeviceInfo& di, DeviceRegistryPropertyType type) const;
	std::vector<std::wstring> GetDeviceRegistryPropertyMultiString(const DeviceInfo& di, DeviceRegistryPropertyType type) const;
	template<typename T>
	T GetDeviceRegistryProperty(const DeviceInfo& di, DeviceRegistryPropertyType type) const;
	HICON GetDeviceIcon(const DeviceInfo& di, bool big = false) const;
	DeviceInfo const& GetDevice(int index) const;
	std::vector<DeviceDriverInfo> EnumDrivers(DeviceInfo const& di, bool compat = false);
	//std::vector<DriverInfo> EnumDrivers();

	// device class
	static std::wstring GetDeviceClassRegistryPropertyString(const GUID* guid, DeviceClassRegistryPropertyType type);
	static std::vector<std::wstring> GetDeviceClassRegistryPropertyMultiString(const GUID* guid, DeviceClassRegistryPropertyType type);
	static std::vector<DeviceClassInfo> EnumDeviceClasses();
	static std::vector<GUID> EnumDeviceClassesGuids();

	template<typename T>
	static T GetDeviceClassRegistryProperty(const GUID* guid, DeviceClassRegistryPropertyType type);

	bool EnumDeviceInterfaces(GUID const& guid, std::vector<DeviceInterfaceInfo>& vec);
	static std::vector<DeviceInterfaceInfo> EnumDeviceInterfaces();
	static std::vector<GUID> EnumDeviceInterfacesGuids();
	static std::wstring GetDeviceInterfaceName(GUID const& guid);

	template<typename F>
	DeviceInfo const* FindDevice(F&& f) const {
		for (auto const& dev : m_devices)
			if (f(dev))
				return &dev;
		return nullptr;
	}

	template<typename T>
	static T GetDeviceInterfaceProperty(GUID const& guid, DEVPROPKEY const& key) {
		T value{};
		ULONG size = sizeof(T);
		WCHAR sguid[64];
		::StringFromGUID2(guid, sguid, _countof(sguid));
		DEVPROPTYPE type;
		::CM_Get_Device_Interface_Property(sguid, &key, &type, &value, &size, 0);
		return value;
	}

	template<>
	static std::wstring GetDeviceInterfaceProperty(GUID const& guid, DEVPROPKEY const& key) {
		DEVPROPTYPE type;
		ULONG size = 0;
		WCHAR sguid[64];
		if (0 == ::StringFromGUID2(guid, sguid, _countof(sguid)))
			return L"";

		if (CR_BUFFER_SMALL != ::CM_Get_Device_Interface_Property(sguid, &key, &type, nullptr, &size, 0))
			return L"";

		assert(type == DEVPROP_TYPE_STRING);
		if (type != DEVPROP_TYPE_STRING)
			return L"";
		std::wstring value;
		value.resize(size / sizeof(WCHAR));
		::CM_Get_Device_Interface_Property(sguid, &key, &type, (PBYTE)value.data(), &size, 0);
		return value;
	}

	int GetDeviceIndex(DEVINST inst) const;

private:
	DeviceManager(const wchar_t* computerName = nullptr, const GUID* classGuid = nullptr, const wchar_t* enumerator = nullptr,
		InfoSetOptions options = InfoSetOptions::Present | InfoSetOptions::AllClasses);

	static std::vector<DEVPROPKEY> GetDeviceClassPropertyKeysCommon(GUID const& guid, bool deviceClass);

private:
	wil::unique_hinfoset m_hInfoSet;
	std::vector<DeviceInfo> m_devices;
	std::unordered_map<DEVINST, int> _devMap;
};

template<typename T>
inline T DeviceManager::GetDeviceRegistryProperty(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	static_assert(std::is_trivially_copyable<T>());
	T result{};

	::SetupDiGetDeviceRegistryProperty(m_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type), nullptr,
		(BYTE*)&result, sizeof(T), nullptr);
	return result;
}

template<typename T>
inline T DeviceManager::GetDeviceClassRegistryProperty(const GUID* guid, DeviceClassRegistryPropertyType type) {
	static_assert(std::is_trivially_copyable<T>());
	T result{};
	DWORD regType;
	if (::SetupDiGetClassRegistryProperty(guid, static_cast<DWORD>(type), &regType,
		(BYTE*)&result, sizeof(T), nullptr, nullptr, nullptr)) {
	}
	return result;
}

template<typename T> requires (std::is_base_of_v<DeviceInfo, T>)
std::vector<T> DeviceManager::EnumDevices(bool includeHidden) {
	std::vector<T> devices;
	SP_DEVINFO_DATA data = { sizeof(data) };
	_devMap.clear();
	m_devices.clear();

	for (DWORD i = 0; ; i++) {
		if (!::SetupDiEnumDeviceInfo(m_hInfoSet.get(), i, &data))
			break;

		if (!includeHidden && (DeviceNode(data.DevInst).GetStatus() & DeviceNodeStatus::NoShowInDeviceManager) == DeviceNodeStatus::NoShowInDeviceManager)
			continue;

		T di;
		di.Description = DeviceNode(data.DevInst).GetName();
		di.Data = data;
		_devMap.insert({ data.DevInst, (int)devices.size() });
		m_devices.push_back(di);
		devices.push_back(std::move(di));
	}
	return devices;
}
