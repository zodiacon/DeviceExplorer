#pragma once

const UINT WM_PAGE_ACTIVATED = WM_APP + 111;
const UINT WM_NEED_REFRESH = WM_PAGE_ACTIVATED + 1;

struct IMainFrame abstract {
	virtual HWND GetHwnd() const = 0;
	virtual BOOL TrackPopupMenu(HMENU hMenu, DWORD flags, int x, int y) = 0;
	virtual CUpdateUIBase& GetUI() = 0;
};
