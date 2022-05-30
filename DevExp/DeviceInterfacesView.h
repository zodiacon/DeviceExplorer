#pragma once

#include "ViewBase.h"
#include "resource.h"
#include "VirtualListView.h"
#include "DeviceManager.h"
#include "TreeViewHelper.h"

class CDeviceInterfacesView :
	public CTreeViewHelper<CDeviceInterfacesView>,
	public CViewBase<CDeviceInterfacesView>,
	public CVirtualListView<CDeviceInterfacesView> {
public:
	using CViewBase::CViewBase;
	DECLARE_WND_CLASS(nullptr)

	CString GetColumnText(HWND h, int row, int col);
	//int GetRowImage(HWND h, int row, int col);
	void DoSort(const SortInfo* si);

protected:
	BEGIN_MSG_MAP(CDeviceInterfacesView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnNotifySetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CDeviceInterfacesView>)
		CHAIN_MSG_MAP(CTreeViewHelper<CDeviceInterfacesView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWHIDDENDEVICES, OnShowHiddenDevices)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWEMPTYCLASSES, OnShowEmptyClasses)
	END_MSG_MAP()

	void OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew);
	void OnPageActivated(bool active);
	void UpdateUI(CUpdateUIBase& ui);

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

	void Refresh();
	static CString GetInterfaceName(GUID const& info);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHiddenDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowEmptyClasses(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNotifySetFocus(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	CSplitterWindow m_Splitter;
	std::vector<Property> m_Items;
	std::unique_ptr<DeviceManager> m_DevMgr;
	std::vector<DeviceInfo> m_Devices;
	std::unordered_map<GUID, HTREEITEM> m_Guids;
	std::vector<GUID> m_Interfaces;
	HWND m_Focus{ nullptr };
	bool m_ShowHiddenDevices{ false };
	bool m_ShowEmptyClasses{ false };
};
