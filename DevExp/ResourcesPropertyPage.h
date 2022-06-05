#pragma once

#include "DialogHelper.h"
#include "resource.h"
#include <VirtualListView.h>

struct DeviceResource;

class CResourcesPropertyPage :
	public CPropertyPageImpl<CResourcesPropertyPage>,
	public CVirtualListView< CResourcesPropertyPage>,
	public CDialogHelper<CResourcesPropertyPage> {
public:
	enum { IDD = IDD_PROP_RES };

	explicit CResourcesPropertyPage(std::vector<DeviceResource> const& resources);

	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(HWND, int row, int) const;

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CVirtualListView<CResourcesPropertyPage>)
	END_MSG_MAP()

	static std::wstring ResourceSettingToString(DeviceResource const& res);

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CListViewCtrl m_List;
	std::vector<DeviceResource> const& m_Items;
};

