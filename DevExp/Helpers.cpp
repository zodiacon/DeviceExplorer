#include "pch.h"
#include "Helpers.h"
#include <sddl.h>
#include <wiaintfc.h>
#include <Ntddmou.h>
#include <Ntddkbd.h>
#include <ntddvdeo.h>
#include <ntddcdvd.h>
#include <ndisguid.h>
#include <usbiodef.h>
#include <Ntddpar.h>
#include <hidclass.h>
#include <ntddmodm.h>
#include <Portabledevice.h>
#include <ioevent.h>
#include <bthdef.h>
#include "DeviceManager.h"
#include "MultiStringListDlg.h"
#include "GeneralPropertyPage.h"
#include "ResourcesPropertyPage.h"
#include "DriversPropertyPage.h"

namespace std {
	template<>
	struct hash<DEVPROPKEY> {
		size_t operator()(DEVPROPKEY const& p) const {
			return p.pid ^ p.fmtid.Data1 ^ (p.fmtid.Data2 << 16);
		}
	};
}

#pragma region From WDK

DEFINE_GUID(GUID_DEVINTERFACE_BRIGHTNESS, 0xFDE5BBA4, 0xB3F9, 0x46FB, 0xBD, 0xAA, 0x07, 0x28, 0xCE, 0x31, 0x00, 0xB4);
DEFINE_GUID(GUID_DEVINTERFACE_I2C, 0x2564AA4F, 0xDDDB, 0x4495, 0xB4, 0x97, 0x6A, 0xD4, 0xA8, 0x41, 0x63, 0xD7);
DEFINE_GUID(GUID_VIRTUAL_AVC_CLASS, 0x616ef4d0, 0x23ce, 0x446d, 0xa5, 0x68, 0xc3, 0x1e, 0xb0, 0x19, 0x13, 0xd0);
DEFINE_GUID(GUID_AVC_CLASS, 0x095780c3, 0x48a1, 0x4570, 0xbd, 0x95, 0x46, 0x70, 0x7f, 0x78, 0xc2, 0xdc);
DEFINE_GUID(GUID_61883_CLASS, 0x7ebefbc0, 0x3200, 0x11d2, 0xb4, 0xc2, 0x0, 0xa0, 0xc9, 0x69, 0x7d, 0x7);
DEFINE_GUID(BUS1394_CLASS_GUID, 0x6BDD1FC1, 0x810F, 0x11d0, 0xBE, 0xC7, 0x08, 0x00, 0x2B, 0xE2, 0x09, 0x2F);

#pragma endregion

DEFINE_DEVPROPKEY(DEVPKEY_Device_IsConnected, 0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 15);
DEFINE_DEVPROPKEY(DEVPKEY_Device_DriverNodeStrongName, 0x83da6326, 0x97a6, 0x4088, 0x94, 0x53, 0xa1, 0x92, 0x3f, 0x57, 0x3b, 0x29, 3);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_Inbox, 0x6a3433f4, 0x5626, 0x40e8, 0xa9, 0xb9, 0xdb, 0xd9, 0xec, 0xd2, 0x88, 0x4b, 6);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_Configurable, 0x6a3433f4, 0x5626, 0x40e8, 0xa9, 0xb9, 0xdb, 0xd9, 0xec, 0xd2, 0x88, 0x4b, 4);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_BootCritical, 0x6a3433f4, 0x5626, 0x40e8, 0xa9, 0xb9, 0xdb, 0xd9, 0xec, 0xd2, 0x88, 0x4b, 3);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_FSFilterClass, 0x259abffc, 0x50a7, 0x47ce, 0xaf, 0x8, 0x68, 0xc9, 0xa7, 0xd7, 0x33, 0x66, 15);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_CompoundUpperFilters, 0x6a3433f4, 0x5626, 0x40e8, 0xa9, 0xb9, 0xdb, 0xd9, 0xec, 0xd2, 0x88, 0x4b, 20);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_CompoundLowerFilters, 0x6a3433f4, 0x5626, 0x40e8, 0xa9, 0xb9, 0xdb, 0xd9, 0xec, 0xd2, 0x88, 0x4b, 21);
DEFINE_DEVPROPKEY(DEVPKEY_DeviceClass_LowerLogoVersion, 0x259abffc, 0x50a7, 0x47ce, 0xaf, 0x8, 0x68, 0xc9, 0xa7, 0xd7, 0x33, 0x66, 13);

