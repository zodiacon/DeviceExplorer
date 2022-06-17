#include "pch.h"
#include "DeviceClassesView.h"
#include "Helpers.h"
#include "resource.h"
#include "SecurityHelper.h"
#include "SortHelper.h"
#include "ClipboardHelper.h"
#include "ListViewhelper.h"

CString CDeviceClassesView::GetColumnText(HWND, int row, int col) {
	auto& item = m_Items[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(col))) {
		case ColumnType::Name: return item.Name;
		case ColumnType::Value: return item.ValueAsString;
		case ColumnType::Details: return Helpers::GetPropertyDetails(item.Key, item.Value.get(), item.ValueSize);
	}
	ATLASSERT(false);
	return L"";
}

int CDeviceClassesView::GetRowImage(HWND h, int row, int col) {
	return 0;
}

void CDeviceClassesView::DoSort(SortInfo const* si) {
	auto compare = [&](auto const& n1, auto const& n2) {
		return SortHelper::Sort(n1.Name, n2.Name, si->SortAscending);
	};

	std::sort(m_Items.begin(), m_Items.end(), compare);
}

LRESULT CDeviceClassesView::OnShowHiddenDevices(WORD, WORD, HWND, BOOL&) {
	m_ShowHiddenDevices = !m_ShowHiddenDevices;
	GetFrame()->GetUI().UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);

	Refresh();
	return 0;
}

void CDeviceClassesView::Refresh() {
	m_Tree.SetRedraw(FALSE);

	CImageList images(DeviceManager::GetClassImageList());
	auto images2 = images.Duplicate();
	int computerIcon = images2.AddIcon(AtlLoadIconImage(IDI_DEVICES, 0, 16, 16));
	m_Tree.DeleteAllItems();
	m_Tree.SetImageList(images2);

	m_DevMgr = DeviceManager::Create();
	m_Devices = m_DevMgr->EnumDevices(m_ShowHiddenDevices);
	auto root = DeviceManager::GetRootDeviceNode();
	WCHAR name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = _countof(name);
	auto ok = ::GetComputerName(name, &size);
	auto hRoot = InsertTreeItem(m_Tree, ok ? name : L"This PC", computerIcon, root);

	m_Guids.clear();
	m_Guids.reserve(64);
	m_Classes = DeviceManager::EnumDeviceClassesGuids();
	int c = 0;
	for (auto& guid : m_Classes) {
		auto hItem = InsertTreeItem<int>(m_Tree,
			DeviceManager::GetDeviceClassDescription(guid).c_str(),
			DeviceManager::GetClassImageIndex(guid), 0x8000 + c, hRoot, TVI_SORT);
		c++;
		m_Guids.insert({ guid, hItem });
	}
	for (auto& di : m_Devices) {
		if (auto it = m_Guids.find(di.Data.ClassGuid); it != m_Guids.end()) {
			auto hIcon = m_DevMgr->GetDeviceIcon(di);
			int image = -1;
			if (hIcon) {
				image = images2.AddIcon(hIcon);
				::DestroyIcon(hIcon);
			}
			auto isHidden = (DeviceNode(di.Data.DevInst).GetStatus() & DeviceNodeStatus::NoShowInDeviceManager) == DeviceNodeStatus::NoShowInDeviceManager;
			if (!isHidden || m_ShowHiddenDevices) {
				auto hItem = InsertTreeItem(m_Tree, di.Description.c_str(), image, di.Data.DevInst, it->second, TVI_SORT);
				if (isHidden)
					m_Tree.SetItemState(hItem, TVIS_CUT, TVIS_CUT);
			}
		}
	}

	if (!m_ShowEmptyClasses) {
		//
		// delete classes with no devices
		//
		for (auto& [_, hItem] : m_Guids) {
			if (m_Tree.GetChildItem(hItem) == nullptr)
				m_Tree.DeleteItem(hItem);
		}
	}
	m_Tree.Expand(hRoot, TVE_EXPAND);
	m_Tree.SetRedraw(TRUE);
}

LRESULT CDeviceClassesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
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

	m_Splitter.SetSplitterPosPct(25);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);

	Refresh();

	return 0;
}

void CDeviceClassesView::OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew) {
	m_Items.clear();
	auto inst = GetItemData<DEVINST>(m_Tree, hNew);
	DeviceNode node(inst);
	bool device = inst < 0x8000;
	std::vector<DEVPROPKEY> keys;
	GUID classGuid{ GUID_NULL };
	if (device) {	// device instance
		keys = node.GetPropertyKeys();
	}
	else {
		//
		//device class
		//
		classGuid = m_Classes[inst - 0x8000];
		keys = DeviceManager::GetDeviceClassPropertyKeys(classGuid);
	}

	m_Items.reserve(keys.size() + 1);
	if (device) {
		{
			Property p{};
			p.ValueAsString = std::to_wstring(inst).c_str();
			p.Name = L"Index";
			m_Items.push_back(std::move(p));
		}
		{
			Property p{};
			p.ValueAsString = std::to_wstring(DeviceNode(inst).GetDepth()).c_str();
			p.Name = L"Depth";
			m_Items.push_back(std::move(p));
		}
	}

	for (auto& key : keys) {
		Property prop;
		prop.Key = key;
		if (device)
			prop.Value = node.GetPropertyValue(key, prop.Type, &prop.ValueSize);
		else
			prop.Value = DeviceManager::GetClassPropertyValue(classGuid, key, prop.Type, &prop.ValueSize);

		prop.ValueAsString = Helpers::GetPropertyValueAsString(prop.Value.get(), prop.Type, prop.ValueSize);
		prop.Name = Helpers::GetPropertyName(key);
		m_Items.push_back(std::move(prop));
	}

	auto si = GetSortInfo(m_List);
	if (si)
		DoSort(si);
	m_List.SetItemCount((int)keys.size());
	UpdateUI(GetFrame()->GetUI());
}

