#pragma once

#include "Interfaces.h"

template<typename T>
struct CViewBase {
	CViewBase(IMainFrame* frame) : m_pFrame(frame) {}

protected:
	BEGIN_MSG_MAP(CViewBase)
		MESSAGE_HANDLER(WM_PAGE_ACTIVATED, OnPageActivated)
	END_MSG_MAP()

	IMainFrame* GetFrame() {
		return m_pFrame;
	}

	LRESULT OnPageActivated(UINT, WPARAM wp, LPARAM, BOOL&) {
		static_cast<T*>(this)->OnPageActivated(wp ? true : false);
		if (wp)
			static_cast<T*>(this)->UpdateUI(m_pFrame->GetUI());
		return 0;
	}

	//
	// overridables
	//
	void OnPageActivated(bool activate) {}
	void UpdateUI(CUpdateUIBase& ui) {}

private:
	IMainFrame* m_pFrame;
};