DEFINE_GUID(GUID_DEVINTERFACE_SMARTCARD_READER,
	0x50DD5230, 0xBA8A, 0x11D1, 0xBF, 0x5D, 0x00, 0x00, 0xF8, 0x05, 0xF5, 0x30);

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
		{ DEVPKEY_Device_DependencyDependents, L"Dependents" },
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
		{ DEVPKEY_Device_IsConnected, L"Is Connected" },
		{ DEVPKEY_Device_DriverNodeStrongName, L"Strong Name" },

		{ DEVPKEY_DrvPkg_Model, L"Package Model" },
		{ DEVPKEY_DrvPkg_VendorWebSite, L"Vendor Website" },
		{ DEVPKEY_DrvPkg_DetailedDescription, L"Detailed Description" },
		{ DEVPKEY_DrvPkg_DocumentationLink, L"Documentation Link" },
		{ DEVPKEY_DrvPkg_Icon, L"Package Icon" },
		{ DEVPKEY_DrvPkg_BrandingIcon, L"Branding Icon" },

		{ DEVPKEY_DeviceClass_UpperFilters, L"Upper Filters" },
		{ DEVPKEY_DeviceClass_LowerFilters, L"Lower Filters" },
		{ DEVPKEY_DeviceClass_Security, L"Security" },
		{ DEVPKEY_DeviceClass_SecuritySDS, L"Security SDDL" },
		{ DEVPKEY_DeviceClass_DevType, L"Device Type" },
		{ DEVPKEY_DeviceClass_Exclusive, L"Exclusive" },
		{ DEVPKEY_DeviceClass_Characteristics, L"Characteristics" },
		{ DEVPKEY_DeviceClass_Name, L"Description" },
		{ DEVPKEY_DeviceClass_ClassName, L"Class Name" },
		{ DEVPKEY_DeviceClass_ClassInstaller, L"Class Installer" },
		{ DEVPKEY_DeviceClass_ClassCoInstallers, L"Class Co-Installers" },
		{ DEVPKEY_DeviceClass_Icon, L"Icon" },
		{ DEVPKEY_DeviceClass_NoDisplayClass, L"No Display Class" },
		{ DEVPKEY_DeviceClass_SilentInstall, L"Silent Install" },
		{ DEVPKEY_DeviceClass_NoUseClass, L"No Use Class" },
		{ DEVPKEY_DeviceClass_DefaultService, L"Default Service" },
		{ DEVPKEY_DeviceClass_PropPageProvider, L"Property Page Provider" },
		{ DEVPKEY_DeviceClass_IconPath, L"Icon Path" },
		{ DEVPKEY_DeviceClass_NoInstallClass, L"No Install Class" },
		{ DEVPKEY_DeviceClass_Inbox, L"Inbox" },
		{ DEVPKEY_DeviceClass_Configurable, L"Configurable" },
		{ DEVPKEY_DeviceClass_BootCritical, L"Boot Critical" },
		{ DEVPKEY_DeviceClass_FSFilterClass, L"FS Filter Class" },
		{ DEVPKEY_DeviceClass_CompoundUpperFilters, L"Compound Upper Filters" },
		{ DEVPKEY_DeviceClass_CompoundLowerFilters, L"Compound Lower Filters" },
		{ DEVPKEY_DeviceClass_NoUseClass, L"No Use Class" },
		{ DEVPKEY_DeviceClass_LowerLogoVersion, L"Lower Logo Version" },

		{ DEVPKEY_DeviceInterface_FriendlyName, L"Friendly Name" },
		{ DEVPKEY_DeviceInterface_Enabled, L"Enabled" },
		{ DEVPKEY_DeviceInterface_SchematicName, L"Semantic Name" },
		{ DEVPKEY_DeviceInterfaceClass_Name, L"Class Name" },
		{ DEVPKEY_DeviceInterfaceClass_DefaultInterface, L"Default Interface" },

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
		{ *(DEVPROPKEY*)&PKEY_DeviceInterface_DevicePath, L"Device Path" },
	};

	if (auto it = properties.find(key); it != properties.end())
		return it->second;

	return GuidToString(key.fmtid) + L"[" + std::to_wstring(key.pid).c_str() + L"]";
}

