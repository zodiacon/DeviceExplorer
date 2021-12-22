#include "pch.h"
#include "Helpers.h"
#include <pciprop.h>
#include <functiondiscoverykeys.h>
#include <sddl.h>

namespace std {
	template<>
	struct hash<DEVPROPKEY> {
		size_t operator()(DEVPROPKEY const& p) const {
			return p.pid ^ p.fmtid.Data1 ^ (p.fmtid.Data2 << 16);
		}
	};
}

CString Helpers::GetPropertyName(DEVPROPKEY const& key) {
	static const std::unordered_map<DEVPROPKEY, CString> properties{
		{ DEVPKEY_NAME, L"Name" },
		{ DEVPKEY_Device_DeviceDesc, L"Description" },
		{ DEVPKEY_Device_HardwareIds, L"Hardware IDs" },
		{ DEVPKEY_Device_CompatibleIds, L"Compatible IDs" },
		{ DEVPKEY_Device_Service, L"Service" },
		{ DEVPKEY_Device_Class, L"Class" },
		{ DEVPKEY_Device_ClassGuid, L"Class GUID" },
		{ DEVPKEY_Device_Driver, L"Driver" },
		{ DEVPKEY_Device_ConfigFlags, L"Config Flags" },
		{ DEVPKEY_Device_Manufacturer, L"Manufacturer" },
		{ DEVPKEY_Device_FriendlyName, L"Friendly Name" },
		{ DEVPKEY_Device_LocationInfo, L"Location Info" },
		{ DEVPKEY_Device_PDOName, L"PDO Name" },
		{ DEVPKEY_Device_Capabilities, L"Capabilities" },
		{ DEVPKEY_Device_UINumber, L"UI Number" },
		{ DEVPKEY_Device_UpperFilters, L"Upper Filters" },
		{ DEVPKEY_Device_LowerFilters, L"Lower Filters" },
		{ DEVPKEY_Device_BusTypeGuid, L"Bus Type GUID" },
		{ DEVPKEY_Device_LegacyBusType, L"Legacy Bus Type" },
		{ DEVPKEY_Device_BusNumber, L"Bus Number" },
		{ DEVPKEY_Device_EnumeratorName, L"Enumerator" },
		{ DEVPKEY_Device_Security, L"Security" },
		{ DEVPKEY_Device_SecuritySDS, L"Security SDDL" },
		{ DEVPKEY_Device_DevType, L"Device Type" },
		{ DEVPKEY_Device_Exclusive, L"Exclusive" },
		{ DEVPKEY_Device_Characteristics, L"Characteristics" },
		{ DEVPKEY_Device_Address, L"Address" },
		{ DEVPKEY_Device_UINumberDescFormat, L"UI Number Desc Format" },
		{ DEVPKEY_Device_PowerData, L"Power Data" },
		{ DEVPKEY_Device_RemovalPolicy, L"Remove Policy" },
		{ DEVPKEY_Device_RemovalPolicyDefault, L"Default Remove Policy" },
		{ DEVPKEY_Device_RemovalPolicyOverride, L"Override Remove Policy" },
		{ DEVPKEY_Device_InstallState, L"Install State" },
		{ DEVPKEY_Device_LocationPaths, L"Location Paths" },
		{ DEVPKEY_Device_BaseContainerId, L"Base Container ID" },
		{ DEVPKEY_Device_InstanceId, L"Instance ID" },
		{ DEVPKEY_Device_DevNodeStatus, L"Status" },
		{ DEVPKEY_Device_ProblemCode, L"Problem Code" },
		{ DEVPKEY_Device_EjectionRelations, L"Ejection Relations" },
		{ DEVPKEY_Device_RemovalRelations, L"Removal Relations" },
		{ DEVPKEY_Device_PowerRelations, L"Power Relations" },
		{ DEVPKEY_Device_BusRelations, L"Bus Relations" },
		{ DEVPKEY_Device_Parent, L"Parent" },
		{ DEVPKEY_Device_Children, L"Children" },
		{ DEVPKEY_Device_Siblings, L"Siblings" },
		{ DEVPKEY_Device_TransportRelations, L"Transport Relations" },
		{ DEVPKEY_Device_ProblemStatus, L"Problem Status" },
		{ DEVPKEY_Device_Reported, L"Reported" },
		{ DEVPKEY_Device_Legacy, L"Legacy" },
		{ DEVPKEY_Device_ContainerId, L"Container ID" },
		{ DEVPKEY_Device_InLocalMachineContainer, L"In Local Machine Container" },
		{ DEVPKEY_Device_Model, L"Model" },
		{ DEVPKEY_Device_ModelId, L"Model ID" },
		{ DEVPKEY_Device_FriendlyNameAttributes, L"Friendly Name Attributes" },
		{ DEVPKEY_Device_ManufacturerAttributes, L"manufacturer Attributes" },
		{ DEVPKEY_Device_PresenceNotForDevice, L"Presence Not for Device" },
		{ DEVPKEY_Device_SignalStrength, L"Signal Strength" },
		{ DEVPKEY_Device_IsAssociateableByUserAction, L"Associateable by User Action" },
		{ DEVPKEY_Device_ShowInUninstallUI, L"Show in Uninstall UI" },
		{ DEVPKEY_Device_Numa_Proximity_Domain, L"Proximity Domain" },
		{ DEVPKEY_Device_DHP_Rebalance_Policy, L"DHP Rebalance Policy" },
		{ DEVPKEY_Device_Numa_Node, L"NUMA Node" },
		{ DEVPKEY_Device_BusReportedDeviceDesc, L"Bus Reported Device Description" },
		{ DEVPKEY_Device_IsPresent, L"Present" },
		{ DEVPKEY_Device_HasProblem, L"Has Problem" },
		{ DEVPKEY_Device_ConfigurationId, L"Configuration ID" },
		{ DEVPKEY_Device_ReportedDeviceIdsHash, L"Reported Device IDs Hash" },
		{ DEVPKEY_Device_PhysicalDeviceLocation, L"Physical Device Location" },
		{ DEVPKEY_Device_BiosDeviceName, L"BIOS Device Name" },
		{ DEVPKEY_Device_DriverProblemDesc, L"Problem Description" },
		{ DEVPKEY_Device_DebuggerSafe, L"Debugger Safe" },
		{ DEVPKEY_Device_PostInstallInProgress, L"Post Install in Progress" },
		{ DEVPKEY_Device_Stack, L"Device Stack" },
		{ DEVPKEY_Device_ExtendedConfigurationIds, L"Extended Configuration IDs" },
		{ DEVPKEY_Device_IsRebootRequired, L"Reboot Required" },
		{ DEVPKEY_Device_FirmwareDate, L"Firmware Date" },
		{ DEVPKEY_Device_FirmwareVersion, L"Firmware Version" },
		{ DEVPKEY_Device_FirmwareRevision, L"Firmware Revision" },
		{ DEVPKEY_Device_DependencyProviders, L"Dependency Providers" },
		{ DEVPKEY_Device_DependencyDependents, L"Depenedents" },
		{ DEVPKEY_Device_SoftRestartSupported, L"Soft Restart Supported" },
		{ DEVPKEY_Device_ExtendedAddress, L"Extended Address" },
		{ DEVPKEY_Device_AssignedToGuest, L"Assigned to Guest" },
		{ DEVPKEY_Device_CreatorProcessId, L"Creator Process ID" },
		{ DEVPKEY_Device_SessionId, L"Session ID" },
		{ DEVPKEY_Device_InstallDate, L"Install Date" },
		{ DEVPKEY_Device_FirstInstallDate, L"First Install Date" },
		{ DEVPKEY_Device_LastArrivalDate, L"Last Arrival Date" },
		{ DEVPKEY_Device_LastRemovalDate, L"Last Removal Date" },
		{ DEVPKEY_Device_DriverDate, L"Driver Date" },
		{ DEVPKEY_Device_DriverVersion, L"Driver Version" },
		{ DEVPKEY_Device_DriverDesc, L"Driver Description" },
		{ DEVPKEY_Device_DriverInfPath, L"Driver INF Path" },
		{ DEVPKEY_Device_DriverInfSection, L"Driver INF Section" },
		{ DEVPKEY_Device_DriverInfSectionExt, L"Driver INF Extended Section" },
		{ DEVPKEY_Device_MatchingDeviceId, L"Machine Device ID" },
		{ DEVPKEY_Device_DriverProvider, L"Driver Provider" },
		{ DEVPKEY_Device_DriverPropPageProvider, L"Driver Property Page Provider" },
		{ DEVPKEY_Device_DriverCoInstallers, L"Driver CoInstallers" },
		{ DEVPKEY_Device_ResourcePickerTags, L"Resource Picker Tags" },
		{ DEVPKEY_Device_ResourcePickerExceptions, L"Resource Picker Exceptions" },
		{ DEVPKEY_Device_DriverRank, L"Driver Rank" },
		{ DEVPKEY_Device_DriverLogoLevel, L"Driver Logo Level" },
		{ DEVPKEY_Device_NoConnectSound, L"No Connect Sound" },
		{ DEVPKEY_Device_GenericDriverInstalled, L"Generic Driver Installed" },
		{ DEVPKEY_Device_AdditionalSoftwareRequested, L"Additional Software Requested" },
		{ DEVPKEY_Device_SafeRemovalRequired, L"Safe Removal Required" },
		{ DEVPKEY_Device_SafeRemovalRequiredOverride, L"Safe Removal Override Required" },

		{ DEVPKEY_DrvPkg_Model, L"Package Model" },
		{ DEVPKEY_DrvPkg_VendorWebSite, L"Vendor Website" },
		{ DEVPKEY_DrvPkg_DetailedDescription, L"Detailed Description" },
		{ DEVPKEY_DrvPkg_DocumentationLink, L"Documentation Link" },
		{ DEVPKEY_DrvPkg_Icon, L"Package Icon" },
		{ DEVPKEY_DrvPkg_BrandingIcon, L"Branding Icon" },

		{ DEVPKEY_DeviceClass_UpperFilters, L"Class Upper Filters" },
		{ DEVPKEY_DeviceClass_LowerFilters, L"Class Lower Filters" },
		{ DEVPKEY_DeviceClass_Security, L"Class Security" },
		{ DEVPKEY_DeviceClass_SecuritySDS, L"Class Security SDDL" },
		{ DEVPKEY_DeviceClass_DevType, L"Class Device Type" },
		{ DEVPKEY_DeviceClass_Exclusive, L"Class Exclusive" },
		{ DEVPKEY_DeviceClass_Characteristics, L"Class Characteristics" },
		{ DEVPKEY_DeviceClass_Name, L"Class Name" },
		{ DEVPKEY_DeviceClass_ClassName, L"Class Name" },
		{ DEVPKEY_DeviceClass_ClassInstaller, L"Class Installer" },

		{ DEVPKEY_DeviceInterface_FriendlyName, L"Device Interface Name" },
		{ DEVPKEY_DeviceInterface_Enabled, L"Device Interface Enabled" },

		{ DEVPKEY_DeviceContainer_FriendlyName, L"Container Name" },
		{ DEVPKEY_DeviceContainer_Manufacturer, L"Container Manufacturer" },
		{ DEVPKEY_DeviceContainer_ModelName, L"Container Model Name" },
		{ DEVPKEY_DeviceContainer_ModelNumber, L"Container Model Number" },

		{ DEVPKEY_PciDevice_DeviceType, L"PCI Device Type" },
		{ DEVPKEY_PciDevice_BaseClass, L"PCI Base Class" },
		{ DEVPKEY_PciDevice_SubClass, L"PCI Subclass" },
		{ DEVPKEY_PciDevice_ProgIf, L"PCI ProgIf" },
		{ DEVPKEY_PciDevice_CurrentPayloadSize, L"PCI Current Payload Size" },
		{ DEVPKEY_PciDevice_MaxPayloadSize, L"PCI Max Payload Size" },
		{ DEVPKEY_PciDevice_MaxReadRequestSize, L"PCI Max Read Request Size" },
		{ DEVPKEY_PciDevice_CurrentLinkSpeed, L"PCI Current Link Speed" },
		{ DEVPKEY_PciDevice_CurrentLinkWidth, L"PCI Current Link Width" },
		{ DEVPKEY_PciDevice_MaxLinkSpeed, L"PCI Max Link Speed" },
		{ DEVPKEY_PciDevice_MaxLinkWidth, L"PCI Max Link Width" },
		{ DEVPKEY_PciDevice_ExpressSpecVersion, L"PCIe Spec Version" },
		{ DEVPKEY_PciDevice_InterruptSupport, L"PCI Interrupt Support" },
		{ DEVPKEY_PciDevice_InterruptMessageMaximum, L"PCI Max Message Interrupts" },
		{ DEVPKEY_PciDevice_BarTypes, L"PCI Bar Types" },
		{ DEVPKEY_PciDevice_FirmwareErrorHandling, L"PCI Firmware Error Handling" },
		{ DEVPKEY_PciDevice_Uncorrectable_Error_Mask, L"PCI Uncorrectable Error Mask" },
		{ DEVPKEY_PciDevice_Uncorrectable_Error_Severity, L"PCI Uncorrectable Error Sevirity" },
		{ DEVPKEY_PciDevice_Correctable_Error_Mask, L"PCI Correctable Error Mask" },
		{ DEVPKEY_PciDevice_ECRC_Errors, L"PCI ECRC Errors" },
		{ DEVPKEY_PciDevice_Error_Reporting, L"PCI Error Reporting" },
		{ DEVPKEY_PciDevice_RootError_Reporting, L"PCI Root Error Reporting" },
		{ DEVPKEY_PciDevice_S0WakeupSupported, L"PCI S0 Wakeup Supported" },
		{ DEVPKEY_PciDevice_SriovSupport, L"PCI SRI-OV Support" },
		{ DEVPKEY_PciDevice_Label_Id, L"PCI Label ID" },
		{ DEVPKEY_PciDevice_Label_String, L"PCI Label String" },
		{ DEVPKEY_PciDevice_AcsSupport, L"PCI ACS Support" },
		{ DEVPKEY_PciDevice_AriSupport, L"PCI ARI Support" },
		{ DEVPKEY_PciDevice_AcsCompatibleUpHierarchy, L"PCI ACS Compatible Up Hierarchy" },
		{ DEVPKEY_PciDevice_AcsCapabilityRegister, L"PCI ACS Capability Register" },
		{ DEVPKEY_PciDevice_AtsSupport, L"PCI ATS Support" },
		{ DEVPKEY_PciDevice_RequiresReservedMemoryRegion, L"PCI Requires Reserved Memory Region" },
		{ DEVPKEY_PciDevice_AtomicsSupported, L"PCI Atomics Supported" },
		{ DEVPKEY_PciDevice_SupportedLinkSubState, L"PCI Supported Link Substate" },
		{ DEVPKEY_PciDevice_OnPostPath, L"PCI On POST Path" },
		{ DEVPKEY_PciDevice_D3ColdSupport, L"PCI D3 Cold Support" },

		{ DEVPKEY_PciRootBus_SupportedSpeedsAndModes, L"PCI Supported Speeds and Modes" },
		{ DEVPKEY_PciRootBus_DeviceIDMessagingCapable, L"PCI Device ID Message Capable" },
		{ DEVPKEY_PciRootBus_SecondaryBusWidth, L"PCI Secondary Bus Width" },
		{ DEVPKEY_PciRootBus_ExtendedConfigAvailable, L"PCI Extended Config Available" },
		{ DEVPKEY_PciRootBus_ExtendedPCIConfigOpRegionSupport, L"PCI Extended Config OP Region Support" },
		{ DEVPKEY_PciRootBus_ASPMSupport, L"PCI ASPM Support" },
		{ DEVPKEY_PciRootBus_ClockPowerManagementSupport, L"PCI Clock Power Management Support" },
		{ DEVPKEY_PciRootBus_PCISegmentGroupsSupport, L"PCI Segment Groups Support" },
		{ DEVPKEY_PciRootBus_MSISupport, L"PCI MSI Support" },
		{ DEVPKEY_PciRootBus_PCIExpressNativeHotPlugControl, L"PCIe Native Hotplug Control" },
		{ DEVPKEY_PciRootBus_SHPCNativeHotPlugControl, L"PCI Native SHPC Hotplug Control" },

		{ *(DEVPROPKEY*)&PKEY_PNPX_ID, L"PNP-X ID" },
		{ *(DEVPROPKEY*)&PKEY_PNPX_FirmwareVersion, L"PNP-X Firmware Version" },
		{ *(DEVPROPKEY*)&PKEY_PNPX_SerialNumber, L"PNP-X Serial Number" },
		{ *(DEVPROPKEY*)&PKEY_PNPX_GlobalIdentity, L"PNP-X Global Identity" },
		{ *(DEVPROPKEY*)&PKEY_PNPX_Types, L"PNP-X Types" },
		{ *(DEVPROPKEY*)&PKEY_PNPX_RemoteAddress, L"PNP-X Remote Address" },
		{ *(DEVPROPKEY*)&PKEY_PNPX_RootProxy, L"PNP-X Root Proxy" },
	};

	if (auto it = properties.find(key); it != properties.end())
		return it->second;

	return GuidToString(key.fmtid) + L"[" + std::to_wstring(key.pid).c_str() + L"]";
}

