#pragma once

#include "ViewBase.h"
#include "resource.h"
#include <VirtualListView.h>
#include "DeviceManager.h"
#include <TreeViewHelper.h>
#include <CustomSplitterWindow.h>

class CDeviceClassesView :
	public CTreeViewHelper<CDeviceClassesView>,
	public CViewBase<CDeviceClassesView>,
	public CVirtualListView<CDeviceClassesView> {
public:
	using CViewBase::CViewBase;
	DECLARE_WND_CLASS(nullptr)

	void Refresh();

	CString GetColumnText(HWND h, int row, int col);
	int GetRowImage(HWND h, int row, int col);
	void DoSort(SortInfo const* si);
	bool OnDoubleClickList(HWND, int row, int col, POINT const& pt) const;
	bool OnTreeDoubleClick(HWND, HTREEITEM hItem);

protected:
	BEGIN_MSG_MAP(CDeviceClassesView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnNotifySetFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CDeviceClassesView>)
		CHAIN_MSG_MAP(CTreeViewHelper<CDeviceClassesView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWHIDDENDEVICES, OnShowHiddenDevices)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWEMPTYCLASSES, OnShowEmptyClasses)
		COMMAND_ID_HANDLER(ID_DEVICE_PROPERTIES, OnDeviceProperties)
		COMMAND_ID_HANDLER(ID_DEVICE_ENABLE, OnEnableDisableDevice)
		COMMAND_ID_HANDLER(ID_DEVICE_DISABLE, OnEnableDisableDevice)
	END_MSG_MAP()

	void OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew);
	bool OnTreeRightClick(HWND, HTREEITEM hItem, POINT const& pt);
	bool OnRightClickList(HWND, int row, int col, CPoint const& pt);

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

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnNotifySetFocus(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHiddenDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowEmptyClasses(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnableDisableDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDeviceProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	CCustomSplitterWindow m_Splitter;
	std::vector<Property> m_Items;
	std::unique_ptr<DeviceManager> m_DevMgr;
	std::vector<DeviceInfo> m_Devices;
	std::unordered_map<GUID, HTREEITEM> m_Guids;
	std::vector<GUID> m_Classes;
	HWND m_Focus{ nullptr };
	bool m_ShowHiddenDevices{ false };
	bool m_ShowEmptyClasses{ false };
};
