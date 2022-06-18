#include "pch.h"
#include "DriversPropertyPage.h"
#include "Helpers.h"

CDriversPropertyPage::CDriversPropertyPage(std::vector<DeviceDriverInfo> const& drivers) 
	: CPropertyPageImpl<CDriversPropertyPage>(L"Drivers"), m_Items(drivers) {
}

CString CDriversPropertyPage::GetColumnText(HWND, int row, int col) const {
	auto& item = m_Items[row];
	switch (col) {
		case 0: return Helpers::FormatDate(item.DriverDate).c_str();
		case 1: return item.Description.c_str();
	}
	return CString();
}

int CDriversPropertyPage::GetRowImage(HWND, int row, int) const {
	return 0;
}

void CDriversPropertyPage::OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState) {
	if (newState & LVIS_SELECTED) {
		auto& item = m_Items[from];
		SetDlgItemText(IDC_MFG, item.ManufactorName.c_str());
	}
}

LRESULT CDriversPropertyPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	m_List.SetItemCount((int)m_Items.size());

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Date", LVCFMT_LEFT, 100);
	cm->AddColumn(L"File Path", LVCFMT_LEFT | LVCFMT_FILL, 300);
	cm->UpdateColumns();

	return 0;
}
