#pragma once

#include "DialogHelper.h"
#include "resource.h"

struct DeviceInfo;
class DeviceManager;

class CGeneralPropertyPage : 
	public CPropertyPageImpl<CGeneralPropertyPage>,
	public CDialogHelper<CGeneralPropertyPage> {
public:
	enum { IDD = IDD_PROP_GENERAL };

	CGeneralPropertyPage(DeviceManager const& dm, DeviceInfo const& di);

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	DeviceInfo const& m_di;
	DeviceManager const& m_dm;
};

