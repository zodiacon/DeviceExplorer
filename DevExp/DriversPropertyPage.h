#pragma once

#include "resource.h"
#include <VirtualListView.h>
#include "DeviceManager.h"

class CDriversPropertyPage :
	public CPropertyPageImpl<CDriversPropertyPage>,
	public CVirtualListView< CDriversPropertyPage> {
public:
	enum { IDD = IDD_PROP_DRIVERS };

	explicit CDriversPropertyPage(std::vector<DeviceDriverInfo> const& drivers);

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row, int) const;

	BEGIN_MSG_MAP(CDriversPropertyPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CVirtualListView<CDriversPropertyPage>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CListViewCtrl m_List;
	std::vector<DeviceDriverInfo> const& m_Items;
};

