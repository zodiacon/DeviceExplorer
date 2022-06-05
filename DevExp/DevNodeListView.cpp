#include "pch.h"
#include "DevNodeListView.h"
#include "Helpers.h"
#include "SortHelper.h"
#include "AppSettings.h"
#include "ListViewhelper.h"
#include "ClipboardHelper.h"
#include "SecurityHelper.h"

DWORD CDevNodeListView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CDevNodeListView::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	return CDRF_NOTIFYSUBITEMDRAW;
}

DWORD CDevNodeListView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lv = (NMLVCUSTOMDRAW*)cd;
	if (lv->iSubItem != 1)
		return CDRF_SKIPPOSTPAINT;

	auto& item = m_Items[(int)cd->dwItemSpec];
	if ((DeviceNode(item.Data.DevInst).GetStatus() & DeviceNodeStatus::HasProblem) == DeviceNodeStatus::HasProblem) {
		::DrawIconEx(cd->hdc, 12, cd->rc.top + 4, AtlLoadSysIcon(IDI_WARNING), 16, 16, 0, nullptr, DI_NORMAL);
	}

	return CDRF_SKIPPOSTPAINT;
}


void CDevNodeListView::Refresh() {
	bool first = m_Items.empty();
	m_Items = m_dm->EnumDevices<DeviceItem>(m_ShowHiddenDevices);
	m_List.GetImageList(LVSIL_SMALL).RemoveAll();
	if (first) {
		m_List.SetItemCount((int)m_Items.size());
	}
	else {
		m_List.SetItemCountEx((int)m_Items.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
		DoSort(GetSortInfo(m_List));
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}
}

CString CDevNodeListView::GetColumnText(HWND, int row, int col) {
	auto& item = m_Items[row];
	switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(col)) {
		case ColumnType::Name: return item.Description.c_str();
		case ColumnType::Instance: return std::to_wstring(item.Data.DevInst).c_str();
		case ColumnType::Class:
			return GetDeviceClassName(item);
		case ColumnType::Status:
			return Helpers::DevNodeStatusToString(DeviceNode(item.Data.DevInst).GetStatus());
		case ColumnType::Driver:
			return GetStringProperty(item, DEVPKEY_Device_Driver);
		case ColumnType::PDOName:
			return GetStringProperty(item, DEVPKEY_Device_PDOName);
		case ColumnType::Parent:
			return GetStringProperty(item, DEVPKEY_Device_Parent);
		case ColumnType::Enumerator:
			return GetStringProperty(item, DEVPKEY_Device_EnumeratorName);
		case ColumnType::Depth: return std::to_wstring(DeviceNode(item.Data.DevInst).GetDepth()).c_str();
	}
	return L"";
}

bool CDevNodeListView::OnRightClickList(HWND, int row, int col, POINT const& pt) {
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	return GetFrame()->TrackPopupMenu(menu.GetSubMenu(0), TPM_RIGHTBUTTON, pt.x, pt.y);
}

bool CDevNodeListView::OnDoubleClickList(HWND, int row, int col, POINT const&) {
	LRESULT result;
	ProcessWindowMessage(m_hWnd, WM_COMMAND, ID_DEVICE_PROPERTIES, 0, result, 1);
	return true;
}

LRESULT CDevNodeListView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		| LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 64, 32);
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 250, ColumnType::Name);
	cm->AddColumn(L"Instance", LVCFMT_RIGHT, 80, ColumnType::Instance);
	cm->AddColumn(L"Class", LVCFMT_LEFT, 140, ColumnType::Class);
	cm->AddColumn(L"Depth", LVCFMT_RIGHT, 60, ColumnType::Depth);
	cm->AddColumn(L"Status", LVCFMT_LEFT, 240, ColumnType::Status);
	cm->AddColumn(L"PDO Name", LVCFMT_LEFT, 160, ColumnType::PDOName);
	cm->AddColumn(L"Enumerator", LVCFMT_LEFT, 120, ColumnType::Enumerator);
	cm->AddColumn(L"Parent", LVCFMT_LEFT, 220, ColumnType::Parent);

	m_dm = DeviceManager::Create();

	Refresh();

	return 0;
}

LRESULT CDevNodeListView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.SetFocus();

	return 0;
}

CString CDevNodeListView::GetStringProperty(DeviceItem& item, DEVPROPKEY const& key) {
	return DeviceNode(item.Data.DevInst).GetProperty<std::wstring>(key).c_str();
}

CString const& CDevNodeListView::GetDeviceClassName(DeviceItem& item) {
	if (item.Class.IsEmpty()) {
		item.Class = DeviceManager::GetDeviceClassDescription(item.Data.ClassGuid).c_str();
		if (item.Class.IsEmpty())
			item.Class = DeviceNode(item.Data.DevInst).GetProperty<std::wstring>(DEVPKEY_Device_Class).c_str();
	}
	return item.Class;
}

