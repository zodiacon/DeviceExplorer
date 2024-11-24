#pragma once

#include "VirtualListView.h"
#include "DeviceManager.h"
#include "ViewBase.h"
#include "resource.h"
#include "SortedFilteredVector.h"
#include "DriverManager.h"
#include <OwnerDrawnMenu.h>

class DeviceNode;

class CDriversView :
	public CViewBase<CDriversView>,
	public CAutoUpdateUI<CDriversView>,
	public CIdleHandler,
	public COwnerDrawnMenu<CDriversView>,
	public CVirtualListView<CDriversView> {
public:
	using CViewBase::CViewBase;

	DECLARE_WND_CLASS(nullptr)

	void Refresh();
	BOOL OnIdle() override;

protected:
	static const UINT WM_UPDATEDRIVER = WM_USER + 100;

	BEGIN_MSG_MAP(CDriversView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		COMMAND_ID_HANDLER(ID_DRIVER_START, OnStartStopDriver)
		COMMAND_ID_HANDLER(ID_DRIVER_STOP, OnStartStopDriver)
		MESSAGE_HANDLER(WM_UPDATEDRIVER, OnUpdateDriver)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CVirtualListView<CDriversView>)
		CHAIN_MSG_MAP(CAutoUpdateUI<CDriversView>)
		CHAIN_MSG_MAP(COwnerDrawnMenu<CDriversView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
	END_MSG_MAP()

	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(HWND, int row, int col);
	void DoSort(const SortInfo* si);
	int GetSaveColumnRange(HWND, int&) const;
	void OnStateChanged(HWND, int from, int to, UINT oldState, UINT newState);

	bool OnRightClickList(HWND, int row, int col, POINT const& pt);
	//bool OnDoubleClickList(HWND, int row, int col, POINT const&);

	// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	enum class ColumnType {
		Name, DisplayName, Type, ImagePath, State, WDFVersion, StartType, ErrorControl
	};

	//
	// CViewBase overridables
	//
	//void OnPageActivated(bool activate);
	void UpdateUI(CUpdateUIBase& ui);

private:
	void UpdateUI();

	LRESULT OnStartStopDriver(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateDriver(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	SortedFilteredVector<DriverInfo> m_Drivers;
};