CString Helpers::GuidToString(GUID const& guid) {
	WCHAR sguid[64];
	auto hr = ::StringFromGUID2(guid, sguid, _countof(sguid));
	return SUCCEEDED(hr) ? sguid : L"";
}

CString Helpers::GetPropertyValueAsString(DEVPROPKEY const& key, DeviceNode const& node, PCWSTR sep) {
	DEVPROPTYPE type;
	ULONG size;
	auto value = node.GetPropertyValue(key, type, &size);
	if (value == nullptr)
		return L"";

	return GetPropertyValueAsString(value.get(), type, size, sep);
}

CString Helpers::GetPropertyValueAsString(PBYTE value, DEVPROPTYPE type, ULONG size, PCWSTR sep) {
	CString text;
	switch (type) {
		case DEVPROP_TYPE_NULL:
			return L"";

		case DEVPROP_TYPE_SBYTE:
			text.Format(L"%d (0x%X)", (int32_t)value[0], (int32_t)value[0]);
			break;

		case DEVPROP_TYPE_BYTE:
			text.Format(L"%u (0x%X)", (uint32_t)value[0], (uint32_t)value[0]);
			break;

		case DEVPROP_TYPE_INT16:
			text.Format(L"%d (0x%X)", (int32_t) * (short*)value, (int32_t) * (short*)value);
			break;

		case DEVPROP_TYPE_INT32:
		case DEVPROP_TYPE_NTSTATUS:
			text.Format(L"%d (0x%X)", *(int32_t*)value, *(int32_t*)value);
			break;

		case DEVPROP_TYPE_INT64:
		case DEVPROP_TYPE_CURRENCY:
			text.Format(L"%lld (0x%llX)", *(int64_t*)value, *(int64_t*)value);
			break;

		case DEVPROP_TYPE_UINT64:
			text.Format(L"%llu (0x%llX)", *(uint64_t*)value, *(uint64_t*)value);
			break;

		case DEVPROP_TYPE_UINT32:
		case DEVPROP_TYPE_ERROR:
			text.Format(L"%u (0x%08X)", *(int32_t*)value, *(int32_t*)value);
			break;

		case DEVPROP_TYPE_BOOLEAN:
			return value[0] ? L"Yes" : L"No";

		case DEVPROP_TYPE_GUID:
			return GuidToString(*(GUID*)value);

		case DEVPROP_TYPE_FILETIME:
			auto ft = *(FILETIME*)value;
			if (ft.dwHighDateTime == 0 && ft.dwLowDateTime == 0)
				return L"";

			::SHFormatDateTime(&ft, nullptr, text.GetBufferSetLength(64), 64);
			break;

		case DEVPROP_TYPE_DATE:
		{
			CComVariant v(*(DATE*)value);
			v.ChangeType(VT_BSTR);
			return v.bstrVal;
		}

		case DEVPROP_TYPE_STRING:
		case DEVPROP_TYPE_SECURITY_DESCRIPTOR_STRING:
			return (PCWSTR)value;

		case DEVPROP_TYPE_STRING_LIST:
		{
			auto p = (PCWSTR)value;
			while (*p) {
				text += p;
				text += sep;
				p += wcslen(p) + 1;
			}
			if (!text.IsEmpty())
				text = text.Left(text.GetLength() - (int)wcslen(sep));
			break;
		}

		case DEVPROP_TYPE_SECURITY_DESCRIPTOR:
		case DEVPROP_TYPE_BINARY:
			return FormatBytes(value, std::min(size, (ULONG)64));

		case DEVPROP_TYPE_FLOAT:
			text.Format(L"%.2f", *(float*)value);
			break;

		case DEVPROP_TYPE_DOUBLE:
			text.Format(L"%.2lf", *(double*)value);
			break;

		case DEVPROP_TYPE_STRING_INDIRECT:
			::SHLoadIndirectString((PCWSTR)value, text.GetBufferSetLength(512), 512, nullptr);
			break;

	}
	return text;
}

