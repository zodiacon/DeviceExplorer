#pragma once

#include "DeviceNode.h"

struct Helpers final {
	static CString GetPropertyName(DEVPROPKEY const& key);
	static CString GuidToString(GUID const& guid);
	static CString GetPropertyValueAsString(DEVPROPKEY const& key, DeviceNode const& node, PCWSTR sep = L"; ");
	static CString GetPropertyValueAsString(PBYTE value, DEVPROPTYPE type, ULONG size, PCWSTR sep = L"; ");
	static CString FormatBytes(const PBYTE buffer, ULONG size);
	static CString GetPropertyDetails(DEVPROPKEY const& key, PBYTE value, ULONG size);
	static PCWSTR InstallStateToString(ULONG state);
	static PCWSTR RemovalPolicyToString(ULONG policy);
	static CString DevNodeStatusToString(DeviceNodeStatus status, PCWSTR sep = L", ");
	static PCWSTR PciDeviceTypeToString(ULONG type);
};
