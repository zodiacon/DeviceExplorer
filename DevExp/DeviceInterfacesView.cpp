#include "pch.h"
#include "DeviceInterfacesView.h"

using namespace UI;

int DeviceInterfacesView::GetItemCount() {
	return 0;
}

CString DeviceInterfacesView::GetColumnText(int row, int col) {
	return CString();
}

std::vector<ColumnInfo> DeviceInterfacesView::GetColumns() {
	return std::vector<ColumnInfo>();
}

bool DeviceInterfacesView::Init(IServiceHost* pSvcProvider) {
	return false;
}
