// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VirtualListView.h"
#include "TreeViewHelper.h"
#include "DeviceManager.h"
#include "ViewBase.h"

class DeviceNode;

class CDevNodeView : 
	public CFrameWindowImpl<CDevNodeView, CWindow, CControlWinTraits>,
	public CViewBase<CDevNodeView>,
	public CVirtualListView<CDevNodeView>,
	public CTreeViewHelper<CDevNodeView> {
public:
	using CViewBase::CViewBase;

	using BaseFrame = CFrameWindowImpl<CDevNodeView, CWindow, CControlWinTraits>;
	DECLARE_WND_CLASS(nullptr)

	//
	// list view callbacks
	//
	CString GetColumnText(HWND, int row, int col);
	bool IsSortable(HWND, int col) const {
		return col == 0;
	}
	void DoSort(SortInfo* const);

	//
	// tree view callbacks
	//
	void OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew);
	bool OnTreeRightClick(HWND, HTREEITEM hItem, POINT const& pt);

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CDevNodeView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnNotifySetFocus)
		CHAIN_MSG_MAP(CVirtualListView<CDevNodeView>)
		CHAIN_MSG_MAP(CTreeViewHelper<CDevNodeView>)
		CHAIN_MSG_MAP(BaseFrame)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNotifySetFocus(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

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
	void BuildSiblingDevNodes(HTREEITEM hParent, DeviceNode const& node);

	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	CSplitterWindow m_Splitter;
	std::unique_ptr<DeviceManager> m_DevMgr;
	std::vector<DeviceInfo> m_Devices;
	std::vector<Property> m_Items;
	HWND m_Focus{ nullptr };
};
