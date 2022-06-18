#pragma once

enum class DeviceDriverType {
	Kernel = 1,
	FileSystem = 2,
	WDF = 8,
};
DEFINE_ENUM_FLAG_OPERATORS(DeviceDriverType);

struct DriverInfo {
	std::wstring Name;
	std::wstring DisplayName;
	std::wstring ImagePath;
	DeviceDriverType Type;
	uint32_t State;
	uint32_t MajorVersion, MinorVersion;
};

class DriverManager {
public:
	static std::vector<DriverInfo> EnumKernelDrivers(bool runningOnly = false);
	static std::vector<DriverInfo> EnumUserDrivers(bool runningOnly = false);
};

