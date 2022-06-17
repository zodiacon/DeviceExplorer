#include "pch.h"
#include "DriversPropertyPage.h"

CDriversPropertyPage::CDriversPropertyPage(std::vector<DeviceDriverInfo> const& drivers) 
	: CPropertyPageImpl<CDriversPropertyPage>(L"Drivers"), m_Items(drivers) {
}

CString CDriversPropertyPage::GetColumnText(HWND, int row, int col) const {
	return CString();
}

int CDriversPropertyPage::GetRowImage(HWND, int row, int) const {
	return 0;
}

LRESULT CDriversPropertyPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"File Path", LVCFMT_LEFT | LVCFMT_FILL, 100);
	cm->UpdateColumns();

	return 0;
}
