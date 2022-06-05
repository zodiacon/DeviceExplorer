#include "pch.h"
#include "MultiStringListDlg.h"

void CMultiStringListDlg::SetData(std::vector<std::wstring> data) {
    m_Data = std::move(data);
}

LRESULT CMultiStringListDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    InitDynamicLayout();
    SetDialogIcon(IDR_MAINFRAME);

    SetWindowText(m_Title);
    m_List.Attach(GetDlgItem(IDC_LIST));
    CFont font;
    font.CreatePointFont(105, L"Consolas");
    m_List.SetFont(font.Detach());

    std::wstring text;
    for (auto& s : m_Data) {
        text += s + L"\r\n";
    }
    m_List.SetWindowText(text.c_str());
    m_List.SetSelNone(TRUE);

    return 0;
}

LRESULT CMultiStringListDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
    EndDialog(wID);
    return 0;
}