CString Helpers::FormatBytes(const PBYTE buffer, ULONG size) {
	CString text, temp;
	for (ULONG i = 0; i < size; i++) {
		temp.Format(L"%02X ", buffer[i]);
		text += temp;
	}
	return text;
}

CString Helpers::GetPropertyDetails(DEVPROPKEY const& key, PBYTE value, ULONG size) {
	if (key == DEVPKEY_Device_Security) {
		PWSTR sddl;
		::ConvertSecurityDescriptorToStringSecurityDescriptor((PSECURITY_DESCRIPTOR)value,
			SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &sddl, nullptr);
		CString text(sddl);
		::LocalFree(sddl);
		return text;
	}
	if (key == DEVPKEY_Device_InstallState)
		return InstallStateToString(*(ULONG*)value);
	if (key == DEVPKEY_Device_RemovalPolicy || key == DEVPKEY_Device_RemovalPolicyOverride || key == DEVPKEY_Device_RemovalPolicyDefault)
		return RemovalPolicyToString(*(ULONG*)value);
	if (key == DEVPKEY_Device_DevNodeStatus)
		return DevNodeStatusToString(*(DeviceNodeStatus*)value);
	if (key == DEVPKEY_PciDevice_DeviceType)
		return PciDeviceTypeToString(*(ULONG*)value);

	return L"";
}

