#pragma once

enum class DeviceNodeStatus : uint32_t {
	None = 0,
	RootEnumerated = DN_ROOT_ENUMERATED,
	DriverLoaded = DN_DRIVER_LOADED,
	EnumLoaded = DN_ENUM_LOADED,
	Started = DN_STARTED,
	Manual = DN_MANUAL,
	NeedToEnum = DN_NEED_TO_ENUM,
	NotFirstTime = DN_NOT_FIRST_TIME,
	HardwareEnum = DN_HARDWARE_ENUM,
	Liar = DN_LIAR,
	HasMark = DN_HAS_MARK,
	HasProblem = DN_HAS_PROBLEM,
	Filtered = DN_FILTERED,
	Moved = DN_MOVED,
	Disableable = DN_DISABLEABLE,
	Removable = DN_REMOVABLE,
	PrivateProblem = DN_PRIVATE_PROBLEM,
	MfParent = DN_MF_PARENT,
	MfChild = DN_MF_CHILD,
	WillBeRmoved = DN_WILL_BE_REMOVED,
	NotFirstTimeEnum = DN_NOT_FIRST_TIMEE,
	StopFreeResources = DN_STOP_FREE_RES,
	RealCandidate = DN_REBAL_CANDIDATE,
	BadPartial = DN_BAD_PARTIAL,
	NtEnumerator = DN_NT_ENUMERATOR,
	NtDriver = DN_NT_DRIVER,
	NeedsLocking = DN_NEEDS_LOCKING,
	ArmWakeup = DN_ARM_WAKEUP,
	ApmEnumerator = DN_APM_ENUMERATOR,
	ApmDriver = DN_APM_DRIVER,
	SilentInstall = DN_SILENT_INSTALL,
	NoShowInDeviceManager = DN_NO_SHOW_IN_DM,
	BootLogProblem = DN_BOOT_LOG_PROB,
};
DEFINE_ENUM_FLAG_OPERATORS(DeviceNodeStatus);

enum class DeviceNodeProblem {
	None = 0,
	NotConfigured,
	DEVLOADER_FAILED,
	OUT_OF_MEMORY,
	ENTRY_IS_WRONG_TYPE,
	LACKED_ARBITRATOR,
	BOOT_CONFIG_CONFLICT,
	FAILED_FILTER,
	DEVLOADER_NOT_FOUND,
	INVALID_DATA,
	FAILED_START,
	LIAR,
	NORMAL_CONFLICT,
	NOT_VERIFIED,
	NEED_RESTART,
	REENUMERATION,
	PARTIAL_LOG_CONF,
	UNKNOWN_RESOURCE,
	REINSTALL,
	REGISTRY,
	VXDLDR,
	WILL_BE_REMOVED,
	DISABLED,
	DEVLOADER_NOT_READY,
	DEVICE_NOT_THERE,
	MOVED,
	TOO_EARLY,
	NO_VALID_LOG_CONF,
	FAILED_INSTALL,
	HARDWARE_DISABLED,
	CANT_SHARE_IRQ,
	FAILED_ADD,
	DISABLED_SERVICE,
	TRANSLATION_FAILED,
	NO_SOFTCONFIG,
	BIOS_TABLE,
	IRQ_TRANSLATION_FAILED,
	FAILED_DRIVER_ENTRY,
	DRIVER_FAILED_PRIOR_UNLOAD,
	DRIVER_FAILED_LOAD,
	DRIVER_SERVICE_KEY_INVALID,
	LEGACY_SERVICE_NO_DEVICES,
	DUPLICATE_DEVICE,
	FAILED_POST_START,
	HALTED,
	PHANTOM,
	SYSTEM_SHUTDOWN,
	HELD_FOR_EJECT,
	DRIVER_BLOCKED,
	REGISTRY_TOO_LARGE,
	SETPROPERTIES_FAILED,
	WAITING_ON_DEPENDENCY,
	UNSIGNED_DRIVER,
	USED_BY_DEBUGGER,
	DEVICE_RESET,
	CONSOLE_LOCKED,
	NEED_CLASS_CONFIG,
	GUEST_ASSIGNMENT_FAILED,
};

enum class ResourceType {
	Memory = ResType_Mem,
	Interrupt = ResType_IRQ,
	IO = ResType_IO,
	LargeMemory = ResType_MemLarge,
	BusNumber = ResType_BusNumber,
	DMA = ResType_DMA,
	Private = ResType_DevicePrivate,
	PCCardConfig = ResType_PcCardConfig,
	MFCardConfig = ResType_MfCardConfig
};

