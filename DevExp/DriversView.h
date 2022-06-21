#pragma once

#include "VirtualListView.h"
#include "DeviceManager.h"
#include "ViewBase.h"
#include "resource.h"
#include "SortedFilteredVector.h"
#include "DriverManager.h"

class DeviceNode;

class CDriversView :
	public CViewBase<CDriversView>,
	public CVirtualListView<CDriversView> {
public:
	using CViewBase::CViewBase;

	DECLARE_WND_CLASS(nullptr)

	void Refresh();

protected:
	BEGIN_MSG_MAP(CDriversView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CDriversView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
	END_MSG_MAP()

	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(HWND, int row, int col);
	void DoSort(const SortInfo* si);
	bool OnRightClickList(HWND, int row, int col, POINT const& pt);
	bool OnDoubleClickList(HWND, int row, int col, POINT const&);

	// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	enum class ColumnType {
		Name, Type, ImagePath, State, WDFVersion,
	};

	//
	// CViewBase overridables
	//
	void UpdateUI(CUpdateUIBase& ui);
	void OnPageActivated(bool activate);

private:

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	SortedFilteredVector<DriverInfo> m_Drivers;
};