int CDevNodeListView::GetRowImage(HWND, int row, int) {
	auto& item = m_Items[row];
	if (item.Image >= 0)
		return item.Image;
	auto icon = m_dm->GetDeviceIcon(item);
	if (icon) {
		item.Image = m_List.GetImageList(LVSIL_SMALL).AddIcon(icon);
		::DestroyIcon(icon);
	}
	return item.Image;
}

void CDevNodeListView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	auto compare = [&](auto& d1, auto& d2) {
		switch (GetColumnManager(m_List)->GetColumnTag<ColumnType>(si->SortColumn)) {
			case ColumnType::Name: return SortHelper::Sort(d1.Description, d2.Description, si->SortAscending);
			case ColumnType::Class: return SortHelper::Sort(GetDeviceClassName(d1), GetDeviceClassName(d2), si->SortAscending);
			case ColumnType::Instance: return SortHelper::Sort(d1.Data.DevInst, d2.Data.DevInst, si->SortAscending);
			case ColumnType::Status: return SortHelper::Sort(DeviceNode(d1.Data.DevInst).GetStatus(), DeviceNode(d2.Data.DevInst).GetStatus(), si->SortAscending);
			case ColumnType::Enumerator: return SortHelper::Sort(GetStringProperty(d1, DEVPKEY_Device_EnumeratorName), GetStringProperty(d2, DEVPKEY_Device_EnumeratorName), si->SortAscending);
			case ColumnType::PDOName: return SortHelper::Sort(GetStringProperty(d1, DEVPKEY_Device_PDOName), GetStringProperty(d2, DEVPKEY_Device_PDOName), si->SortAscending);
			case ColumnType::Parent: return SortHelper::Sort(GetStringProperty(d1, DEVPKEY_Device_Parent), GetStringProperty(d2, DEVPKEY_Device_Parent), si->SortAscending);
			case ColumnType::Depth: return SortHelper::Sort(DeviceNode(d1.Data.DevInst).GetDepth(), DeviceNode(d2.Data.DevInst).GetDepth(), si->SortAscending);
		}
		return false;
	};

	std::sort(m_Items.begin(), m_Items.end(), compare);
}

LRESULT CDevNodeListView::OnShowHiddenDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_ShowHiddenDevices = !m_ShowHiddenDevices;
	GetFrame()->GetUI().UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);

	Refresh();
	return 0;
}

LRESULT CDevNodeListView::OnViewRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CDevNodeListView::OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CString text;
	for (auto i = m_List.GetNextItem(-1, LVIS_SELECTED); i >= 0; i = m_List.GetNextItem(i, LVIS_SELECTED)) {
		text += ListViewHelper::GetRowAsString(m_List, i);
		text += L"\n";
	}
	ClipboardHelper::CopyText(m_hWnd, text.Left(text.GetLength() - 1));
	return 0;
}

void CDevNodeListView::UpdateUI(CUpdateUIBase& ui) {
	ui.UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);
	int selected = m_List.GetSelectionMark();
	ui.UIEnable(ID_DEVICE_PROPERTIES, m_List.GetSelectedCount() == 1);

	if (SecurityHelper::IsRunningElevated() && m_List.GetSelectedCount() == 1) {
		DeviceNode dn(m_Items[selected].Data.DevInst);
		bool enabled = dn.IsEnabled();
		ui.UIEnable(ID_DEVICE_ENABLE, !enabled);
		ui.UIEnable(ID_DEVICE_DISABLE, enabled);
		ui.UIEnable(ID_DEVICE_UNINSTALL, true);
	}
	else {
		ui.UIEnable(ID_DEVICE_ENABLE, false);
		ui.UIEnable(ID_DEVICE_DISABLE, false);
		ui.UIEnable(ID_DEVICE_UNINSTALL, false);
	}
}

void CDevNodeListView::OnPageActivated(bool active) {
	if (active && IsRefreshNeeded())
		Refresh();
}

LRESULT CDevNodeListView::OnEnableDisableDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto& item = m_Items[m_List.GetSelectionMark()];
	DeviceNode dn(item.Data.DevInst);
	auto result = dn.IsEnabled() ? dn.Disable() : dn.Enable();
	return 0;
}

LRESULT CDevNodeListView::OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	UpdateUI(GetFrame()->GetUI());
	return 0;
}

LRESULT CDevNodeListView::OnDeviceProperties(WORD, WORD, HWND, BOOL&) {
	auto& item = m_Items[m_List.GetSelectionMark()];
	Helpers::DisplayProperties(item.Description.c_str(), *m_dm, item);
	return 0;
}