void CDeviceClassesView::OnPageActivated(bool active) {
	GetFrame()->GetUI().UIEnable(ID_VIEW_SHOWEMPTYCLASSES, active);
}

LRESULT CDeviceClassesView::OnShowEmptyClasses(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_ShowEmptyClasses = !m_ShowEmptyClasses;
	GetFrame()->GetUI().UISetCheck(ID_VIEW_SHOWEMPTYCLASSES, m_ShowEmptyClasses);
	Refresh();
	return 0;
}

LRESULT CDeviceClassesView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	if (m_Focus == nullptr)
		m_Tree.SetFocus();

	return 0;
}

LRESULT CDeviceClassesView::OnCopy(WORD, WORD, HWND, BOOL&) {
	if (m_Focus == m_Tree) {
		auto hItem = m_Tree.GetSelectedItem();
		if (hItem) {
			CString text;
			m_Tree.GetItemText(hItem, text);
			ClipboardHelper::CopyText(m_hWnd, text);
			return 0;
		}
	}
	else if (m_Focus == m_List) {
		CString text;
		for (auto i = m_List.GetNextItem(-1, LVIS_SELECTED); i >= 0; i = m_List.GetNextItem(i, LVIS_SELECTED)) {
			text += ListViewHelper::GetRowAsString(m_List, i);
			text += L"\n";
		}
		ClipboardHelper::CopyText(m_hWnd, text.Left(text.GetLength() - 1));
	}
	return 0;
}

LRESULT CDeviceClassesView::OnItemChanged(int, LPNMHDR, BOOL&) {
	return 0;
}

LRESULT CDeviceClassesView::OnNotifySetFocus(int, LPNMHDR hdr, BOOL&) {
	m_Focus = hdr->hwndFrom;
	return 0;
}

LRESULT CDeviceClassesView::OnViewRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CDeviceClassesView::OnEnableDisableDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	DeviceNode dn((DEVINST)m_Tree.GetItemData(m_Tree.GetSelectedItem()));
	auto result = dn.IsEnabled() ? dn.Disable() : dn.Enable();
	return 0;
}

LRESULT CDeviceClassesView::OnDeviceProperties(WORD, WORD, HWND, BOOL&) {
	CString name;
	m_Tree.GetItemText(m_Tree.GetSelectedItem(), name);
	auto inst = (DEVINST)m_Tree.GetItemData(m_Tree.GetSelectedItem());
	if (inst >= 0x8000)
		return 0;
	
	auto index = m_DevMgr->GetDeviceIndex(inst);
	Helpers::DisplayProperties(name, *m_DevMgr, m_Devices[index]);
	return 0;
}

bool CDeviceClassesView::OnTreeRightClick(HWND, HTREEITEM hItem, POINT const& pt) {
	m_Tree.SelectItem(hItem);
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	auto cmd = GetFrame()->TrackPopupMenu(menu.GetSubMenu(m_Tree.GetItemData(hItem) < 0x8000 ? 0 : 1), 
		TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y);
	LRESULT result;
	if (cmd) {
		ProcessWindowMessage(m_hWnd, WM_COMMAND, cmd, 0, result, 1);
	}
	return false;
}

bool CDeviceClassesView::OnRightClickList(HWND, int row, int col, CPoint const& pt) {
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	return GetFrame()->TrackPopupMenu(menu.GetSubMenu(1), TPM_RIGHTBUTTON, pt.x, pt.y);
}

bool CDeviceClassesView::OnTreeDoubleClick(HWND, HTREEITEM hItem) {
	if (m_Tree.GetItemData(hItem) >= 0x8000)
		return false;

	LRESULT result;
	ProcessWindowMessage(m_hWnd, WM_COMMAND, ID_DEVICE_PROPERTIES, 0, result, 1);
	return true;
}

void CDeviceClassesView::UpdateUI(CUpdateUIBase& ui) {
	ui.UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);
	ui.UISetCheck(ID_VIEW_SHOWEMPTYCLASSES, m_ShowEmptyClasses);
	auto dev = GetItemData<DEVINST>(m_Tree, m_Tree.GetSelectedItem());
	ui.UIEnable(ID_DEVICE_PROPERTIES, dev < 0x8000 && m_Tree.GetSelectedItem() != m_Tree.GetRootItem());

	int selected = m_List.GetSelectionMark();
	if (SecurityHelper::IsRunningElevated()) {
		DeviceNode dn(dev);
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

bool CDeviceClassesView::OnDoubleClickList(HWND, int row, int col, POINT const& pt) const {
	if (row < 0)
		return false;

	auto const& item = m_Items[row];
	return Helpers::DisplayProperty(item.Key, DeviceNode(GetItemData<DEVINST>(m_Tree, m_Tree.GetSelectedItem())), item.Name);
}
