#pragma once

#include "UIFramework.h"

struct DeviceInterfacesView : ComClass<UI::IListViewProvider, UI::IViewHost> {
	// IListViewProvider
	int GetItemCount() override;
	CString GetColumnText(int row, int col) override;
	std::vector<UI::ColumnInfo> GetColumns() override;

	// IViewHost
	bool Init(UI::IServiceHost* pSvcProvider) override;
};
