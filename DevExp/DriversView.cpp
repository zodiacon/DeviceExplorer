#include "pch.h"
#include "DriversView.h"

LRESULT CDriversView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		| LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 64, 32);
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 250, ColumnType::Name);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 90, ColumnType::Type);
	cm->AddColumn(L"State", LVCFMT_LEFT, 140, ColumnType::State);
	cm->AddColumn(L"Image Path", LVCFMT_LEFT, 60, ColumnType::ImagePath);
	cm->AddColumn(L"WDF Version", LVCFMT_LEFT, 240, ColumnType::WDFVersion);

	Refresh();

	return 0;
}

LRESULT CDriversView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.SetFocus();

	return 0;
}

void CDriversView::Refresh() {
	m_Drivers.Set(DriverManager::EnumKernelDrivers());
	m_List.SetItemCount((int)m_Drivers.size());
}
