// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "DevNodeView.h"
#include "IconHelper.h"
#include "Helpers.h"

CString CDevNodeView::GetColumnText(HWND, int row, int col) {
	auto& item = m_Items[row];
	switch (col) {
		case 0: return item.Name;
		case 1: return item.ValueAsString;
		case 2: return Helpers::GetPropertyDetails(item.Key, item.Value.get(), item.ValueSize);
	}
	return L"";
}

void CDevNodeView::OnTreeSelChanged(HTREEITEM hOld, HTREEITEM hNew) {
	auto inst = GetItemData<DEVINST>(m_Tree, hNew);
	DeviceNode node(inst);
	m_Items.clear();
	auto keys = node.GetPropertyKeys();
	m_Items.reserve(keys.size() + 1);
	Property p{};
	p.ValueAsString = std::to_wstring(inst).c_str();
	p.Name = L"Index";
	m_Items.push_back(std::move(p));

	for (auto& key : keys) {
		Property prop;
		prop.Key = key;
		prop.Value = node.GetPropertyValue(key, prop.Type, &prop.ValueSize);
		prop.ValueAsString = Helpers::GetPropertyValueAsString(prop.Value.get(), prop.Type, prop.ValueSize);
		prop.Name = Helpers::GetPropertyName(key);
		m_Items.push_back(std::move(prop));
	}

	m_List.SetItemCount((int)keys.size());
}

BOOL CDevNodeView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CDevNodeView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CDevNodeView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);

	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		| LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Property", LVCFMT_LEFT, 240, 0);
	cm->AddColumn(L"Value", LVCFMT_LEFT, 300, 1);
	cm->AddColumn(L"Details", LVCFMT_LEFT, 450, 2);
	cm->UpdateColumns();

	m_Splitter.SetSplitterPosPct(30);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);

	BuildDevNodeTree();

	return 0;
}

void CDevNodeView::BuildDevNodeTree() {
	m_Tree.SetRedraw(FALSE);

	auto images = DeviceManager::GetClassImageList();
	m_Tree.SetImageList(images, TVSIL_NORMAL);
	auto computerImage = ImageList_AddIcon(images, IconHelper::GetStockIcon(SIID_DESKTOPPC));

	m_DevMgr = DeviceManager::Create();
	m_Devices = m_DevMgr->EnumDevices();
	auto root = DeviceManager::GetRootDeviceNode();
	WCHAR name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = _countof(name);
	auto ok = ::GetComputerName(name, &size);
	int image = computerImage;
	auto hRoot = InsertTreeItem(m_Tree, ok ? name : L"This PC", image, root);
	BuildChildDevNodes(hRoot, root);
	m_Tree.Expand(hRoot, TVE_EXPAND);

	m_Tree.SetRedraw(TRUE);
}

void CDevNodeView::BuildChildDevNodes(HTREEITEM hParent, DeviceNode const& node) {
	for (auto& dn : node.GetChildren()) {
		if ((dn.GetStatus() & DeviceNodeStatus::NoShowInDeviceManager) == DeviceNodeStatus::None) {
			auto icon = m_DevMgr->GetDeviceIcon(m_Devices[dn - 1]);
			int image = -1;
			if (icon) {
				image = m_Tree.GetImageList(TVSIL_NORMAL).AddIcon(icon);
				::DestroyIcon(icon);
			}
			auto hItem = InsertTreeItem(m_Tree, dn.GetProperty<std::wstring>(DEVPKEY_NAME).c_str(), image, dn, hParent, TVI_SORT);
			BuildChildDevNodes(hItem, dn);
		}
	}
}

void CDevNodeView::BuildSiblingDevNodes(HTREEITEM hParent, DeviceNode const& node) {
	for (auto& dn : node.GetSiblings()) {
		if ((dn.GetStatus() & DeviceNodeStatus::NoShowInDeviceManager) == DeviceNodeStatus::None) {
			auto icons = dn.GetProperty<std::vector<std::wstring>>(DEVPKEY_DrvPkg_Icon);
			if (!icons.empty())
				DebugBreak();
			auto hItem = InsertTreeItem(m_Tree, dn.GetProperty<std::wstring>(DEVPKEY_NAME).c_str(), -1, dn, hParent, TVI_SORT);
			BuildChildDevNodes(hItem, dn);
		}
	}
}
