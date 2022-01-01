#pragma once

#include "ViewBase.h"
#include "resource.h"
#include "VirtualListView.h"
#include "DeviceManager.h"
#include "TreeViewHelper.h"

class CDeviceClassesView :
	public CFrameWindowImpl<CDeviceClassesView, CWindow, CControlWinTraits>,
	public CTreeViewHelper<CDeviceClassesView>,
	public CViewBase<CDeviceClassesView>,
	public CVirtualListView<CDeviceClassesView> {
public:
	using BaseFrame = CFrameWindowImpl<CDeviceClassesView, CWindow, CControlWinTraits>;
	DECLARE_WND_CLASS(nullptr)

	CDeviceClassesView(IMainFrame* frame, bool deviceClasses) : CViewBase(frame), m_IsClasses(deviceClasses) {}

	CString GetColumnText(HWND h, int row, int col);
	int GetRowImage(HWND h, int row, int col);
	void DoSort(SortInfo* const si);

protected:
	BEGIN_MSG_MAP(CDeviceClassesView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(BaseFrame)
		CHAIN_MSG_MAP(CVirtualListView<CDeviceClassesView>)
		CHAIN_MSG_MAP(CTreeViewHelper<CDeviceClassesView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWHIDDENDEVICES, OnShowHiddenDevices)
	END_MSG_MAP()

	void OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew);

private:
	enum class ColumnType {
		Name, Value, Details
	};

	struct Property {
		DEVPROPKEY Key;
		DEVPROPTYPE Type;
		CString Name;
		CString ValueAsString;
		std::unique_ptr<BYTE[]> Value;
		ULONG ValueSize;
	};

	CString GetDeviceClassColunnText(int row, int col);
	CString GetDeviceInfoColumnText(int row, int col);

	void Refresh();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHiddenDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	CSplitterWindow m_Splitter;
	std::vector<Property> m_Items;
	std::unique_ptr<DeviceManager> m_DevMgr;
	std::vector<DeviceInfo> m_Devices;
	std::unordered_map<GUID, HTREEITEM> m_Guids;
	std::vector<DeviceClassInfo> m_Classes;
	bool m_IsClasses;
	bool m_ShowHiddenDevices{ false };
	bool m_ShowEmptyClasses{ false };
};
