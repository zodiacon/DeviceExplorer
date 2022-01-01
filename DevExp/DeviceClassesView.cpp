#include "pch.h"
#include "DeviceClassesView.h"
#include "Helpers.h"
#include "resource.h"
#include "SortHelper.h"

CString CDeviceClassesView::GetColumnText(HWND, int row, int col) {
	return GetDeviceClassColunnText(row, col);
}

int CDeviceClassesView::GetRowImage(HWND h, int row, int col) {
	return 0;
}

CString CDeviceClassesView::GetDeviceClassColunnText(int row, int col) {
	auto& item = m_Items[row];
	switch (static_cast<ColumnType>(GetColumnManager(m_List)->GetColumnTag(col))) {
		case ColumnType::Name: return item.Name;
		case ColumnType::Value: return item.ValueAsString;
		case ColumnType::Details: return Helpers::GetPropertyDetails(item.Key, item.Value.get(), item.ValueSize);
	}
	ATLASSERT(false);
	return L"";
}

CString CDeviceClassesView::GetDeviceInfoColumnText(int row, int col) {
	return CString();
}

void CDeviceClassesView::DoSort(SortInfo* const si) {
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

	if (m_IsClasses) {
		m_Guids.clear();
		m_Guids.reserve(64);
		m_Classes = DeviceManager::EnumDeviceClasses();
		int c = 0;
		for (auto& dc : m_Classes) {
			auto hItem = InsertTreeItem<int>(m_Tree, DeviceManager::GetDeviceClassDescription(dc.Guid).c_str(), 
				DeviceManager::GetClassImageIndex(dc.Guid), 0x8000 + c, hRoot, TVI_SORT);
			c++;
			m_Guids.insert({ dc.Guid, hItem });
		}
		for (auto& di : m_Devices) {
			if (auto it = m_Guids.find(di.Data.ClassGuid); it != m_Guids.end()) {
				int image = images2.AddIcon(m_DevMgr->GetDeviceIcon(di));
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
	}
	else {
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

	m_Splitter.SetSplitterPosPct(30);
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
	GUID* classGuid{ nullptr };
	if (device) {	// device instance
		keys = node.GetPropertyKeys();
	}
	else {
		//
		//device class
		//
		classGuid = &m_Classes[inst - 0x8000].Guid;
		keys = DeviceManager::GetDeviceClassPropertyKeys(*classGuid);
	}

	m_Items.reserve(keys.size() + 1);
	if (device) {
		Property p{};
		p.ValueAsString = std::to_wstring(inst).c_str();
		p.Name = L"Index";
		m_Items.push_back(std::move(p));
	}

	for (auto& key : keys) {
		Property prop;
		prop.Key = key;
		if (device)
			prop.Value = node.GetPropertyValue(key, prop.Type, &prop.ValueSize);
		else
			prop.Value = DeviceManager::GetClassPropertyValue(*classGuid, key, prop.Type, &prop.ValueSize);

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

LRESULT CDeviceClassesView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	m_Tree.SetFocus();

	return 0;
}

LRESULT CDeviceClassesView::OnCopy(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CDeviceClassesView::OnItemChanged(int, LPNMHDR, BOOL&) {
	return LRESULT();
}

LRESULT CDeviceClassesView::OnViewRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}
