// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VirtualListView.h"
#include "TreeViewHelper.h"
#include "DeviceManager.h"

class DeviceNode;

class CDevNodeView : 
	public CFrameWindowImpl<CDevNodeView, CWindow, CControlWinTraits>,
	public CVirtualListView<CDevNodeView>,
	public CTreeViewHelper<CDevNodeView> {
public:
	using BaseFrame = CFrameWindowImpl<CDevNodeView, CWindow, CControlWinTraits>;
	DECLARE_WND_CLASS(nullptr)

	//
	// list view callbacks
	//
	CString GetColumnText(HWND, int row, int col);

	//
	// tree view callbacks
	//
	void OnTreeSelChanged(HTREEITEM hOld, HTREEITEM hNew);

	BOOL PreTranslateMessage(MSG* pMsg);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	BEGIN_MSG_MAP(CDevNodeView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CDevNodeView>)
		CHAIN_MSG_MAP(CTreeViewHelper<CDevNodeView>)
		CHAIN_MSG_MAP(BaseFrame)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

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
};
