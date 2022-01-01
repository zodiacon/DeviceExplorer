#include "pch.h"
#include "DeviceInterfacesView.h"
#include "SortHelper.h"
#include "Helpers.h"

void CDeviceInterfacesView::Refresh() {
	m_Tree.SetRedraw(FALSE);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 32, 32);
	int computerIcon = images.AddIcon(AtlLoadIconImage(IDI_DEVICES, 0, 16, 16));
	m_Tree.SetImageList(images);

	m_Tree.DeleteAllItems();
	m_DevMgr = DeviceManager::Create();
	m_Devices = m_DevMgr->EnumDevices(m_ShowHiddenDevices);
	auto root = DeviceManager::GetRootDeviceNode();
	WCHAR name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = _countof(name);
	auto ok = ::GetComputerName(name, &size);
	auto hRoot = InsertTreeItem(m_Tree, ok ? name : L"This PC", computerIcon, root);

	m_Guids.clear();
	m_Guids.reserve(64);
	m_Interfaces = DeviceManager::EnumDeviceInterfacesGuids();
	int c = 0;
	for (auto& iface : m_Interfaces) {
		auto hInterface = InsertTreeItem<int>(m_Tree, GetInterfaceName(iface), computerIcon, 0x8000 + c, hRoot, TVI_SORT);
		c++;
		auto mgr = DeviceManager::Create(nullptr, &iface, nullptr, InfoSetOptions::Present | InfoSetOptions::DeviceInterface);
		for (auto& di : mgr->EnumDevices(m_ShowHiddenDevices)) {
			auto hIcon = mgr->GetDeviceIcon(di);
			int image = -1;
			if (hIcon) {
				image = images.AddIcon(hIcon);
				::DestroyIcon(hIcon);
			}
			auto isHidden = (DeviceNode(di.Data.DevInst).GetStatus() & DeviceNodeStatus::NoShowInDeviceManager) == DeviceNodeStatus::NoShowInDeviceManager;
			if (!isHidden || m_ShowHiddenDevices) {
				auto hItem = InsertTreeItem(m_Tree, di.Description.c_str(), image, di.Data.DevInst, hInterface, TVI_SORT);
				if (isHidden)
					m_Tree.SetItemState(hItem, TVIS_CUT, TVIS_CUT);
			}
		}
		if (!m_ShowEmptyClasses && m_Tree.GetChildItem(hInterface) == nullptr)
			m_Tree.DeleteItem(hInterface);
	}

	m_Tree.Expand(hRoot, TVE_EXPAND);

	m_Tree.SetRedraw(TRUE);
}

CString CDeviceInterfacesView::GetInterfaceName(GUID const& guid) {
	return Helpers::DeviceInterfaceToString(guid);
}

LRESULT CDeviceInterfacesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, 0);


	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		| LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Property", LVCFMT_LEFT, 250, 0);
	cm->AddColumn(L"Value", LVCFMT_LEFT, 350, 1);
	cm->AddColumn(L"Details", LVCFMT_LEFT, 550, 2);
	cm->UpdateColumns();

	m_Splitter.SetSplitterPosPct(30);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);

	Refresh();

	return 0;
}

CString CDeviceInterfacesView::GetColumnText(HWND, int row, int col) {
	auto& item = m_Items[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(col))) {
		case ColumnType::Name: return item.Name;
		case ColumnType::Value: return item.ValueAsString;
		case ColumnType::Details: return Helpers::GetPropertyDetails(item.Key, item.Value.get(), item.ValueSize);
	}
	ATLASSERT(false);
	return L"";
}

LRESULT CDeviceInterfacesView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	m_Tree.SetFocus();
	return 0;
}

void CDeviceInterfacesView::DoSort(SortInfo* const si) {
	auto compare = [&](auto const& n1, auto const& n2) {
		return SortHelper::Sort(n1.Name, n2.Name, si->SortAscending);
	};

	std::sort(m_Items.begin(), m_Items.end(), compare);
}

LRESULT CDeviceInterfacesView::OnShowHiddenDevices(WORD, WORD, HWND, BOOL&) {
	m_ShowHiddenDevices = !m_ShowHiddenDevices;
	GetFrame()->GetUI().UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);

	Refresh();
	return 0;
}

void CDeviceInterfacesView::OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew) {
}

void CDeviceInterfacesView::OnPageActivated(bool active) {
	GetFrame()->GetUI().UIEnable(ID_VIEW_SHOWEMPTYCLASSES, active);
}

LRESULT CDeviceInterfacesView::OnShowEmptyClasses(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_ShowEmptyClasses = !m_ShowEmptyClasses;
	GetFrame()->GetUI().UISetCheck(ID_VIEW_SHOWEMPTYCLASSES, m_ShowEmptyClasses);
	Refresh();
	return 0;
}

LRESULT CDeviceInterfacesView::OnCopy(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CDeviceInterfacesView::OnItemChanged(int, LPNMHDR, BOOL&) {
	return LRESULT();
}

LRESULT CDeviceInterfacesView::OnViewRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}
