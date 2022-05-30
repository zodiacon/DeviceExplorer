#pragma once

#include "Interfaces.h"

template<typename T>
struct CViewBase 	
	: public CFrameWindowImpl<T, CWindow, CControlWinTraits> {
	using BaseFrame = CFrameWindowImpl<T, CWindow, CControlWinTraits>;
	
	CViewBase(IMainFrame* frame) : m_pFrame(frame) {}

protected:
	BEGIN_MSG_MAP(CViewBase)
		MESSAGE_HANDLER(WM_PAGE_ACTIVATED, OnPageActivated)
		MESSAGE_HANDLER(WM_NEED_REFRESH, OnNeedRefresh)
		CHAIN_MSG_MAP(BaseFrame)
	END_MSG_MAP()

	void OnFinalMessage(HWND) override {
		delete this;
	}

	IMainFrame* GetFrame() {
		return m_pFrame;
	}

	bool IsRefreshNeeded() const {
		auto need = m_NeedRefresh;
		m_NeedRefresh = false;
		return need;
	}

	LRESULT OnPageActivated(UINT, WPARAM wp, LPARAM, BOOL&) {
		m_IsActive = (bool)wp;
		static_cast<T*>(this)->OnPageActivated(wp ? true : false);
		if (wp)
			static_cast<T*>(this)->UpdateUI(m_pFrame->GetUI());
		return 0;
	}

	LRESULT OnNeedRefresh(UINT, WPARAM wp, LPARAM, BOOL&) {
		auto pT = static_cast<T*>(this);
		if (m_IsActive) {
			LRESULT result;
			pT->ProcessWindowMessage(pT->m_hWnd, WM_COMMAND, ID_VIEW_REFRESH, 0, result, 1);
		}
		else
			m_NeedRefresh = true;
		return 0;
	}

	//
	// overridables
	//
	void OnPageActivated(bool activate) {}
	void UpdateUI(CUpdateUIBase& ui) {}

private:
	IMainFrame* m_pFrame;
	mutable bool m_NeedRefresh{ false };
	bool m_IsActive{ false };
};
