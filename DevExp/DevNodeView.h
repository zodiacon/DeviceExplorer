// View.h : interface of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "VirtualListView.h"
#include "TreeViewHelper.h"

class CDevNodeView : 
	public CFrameWindowImpl<CDevNodeView, CWindow, CControlWinTraits>,
	public CVirtualListView<CDevNodeView>,
	public CTreeViewHelper<CDevNodeView> {
public:
	using BaseFrame = CFrameWindowImpl<CDevNodeView, CWindow, CControlWinTraits>;
	DECLARE_WND_CLASS(nullptr)

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
	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	CSplitterWindow m_Splitter;
};
