// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "DevNodeView.h"
#include "IconHelper.h"
#include "Helpers.h"
#include "SortHelper.h"
#include "resource.h"
#include "ClipboardHelper.h"
#include "ListViewhelper.h"
#include "AppSettings.h"

CString CDevNodeView::GetColumnText(HWND, int row, int col) {
	auto& item = m_Items[row];
	switch (col) {
		case 0: return item.Name;
		case 1: return item.ValueAsString;
		case 2: return Helpers::GetPropertyDetails(item.Key, item.Value.get(), item.ValueSize);
	}
	return L"";
}

void CDevNodeView::DoSort(SortInfo* const si) {
	auto compare = [&](auto const& n1, auto const& n2) {
		return SortHelper::Sort(n1.Name, n2.Name, si->SortAscending);
	};

	std::sort(m_Items.begin(), m_Items.end(), compare);
}

void CDevNodeView::OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew) {
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

	auto si = GetSortInfo(m_List);
	if (si)
		DoSort(si);
	m_List.SetItemCount((int)keys.size());
	UpdateUI(GetFrame()->GetUI());
}

bool CDevNodeView::OnTreeRightClick(HWND, HTREEITEM hItem, POINT const& pt) {
	m_Tree.SelectItem(hItem);
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	auto cmd = GetFrame()->TrackPopupMenu(menu.GetSubMenu(0), TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y);
	LRESULT result;
	if (cmd) {
		ProcessWindowMessage(m_hWnd, WM_COMMAND, cmd, 0, result, 1);
	}
	return false;
}

BOOL CDevNodeView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CDevNodeView::UpdateUI(CUpdateUIBase& ui) {
	ui.UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);
}

void CDevNodeView::OnPageActivated(bool active) {
	if (active && IsRefreshNeeded())
		BuildDevNodeTree();
}

void CDevNodeView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CDevNodeView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 64, 64);
	m_Tree.SetImageList(images);

	images.AddIcon(AtlLoadIconImage(IDI_DEVICES, 0, 16, 16));

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

	BuildDevNodeTree();

	return 0;
}

LRESULT CDevNodeView::OnSetFocus(UINT, WPARAM, LPARAM, BOOL&) {
	if (m_Focus == nullptr)
		m_Tree.SetFocus();
	return 0;
}

LRESULT CDevNodeView::OnCopy(WORD, WORD, HWND, BOOL&) {
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

LRESULT CDevNodeView::OnNotifySetFocus(int, LPNMHDR hdr, BOOL&) {
	m_Focus = hdr->hwndFrom;
	return 0;
}

LRESULT CDevNodeView::OnViewRefresh(WORD, WORD, HWND, BOOL&) {
	BuildDevNodeTree();
	return 0;
}

LRESULT CDevNodeView::OnShowHiddenDevices(WORD, WORD, HWND, BOOL&) {
	m_ShowHiddenDevices = !m_ShowHiddenDevices;
	GetFrame()->GetUI().UISetCheck(ID_VIEW_SHOWHIDDENDEVICES, m_ShowHiddenDevices);

	BuildDevNodeTree();
	return 0;
}

void CDevNodeView::BuildDevNodeTree() {
	m_Tree.SetRedraw(FALSE);

	m_Tree.DeleteAllItems();
	m_DevMgr = DeviceManager::Create();
	m_Devices = m_DevMgr->EnumDevices(m_ShowHiddenDevices);
	auto root = DeviceManager::GetRootDeviceNode();
	WCHAR name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = _countof(name);
	auto ok = ::GetComputerName(name, &size);
	auto hRoot = InsertTreeItem(m_Tree, ok ? name : L"This PC", 0, root);
	BuildChildDevNodes(hRoot, root);
	m_Tree.Expand(hRoot, TVE_EXPAND);

	m_Tree.SetRedraw(TRUE);
}

void CDevNodeView::BuildChildDevNodes(HTREEITEM hParent, DeviceNode const& node) {
	auto showHidden = m_ShowHiddenDevices;

	for (auto& dn : node.GetChildren()) {
		int index = m_DevMgr->GetDeviceIndex(dn);
		if (index < 0)
			continue;
		auto icon = m_DevMgr->GetDeviceIcon(m_Devices[index]);
		int image = -1;
		if (icon) {
			image = m_Tree.GetImageList(TVSIL_NORMAL).AddIcon(icon);
			::DestroyIcon(icon);
		}
		auto isHidden = (dn.GetStatus() & DeviceNodeStatus::NoShowInDeviceManager) == DeviceNodeStatus::NoShowInDeviceManager;
		if (showHidden || !isHidden) {
			auto hItem = InsertTreeItem(m_Tree, dn.GetName().c_str(), image, dn, hParent, TVI_SORT);
			if(isHidden)
				m_Tree.SetItemState(hItem, TVIS_CUT, TVIS_CUT);
			BuildChildDevNodes(hItem, dn);
		}
	}
}

