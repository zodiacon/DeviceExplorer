// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VirtualListView.h"
#include "TreeViewHelper.h"
#include "DeviceManager.h"
#include "ViewBase.h"
#include "resource.h"
#include <CustomSplitterWindow.h>

class DeviceNode;

class CDevNodeView : 
	public CViewBase<CDevNodeView>,
	public CVirtualListView<CDevNodeView>,
	public CCustomDraw<CDevNodeView>,
	public CTreeViewHelper<CDevNodeView> {
public:
	using CViewBase::CViewBase;

	DECLARE_WND_CLASS(nullptr)

	//
	// list view callbacks
	//
	CString GetColumnText(HWND, int row, int col);
	bool IsSortable(HWND, int col) const {
		return col == 0;
	}
	void DoSort(SortInfo const*);
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt) const;

	//
	// tree view callbacks
	//
	void OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew);
	bool OnTreeRightClick(HWND, HTREEITEM hItem, POINT const& pt);
	bool OnTreeDoubleClick(HWND, HTREEITEM hItem);

	BOOL PreTranslateMessage(MSG* pMsg);

	//
	// CViewBase overridables
	//
	void UpdateUI(CUpdateUIBase& ui);
	void OnPageActivated(bool active);

	BEGIN_MSG_MAP(CDevNodeView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnNotifySetFocus)
		CHAIN_MSG_MAP(CVirtualListView<CDevNodeView>)
		CHAIN_MSG_MAP(CCustomDraw<CDevNodeView>)
		CHAIN_MSG_MAP(CTreeViewHelper<CDevNodeView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
		COMMAND_ID_HANDLER(ID_DEVICE_ENABLE, OnEnableDisableDevice)
		COMMAND_ID_HANDLER(ID_DEVICE_DISABLE, OnEnableDisableDevice)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWHIDDENDEVICES, OnShowHiddenDevices)
		COMMAND_ID_HANDLER(ID_DEVICE_PROPERTIES, OnViewProperties)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHiddenDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNotifySetFocus(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnableDisableDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	struct Property {
		DEVPROPKEY Key;
		DEVPROPTYPE Type;
		CString Name;
		CString ValueAsString;
		std::unique_ptr<BYTE[]> Value;
		ULONG ValueSize;
	};

	void BuildDevNodeTree();
	void BuildChildDevNodes(HTREEITEM hParent, DeviceNode const& node);

	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	CCustomSplitterWindow m_Splitter;
	std::unique_ptr<DeviceManager> m_DevMgr;
	std::vector<DeviceInfo> m_Devices;
	std::vector<Property> m_Items;
	HWND m_Focus{ nullptr };
	bool m_ShowHiddenDevices{ false };
};
