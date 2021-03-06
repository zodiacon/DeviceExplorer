#pragma once

#include "VirtualListView.h"
#include "DeviceManager.h"
#include "ViewBase.h"
#include "resource.h"

class DeviceNode;

class CDevNodeListView :
	public CViewBase<CDevNodeListView>,
	public CCustomDraw<CDevNodeListView>,
	public CVirtualListView<CDevNodeListView> {
public:
	using CViewBase::CViewBase;
	DECLARE_WND_CLASS(nullptr)

	DWORD OnPrePaint(int, LPNMCUSTOMDRAW cd);
	DWORD OnItemPrePaint(int, LPNMCUSTOMDRAW cd);
	DWORD OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd);

	void Refresh();

protected:
	BEGIN_MSG_MAP(CDevNodeListView)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CVirtualListView<CDevNodeListView>)
		CHAIN_MSG_MAP(CCustomDraw<CDevNodeListView>)
		CHAIN_MSG_MAP(CViewBase)
	ALT_MSG_MAP(1)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWHIDDENDEVICES, OnShowHiddenDevices)
		COMMAND_ID_HANDLER(ID_DEVICE_PROPERTIES, OnDeviceProperties)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnViewRefresh)
		COMMAND_ID_HANDLER(ID_DEVICE_ENABLE, OnEnableDisableDevice)
		COMMAND_ID_HANDLER(ID_DEVICE_DISABLE, OnEnableDisableDevice)
	END_MSG_MAP()

	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(HWND, int row, int col);
	void DoSort(const SortInfo* si);
	bool OnRightClickList(HWND, int row, int col, POINT const& pt);
	bool OnDoubleClickList(HWND, int row, int col, POINT const&);

	// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	enum class ColumnType {
		Name, Class, ClassGuid,
		Status, Instance, Driver, Enumerator, PDOName, Parent, Depth
	};

	//
	// CViewBase overridables
	//
	void UpdateUI(CUpdateUIBase& ui);
	void OnPageActivated(bool activate);

private:

	struct DeviceItem : DeviceInfo {
		CString Class;
		int Image{ -1 };
	};

	CString const& GetDeviceClassName(DeviceItem& di);
	CString GetStringProperty(DeviceItem& item, DEVPROPKEY const& key);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowHiddenDevices(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnableDisableDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnDeviceProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CListViewCtrl m_List;
	std::vector<DeviceItem> m_Items;
	std::unique_ptr<DeviceManager> m_dm;
	bool m_ShowHiddenDevices{ false };
};
