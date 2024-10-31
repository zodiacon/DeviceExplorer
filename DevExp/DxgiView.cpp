#include "pch.h"
#include "DxgiView.h"
#include "Helpers.h"

#pragma comment(lib, "dxgi")

static DWORD TreeNodeMode = 0x1000;

LRESULT CDxgiView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, 0);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 64, 64);
	m_Tree.SetImageList(images);

	UINT ids[] = { IDI_DIRECTX, IDI_ADAPTER, IDI_DEVICE, IDI_RECT, IDI_RESOLUTION };
	for(auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));

	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		| LVS_OWNERDATA | LVS_REPORT | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Property", LVCFMT_LEFT, 320, 0);
	cm->AddColumn(L"Value", LVCFMT_LEFT, 360, 1);
	cm->AddColumn(L"Details", LVCFMT_LEFT, 550, 2);
	cm->UpdateColumns();

	m_Splitter.SetSplitterPosPct(30);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);

	if (!BuildTree()) {
		AtlMessageBox(nullptr, L"Failed to create DXGI factory", IDS_TITLE, MB_ICONERROR);
		return -1;
	}

	return 0;
}

bool CDxgiView::BuildTree() {
	m_Tree.SetRedraw(FALSE);
	m_Tree.DeleteAllItems();
	m_TreeNodes.clear();
	auto hRoot = m_Tree.InsertItem(L"DXGI", 0, 0, TVI_ROOT, TVI_LAST);
	auto hr = ::CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&m_Factory));
	if (FAILED(hr))
		return false;

	for (UINT i = 0;; i++) {
		CComPtr<IDXGIAdapter1> adapter;
		hr = m_Factory->EnumAdapters1(i, &adapter);
		if (FAILED(hr))
			break;

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		auto hItem = m_Tree.InsertItem(desc.Description, 1, 1, hRoot, TVI_SORT);
		m_TreeNodes.insert({ hItem, (IUnknown*)adapter });

		for (UINT i = 0;; i++) {
			CComPtr<IDXGIOutput> output;
			hr = adapter->EnumOutputs(i, &output);
			if (FAILED(hr))
				break;

			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);
			auto hOutput = m_Tree.InsertItem(desc.DeviceName, 2, 2, hItem, TVI_SORT);
			m_TreeNodes.insert({ hOutput, (IUnknown*)output});

			for (UINT f = 1; f <= 200; f++) {
				UINT count = 0;
				output->GetDisplayModeList((DXGI_FORMAT)f, 0, &count, nullptr);
				if (count) {
					auto hModes = m_Tree.InsertItem(Helpers::DxgiFormatToString((DXGI_FORMAT)f).c_str(), 3, 3, hOutput, TVI_SORT);
					m_Tree.SetItemData(hModes, f);
					auto modes = std::make_unique<DXGI_MODE_DESC[]>(count);
					output->GetDisplayModeList((DXGI_FORMAT)f, 0, &count, modes.get());
					for (UINT i = 0; i < count; i++) {
						auto& mode = modes[i];
						auto hMode = m_Tree.InsertItem(std::format(L"{}x{} {:.0f} Hz", mode.Width, mode.Height,
							mode.RefreshRate.Numerator / (float)mode.RefreshRate.Denominator).c_str(), 
							4, 4, hModes, TVI_LAST);
						m_Tree.SetItemData(hMode, i + TreeNodeMode);
					}
				}
			}
		}

	}

	m_Tree.Expand(hRoot, TVE_EXPAND);
	m_Tree.SetRedraw();

	return true;
}

CString CDxgiView::GetColumnText(HWND, int row, int col) {
	return CString();
}

void CDxgiView::DoSort(SortInfo const*) {
}

bool CDxgiView::OnDoubleClickList(HWND, int row, int col, POINT const& pt) const {
	return false;
}

void CDxgiView::OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew) {
}

void CDxgiView::UpdateUI(CUpdateUIBase& ui) {
}

void CDxgiView::OnPageActivated(bool active) {
}