enum class LogicalConfigurationType {
	Basic = BASIC_LOG_CONF,
	Filtered = FILTERED_LOG_CONF,
	Allocated = ALLOC_LOG_CONF,
	Boot = BOOT_LOG_CONF,
	Forced = FORCED_LOG_CONF,
	Override = OVERRIDE_LOG_CONF,
};

struct DeviceResource {
	ResourceType Type;
	IO_RESOURCE& IO() const {
		return *(IO_RESOURCE*)_buffer.get();
	}
	IRQ_RESOURCE& Interrupt() const {
		return *(IRQ_RESOURCE*)_buffer.get();
	}
	MEM_RESOURCE& Memory() const {
		return *(MEM_RESOURCE*)_buffer.get();
	}
	MEM_LARGE_RESOURCE& LargeMemory() const {
		return *(MEM_LARGE_RESOURCE*)_buffer.get();
	}
	BUSNUMBER_RESOURCE& BusNumber() const {
		return *(BUSNUMBER_RESOURCE*)_buffer.get();
	}
	BYTE* Buffer() const {
		return _buffer.get();
	}
	ULONG Size() const {
		return _size;
	}
private:
	friend class DeviceNode;
	std::unique_ptr<BYTE[]> _buffer;
	ULONG _size;
};

class DeviceNode {
public:
	DeviceNode(DEVINST inst) : m_Inst(inst) {}

	operator DEVINST() const {
		return m_Inst;
	}

	static std::vector<DeviceNode> GetChildDevNodes(DeviceNode const& inst);
	static std::vector<DeviceNode> GetSiblingDevNodes(DeviceNode const& inst);

	std::vector<DeviceNode> GetChildren() const;
	std::vector<DeviceNode> GetSiblings() const;
	std::vector<DEVPROPKEY> GetPropertyKeys() const;
	std::wstring GetName() const;

	bool Enable();
	bool Disable();
	bool Uninstall();
	bool Rescan();
	bool IsEnabled() const;

	std::unique_ptr<BYTE[]> GetPropertyValue(DEVPROPKEY const& key, DEVPROPTYPE& type, ULONG* len = nullptr) const {
		ULONG size = 0;
		::CM_Get_DevNode_Property(m_Inst, &key, &type, nullptr, &size, 0);
		auto value = std::make_unique<BYTE[]>(size);
		::CM_Get_DevNode_Property(m_Inst, &key, &type, value.get(), &size, 0);
		if (len)
			*len = size;
		return value;
	}

	template<typename T>
	T GetProperty(DEVPROPKEY const& key) const {
		T value{};
		ULONG size = sizeof(T);
		::CM_Get_DevNode_Property(m_Inst, &key, nullptr, (PBYTE)&value, &size, 0);
		return value;
	}

	template<>
	std::wstring GetProperty(DEVPROPKEY const& key) const {
		DEVPROPTYPE type;
		ULONG size = 0;
		if (CR_BUFFER_SMALL != ::CM_Get_DevNode_Property(m_Inst, &key, &type, nullptr, &size, 0))
			return L"";

		assert(type == DEVPROP_TYPE_STRING);
		if (type != DEVPROP_TYPE_STRING)
			return L"";
		std::wstring value;
		value.resize(size / sizeof(WCHAR));
		::CM_Get_DevNode_Property(m_Inst, &key, &type, (PBYTE)value.data(), &size, 0);
		return value;
	}

	template<>
	std::vector<std::wstring> GetProperty(DEVPROPKEY const& key) const {
		DEVPROPTYPE type;
		ULONG size = 0;
		if (ERROR_INSUFFICIENT_BUFFER != ::CM_Get_DevNode_Property(m_Inst, &key, &type, nullptr, &size, 0))
			return {};

		assert(type == DEVPROP_TYPE_STRING_LIST);
		if (type != DEVPROP_TYPE_STRING_LIST)
			return {};
		std::vector<std::wstring> value;
		auto buffer = std::make_unique<WCHAR[]>(size / sizeof(WCHAR));
		::CM_Get_DevNode_Property(m_Inst, &key, &type, (PBYTE)buffer.get(), &size, 0);
		auto p = buffer.get();
		while (*p) {
			value.push_back(p);
			p += wcslen(p) + 1;
		}
		return value;
	}

	DeviceNodeStatus GetStatus(DeviceNodeProblem* problem = nullptr) const;

	std::vector<DeviceResource> GetResources(LogicalConfigurationType type = LogicalConfigurationType::Basic);

private:
	DEVINST m_Inst;
};

