#include "pch.h"
#include "GeneralPropertyPage.h"
#include "DeviceManager.h"
#include "Helpers.h"

CGeneralPropertyPage::CGeneralPropertyPage(DeviceManager const& dm, DeviceInfo const& di) 
	: CPropertyPageImpl(L"General"), m_dm(dm), m_di(di) {
}

LRESULT CGeneralPropertyPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	auto hIcon = m_dm.GetDeviceIcon(m_di, true);
	ATLASSERT(hIcon);
	((CStatic)GetDlgItem(IDC_IMAGE)).SetIcon(hIcon);

	SetDlgItemText(IDC_NAME, m_di.Description.c_str());

	DeviceNode node(m_di.Data.DevInst);

	SetDlgItemText(IDC_DEVICETYPE, m_dm.GetDeviceClassDescription(m_di.Data.ClassGuid).c_str());
	SetDlgItemText(IDC_MFG, m_dm.GetDeviceRegistryPropertyString(m_di, DeviceRegistryPropertyType::Mfg).c_str());
	auto location = m_dm.GetDeviceRegistryPropertyString(m_di, DeviceRegistryPropertyType::Location);
	if (location.empty()) {
		auto parent = node.GetProperty<std::wstring>(DEVPKEY_Device_Parent);
		auto dev = m_dm.FindDevice([&](auto& device) {
			return _wcsicmp(DeviceNode(device.Data.DevInst).GetProperty<std::wstring>(DEVPKEY_Device_InstanceId).c_str(), parent.c_str()) == 0;
			});
		if(dev)
			location = dev->Description;
	}
	SetDlgItemText(IDC_LOCATION, location.c_str());
	
	DeviceNodeProblem problem;
	node.GetStatus(&problem);
	if (problem != DeviceNodeProblem::None) {
		SetDlgItemText(IDC_STATUS, std::format(L"Error: {}", (int)problem).c_str());
	}
	else {
		SetDlgItemText(IDC_STATUS, L"The device is working properly.");
	}
	return 0;
}
