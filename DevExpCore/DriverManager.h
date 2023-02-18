#pragma once

enum class DeviceDriverType {
	Kernel = 1,
	FileSystem = 2,
	TypeMask = 15,
	KMDF = 0x20,
	UMDF = 0x40,
};
DEFINE_ENUM_FLAG_OPERATORS(DeviceDriverType);

enum class DriverStartType {
	Boot,
	System,
	Auto,
	Demand,
	Disabled
};

enum class DriverState {
	Stopped = 1,
	StartPending,
	StopPending,
	Running,
	ContinuePending,
	PausePending,
	Paused
};

struct DriverInfo {
	std::wstring Name;
	std::wstring DisplayName;
	std::wstring ImagePath;
	DeviceDriverType Type;
	DriverStartType StartType;
	DriverState State;
	uint32_t MajorVersion{ 0 }, MinorVersion{ 0 };
};

class DriverManager {
public:
	static std::vector<DriverInfo> EnumKernelDrivers(bool runningOnly = false);
	static std::vector<DriverInfo> EnumUserDrivers(bool runningOnly = false);
	static bool Start(std::wstring_view name);
	static bool Stop(std::wstring_view name);
};