CString Helpers::GuidToString(GUID const& guid) {
	WCHAR sguid[68];
	auto hr = ::StringFromGUID2(guid, sguid, _countof(sguid));
	ATLASSERT(SUCCEEDED(hr));
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
	if (key == DEVPKEY_Device_Security || key == DEVPKEY_DeviceClass_Security) {
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
	if (key == DEVPKEY_Device_PowerData) {
		auto data = (CM_POWER_DATA*)value;
		CString text;
		text.Format(L"Power State: %s", DevicePowerStateToString(data->PD_MostRecentPowerState));
		return text;
	}
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
		{ DeviceNodeStatus::NoShowInDeviceManager, L"Hidden" },
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

PCWSTR Helpers::DevicePowerStateToString(DEVICE_POWER_STATE state) {
	static PCWSTR states[] = {
		L"(Unspecified)", L"D0", L"D1", L"D2", L"D3"
	};
	return states[state];
}

CString Helpers::DeviceInterfaceToString(GUID const& guid) {
	static const std::unordered_map<GUID, PCWSTR> map = {
		{ GUID_DEVICE_BATTERY, L"Battery" },
		{ GUID_DEVICE_APPLICATIONLAUNCH_BUTTON, L"App Launch Button" },
		{ GUID_DEVINTERFACE_CDROM, L"CD-ROM" },
		{ GUID_DEVINTERFACE_DISK, L"Disk" },
		{ GUID_DEVINTERFACE_PARTITION, L"Partition" },
		{ GUID_DEVINTERFACE_VOLUME, L"Volume" },
		{ GUID_DEVINTERFACE_IMAGE, L"Image" },
		{ GUID_DEVINTERFACE_KEYBOARD, L"Keyboard" },
		{ GUID_DEVINTERFACE_MOUSE, L"Mouse" },
		{ GUID_DEVINTERFACE_MONITOR, L"Monitor" },
		{ GUID_DEVINTERFACE_SMARTCARD_READER, L"Smart Card Reader" },
		{ GUID_DEVINTERFACE_NET, L"Net" },
		{ GUID_DEVINTERFACE_USB_DEVICE, L"USB Device" },
		{ GUID_DEVINTERFACE_USB_HOST_CONTROLLER, L"USB Host Controller" },
		{ GUID_DEVINTERFACE_USB_HUB, L"USB Hub" },
		{ GUID_DEVINTERFACE_TAPE, L"Tape" },
		{ GUID_DEVINTERFACE_COMPORT, L"COM Port" },
		{ GUID_DEVINTERFACE_CDCHANGER, L"CD Changer" },
		{ GUID_DEVINTERFACE_DISPLAY_ADAPTER, L"Display Adapter" },
		{ GUID_DEVINTERFACE_HIDDEN_VOLUME, L"Hidden Volume" },
		{ GUID_DEVINTERFACE_NETUIO, L"Net UIO" },
		{ GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, L"Serial Bus Enumerator" },
		{ GUID_DEVINTERFACE_STORAGEPORT, L"Storage Port" },
		{ GUID_DEVINTERFACE_PARCLASS, L"Parallel Port" },
		{ GUID_DEVINTERFACE_HID, L"Human Interface Device" },
		{ GUID_DEVINTERFACE_BRIGHTNESS, L"Brightness" },
		{ GUID_DEVINTERFACE_I2C, L"WDDM Display Adapter" },
		{ GUID_DEVICE_PROCESSOR, L"Processor" },
		{ GUID_DEVICE_MEMORY, L"Memory" },
		{ GUID_DEVICE_LID, L"Lid" },
		{ GUID_DEVINTERFACE_MODEM, L"Modem" },
		{ GUID_DEVINTERFACE_WPD, L"Windows Portable Devices" },
		{ GUID_DEVINTERFACE_WPD_PRIVATE, L"Private Windows Portable Devices" },
		{ GUID_DEVINTERFACE_VIDEO_OUTPUT_ARRIVAL, L"Display Device Children" },
		{ GUID_DEVINTERFACE_WRITEONCEDISK, L"Write Once Disks" },
		{ GUID_AVC_CLASS, L"Audio/Video Control" },
		{ GUID_VIRTUAL_AVC_CLASS, L"Virtual Audio/Video Control" },
		{ GUID_IO_VOLUME_DEVICE_INTERFACE, L"Volume" },
		{ GUID_BTHPORT_DEVICE_INTERFACE, L"Bluetooth Radio" },
		{ GUID_61883_CLASS, L"61883" },
		{ BUS1394_CLASS_GUID, L"Firewire (1394)" },
	};

	if (auto it = map.find(guid); it != map.end())
		return it->second;

	auto name = DeviceManager::GetDeviceInterfaceProperty<std::wstring>(guid, DEVPKEY_NAME);
	if(name.empty())
		name = DeviceManager::GetDeviceInterfaceProperty<std::wstring>(guid, DEVPKEY_DeviceInterfaceClass_Name);
	if (name.empty())
		name = DeviceManager::GetDeviceInterfaceProperty<std::wstring>(guid, DEVPKEY_DeviceInterface_FriendlyName);
	if (name.empty())
		name = DeviceManager::GetDeviceInterfaceProperty<std::wstring>(guid, DEVPKEY_DeviceClass_ClassName);

	if (!name.empty())
		return name.c_str();

	return GuidToString(guid);
}

bool Helpers::DisplayProperty(DEVPROPKEY const& key, DeviceNode const& node, PCWSTR name) {
	if (key == DEVPKEY_Device_PowerData) {
		DEVPROPTYPE type;
		ULONG len;
		auto value = node.GetPropertyValue(key, type, &len);
		auto data = (CM_POWER_DATA*)value.get();
		CMultiStringListDlg dlg(name);
		std::vector<std::wstring> text;
		text.emplace_back(std::format(L"Power State: {}", DevicePowerStateToString(data->PD_MostRecentPowerState)));
		text.push_back({});
		text.emplace_back(std::format(L"Capabilities: 0x{:X} ({})", data->PD_Capabilities, PowerCapabilitiesToString(data->PD_Capabilities)));
		if(data->PD_Capabilities & PDCAP_WAKE_FROM_D1_SUPPORTED)
			text.emplace_back(std::format(L"D1 Latency: {}", data->PD_D1Latency));
		if (data->PD_Capabilities & PDCAP_WAKE_FROM_D2_SUPPORTED)
			text.emplace_back(std::format(L"D2 Latency: {}", data->PD_D2Latency));
		if (data->PD_Capabilities & PDCAP_WAKE_FROM_D3_SUPPORTED)
			text.emplace_back(std::format(L"D3 Latency: {}", data->PD_D3Latency));
		text.push_back({});
		text.emplace_back(L"Power state mappings:");
		for (int i = 1; i < POWER_SYSTEM_MAXIMUM; i++) {
			if (data->PD_PowerStateMapping[i] != PowerDeviceUnspecified)
				text.emplace_back(std::format(L"S{} -> D{}", i - 1, (int)data->PD_PowerStateMapping[i] - 1));
		}
		dlg.SetData(text);
		dlg.DoModal();
		return true;
	}

	auto type = node.GetPropertyType(key);
	switch(type) {
		case DEVPROP_TYPE_STRING_LIST:
			CMultiStringListDlg dlg(name);
			dlg.SetData(node.GetProperty<std::vector<std::wstring>>(key));
			dlg.DoModal();
			return true;
	}
	return false;
}

std::wstring Helpers::PowerCapabilitiesToString(DWORD caps) {
	static const struct {
		DWORD cap;
		PCWSTR text;
	} capabilities[] = {
		{ PDCAP_D0_SUPPORTED, L"D0" },
		{ PDCAP_D1_SUPPORTED, L"D1" },
		{ PDCAP_D2_SUPPORTED, L"D2" },
		{ PDCAP_D3_SUPPORTED, L"D3" },
		{ PDCAP_WAKE_FROM_D0_SUPPORTED, L"Wake from D0" },
		{ PDCAP_WAKE_FROM_D1_SUPPORTED, L"Wake from D1" },
		{ PDCAP_WAKE_FROM_D2_SUPPORTED, L"Wake from D2" },
		{ PDCAP_WAKE_FROM_D3_SUPPORTED, L"Wake from D3" },
		{ PDCAP_WARM_EJECT_SUPPORTED, L"Warm Eject" },
	};

	std::wstring text;
	for (auto const& c : capabilities)
		if ((c.cap & caps) == c.cap)
			text += c.text + std::wstring(L", ");

	if (!text.empty())
		text = text.substr(0, text.length() - 2);
	return text;
}

void Helpers::DisplayProperties(PCWSTR title, DeviceManager const& dm, DeviceInfo const& di) {
	CPropertySheet sheet(title);
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP | PSH_NOMARGIN;
	CGeneralPropertyPage general(dm, di);
	sheet.AddPage(general);
	DeviceNode dn(di.Data.DevInst);
	auto resources = dn.GetResources();
	CResourcesPropertyPage res(resources);
	if (!resources.empty())
		sheet.AddPage(res);
	auto drivers = dm.EnumDrivers(di);
	CDriversPropertyPage driversPage(drivers);
	if (!drivers.empty())
		sheet.AddPage(driversPage);

	sheet.DoModal();
}

PCWSTR Helpers::ResourceTypeToString(ResourceType type) {
	switch (type & ResourceType::ClassSpecific) {
		case ResourceType::IO: return L"I/O";
		case ResourceType::Memory: return L"Memory";
		case ResourceType::Interrupt: return L"Interrupt";
		case ResourceType::DMA: return L"DMA";
		case ResourceType::LargeMemory: return L"Large Memory";
		case ResourceType::BusNumber: return L"Bus Number";
		case ResourceType::PCCardConfig: return L"PC Card Config";
		case ResourceType::Private: return L"Private";
		case ResourceType::MFCardConfig: return L"MF Card Config";
	}
	return L"(Unknown)";
}