PCWSTR Helpers::InstallStateToString(ULONG state) {
	switch (state) {
		case 0: return L"Installed";
		case 1: return L"Need Reinstall";
		case 2: return L"Failed Install";
		case 3: return L"Finish Install";
	}
	return L"";
}

PCWSTR Helpers::RemovalPolicyToString(ULONG policy) {
	switch (policy) {
		case CM_REMOVAL_POLICY_EXPECT_NO_REMOVAL: return L"None";
		case CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL: return L"Orderly";
		case CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL: return L"Surprise";
	}
	return L"(Unknown)";
}

CString Helpers::DevNodeStatusToString(DeviceNodeStatus status, PCWSTR sep) {
	static const struct {
		DeviceNodeStatus Status;
		PCWSTR Text;
	} st[] = {
		{ DeviceNodeStatus::RootEnumerated, L"Root Enumerated" },
		{ DeviceNodeStatus::DriverLoaded, L"Driver Loaded" },
		{ DeviceNodeStatus::EnumLoaded, L"Enum Loaded" },
		{ DeviceNodeStatus::Started, L"Started" },
		{ DeviceNodeStatus::Manual, L"Manual" },
		{ DeviceNodeStatus::NeedToEnum, L"Need to Enum" },
		{ DeviceNodeStatus::NotFirstTime, L"Not First Time" },
		{ DeviceNodeStatus::HardwareEnum, L"Hardware Enum" },
		{ DeviceNodeStatus::Liar, L"Liar" },
		{ DeviceNodeStatus::HasMark, L"Has Mark" },
		{ DeviceNodeStatus::HasProblem, L"Has Problem" },
		{ DeviceNodeStatus::Filtered, L"Filtered" },
		{ DeviceNodeStatus::Moved, L"Moved" },
		{ DeviceNodeStatus::Disableable, L"Disableable" },
		{ DeviceNodeStatus::Removable, L"Removable" },
		{ DeviceNodeStatus::PrivateProblem, L"Private Problem" },
		{ DeviceNodeStatus::MfParent, L"MF Parent" },
		{ DeviceNodeStatus::MfChild, L"MF Child" },
		{ DeviceNodeStatus::WillBeRmoved, L"Will Be Removed" },
		{ DeviceNodeStatus::NotFirstTimeEnum, L"Not First Time Enum" },
		{ DeviceNodeStatus::StopFreeResources, L"Stop Free Resources" },
		{ DeviceNodeStatus::RealCandidate, L"Real Candidate" },
		{ DeviceNodeStatus::BadPartial, L"Bad Prtial" },
		{ DeviceNodeStatus::NtEnumerator, L"NT Enumerator" },
		{ DeviceNodeStatus::NtDriver, L"NT Driver" },
		{ DeviceNodeStatus::NeedsLocking, L"Needs Locking" },
		{ DeviceNodeStatus::ArmWakeup, L"Arm Wakeup" },
		{ DeviceNodeStatus::ApmEnumerator, L"APM Enumerator" },
		{ DeviceNodeStatus::ApmDriver, L"APM Driver" },
		{ DeviceNodeStatus::SilentInstall, L"Silent Install" },
		{ DeviceNodeStatus::NoShowInDeviceManager, L"Don't Show" },
		{ DeviceNodeStatus::BootLogProblem, L"Boot Log Problem" },
	};

	CString text;
	for (auto& s : st) {
		if (status == DeviceNodeStatus::None)
			break;

		if ((status & s.Status) == s.Status)
			(text += s.Text) += sep;
		status &= ~s.Status;
	}

	if (!text.IsEmpty())
		text = text.Left(text.GetLength() - (int)wcslen(sep));
	return text;
}

PCWSTR Helpers::PciDeviceTypeToString(ULONG type) {
	static PCWSTR types[] = {
		L"PCI Conventional",
		L"PCI-X",
		L"PCI Express Endpoint",
		L"PCI Express Legacy Endpoint",
		L"PCI Express Root Complex Integrated Endpoint",
		L"PCI Express Treated As PCI ",
		L"PCI Conventional (Bridge)",
		L"PCI-X (Bridge)",
		L"PCI Express Root Port (Bridge)",
		L"PCI Express Upstream Switch Port (Bridge)",
		L"PCI Express Downstream Switch Port (Bridge)",
		L"PCI Express To PCI XBridge",
		L"PCI-X To Express Bridge",
		L"PCI Express Treated As PCI (Bridge)",
		L"PCI Express Event Collector (Bridge)",
	};
	return types[type];
}
