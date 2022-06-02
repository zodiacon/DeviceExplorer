#pragma once

#include "resource.h"
#include <DialogHelper.h>

class CMultiStringListDlg : 
	public CDialogImpl<CMultiStringListDlg>,
	public CDialogHelper<CMultiStringListDlg>,
	public CDynamicDialogLayout<CMultiStringListDlg> {
public:
	enum { IDD = IDD_STRINGLIST };

	CMultiStringListDlg(PCWSTR title) : m_Title(title) {}

	void SetData(std::vector<std::wstring> data);

	BEGIN_MSG_MAP(CMultiStringListDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CMultiStringListDlg>)
	END_MSG_MAP()

private:
	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CString m_Title;
	CEdit m_List;
	std::vector<std::wstring> m_Data;
};
