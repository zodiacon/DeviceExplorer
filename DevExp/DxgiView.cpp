#include "pch.h"
#include "DxgiView.h"
#include "Helpers.h"

#pragma comment(lib, "dxgi")

static constexpr DWORD TreeNodeMode = 0x1000;
static constexpr DWORD TreeNodeFormat = 0x2000;

template<typename T>
T* QueryAdapterInfo(D3DKMT_HANDLE hAdpater, KMTQUERYADAPTERINFOTYPE type, T& data) {
	D3DKMT_QUERYADAPTERINFO queryInfo;
	queryInfo.hAdapter = hAdpater;
	queryInfo.Type = type;
	queryInfo.pPrivateDriverData = &data;
	queryInfo.PrivateDriverDataSize = sizeof(T);
	auto status = ::D3DKMTQueryAdapterInfo(&queryInfo);
	return status == STATUS_SUCCESS ? &data : nullptr;
}


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

	D3DKMT_ENUMADAPTERS adapters{};
	auto status = D3DKMTEnumAdapters(&adapters);

	for (UINT i = 0;; i++) {
		CComPtr<IDXGIAdapter1> adapter;
		hr = m_Factory->EnumAdapters1(i, &adapter);
		if (FAILED(hr))
			break;

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		auto hItem = m_Tree.InsertItem(desc.Description, 1, 1, hRoot, TVI_SORT);
		TreeItem item;
		item.spUnknown = adapter;
		item.hAdapter = adapters.Adapters[i].hAdapter;

		m_TreeNodes.insert({ hItem, item });

		for (UINT i = 0;; i++) {
			CComPtr<IDXGIOutput> output;
			hr = adapter->EnumOutputs(i, &output);
			if (FAILED(hr))
				break;

			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);
			auto hOutput = m_Tree.InsertItem(desc.DeviceName, 2, 2, hItem, TVI_SORT);
			item.spUnknown = output;
			m_TreeNodes.insert({ hOutput, item});

			for (UINT f = 1; f <= 200; f++) {
				UINT count = 0;
				output->GetDisplayModeList((DXGI_FORMAT)f, 0, &count, nullptr);
				if (count) {
					auto hModes = m_Tree.InsertItem(Helpers::DxgiFormatToString((DXGI_FORMAT)f).c_str(), 3, 3, hOutput, TVI_SORT);
					m_Tree.SetItemData(hModes, f + TreeNodeFormat);
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
	auto& item = m_Items[row];
	switch (col) {
		case 0: return item.Name;
		case 1: return item.ValueAsString;
		case 2: return item.Details;
	}
	return L"";
}

void CDxgiView::DoSort(SortInfo const*) {
}

bool CDxgiView::OnDoubleClickList(HWND, int row, int col, POINT const& pt) const {
	return false;
}

void CDxgiView::OnTreeSelChanged(HWND, HTREEITEM hOld, HTREEITEM hNew) {
	UpdateList(hNew);
}

void CDxgiView::UpdateUI(CUpdateUIBase& ui) {
}

void CDxgiView::OnPageActivated(bool active) {
}

void CDxgiView::UpdateList(HTREEITEM hItem) {
	m_Items.clear();
	if (auto it = m_TreeNodes.find(hItem); it != m_TreeNodes.end()) {
		auto& item = it->second;
		if (CComQIPtr<IDXGIAdapter1> adapter(item.spUnknown); adapter) {
			DXGI_ADAPTER_DESC1 desc1;
			adapter->GetDesc1(&desc1);
			m_Items.emplace_back(L"Description", CString(desc1.Description));
			m_Items.emplace_back(L"Vendor ID", std::format(L"0x{:X}", desc1.VendorId).c_str());
			m_Items.emplace_back(L"Device ID", std::format(L"0x{:X}", desc1.DeviceId).c_str());
			m_Items.emplace_back(L"Subsystem ID", std::format(L"0x{:X}", desc1.SubSysId).c_str());
			m_Items.emplace_back(L"Revision", std::format(L"0x{:X}", desc1.Revision).c_str());
			m_Items.emplace_back(L"LUID", std::format(L"0x{:X}:{:08X}", desc1.AdapterLuid.HighPart, desc1.AdapterLuid.LowPart).c_str());
			//m_Items.emplace_back(L"Dedicated Video Memory", std::format(L"{} MB", desc1.DedicatedVideoMemory >> 20).c_str());
			//m_Items.emplace_back(L"Dedicated System Memory", std::format(L"{} MB", desc1.DedicatedSystemMemory>> 20).c_str());
			//m_Items.emplace_back(L"Shared System Memory", std::format(L"{} MB", desc1.SharedSystemMemory >> 20).c_str());
			m_Items.emplace_back(L"Flags", std::format(L"0x{:X}", desc1.Flags).c_str(), Helpers::AdapterFlagsToString(desc1.Flags));

			if (item.hAdapter) {
				D3DKMT_ADAPTERADDRESS address;
				if(QueryAdapterInfo(item.hAdapter, KMTQAITYPE_ADAPTERADDRESS, address)) {
					m_Items.emplace_back(L"Adapter Address", std::format(L"Bus: 0x{:X} Device: 0x{:X} Function: 0x{:X}", 
						address.BusNumber, address.DeviceNumber, address.FunctionNumber).c_str());
				}
				D3DKMT_SEGMENTSIZEINFO segmentInfo;
				if (QueryAdapterInfo(item.hAdapter, KMTQAITYPE_GETSEGMENTSIZE, segmentInfo)) {
					m_Items.emplace_back(L"Video Memory", std::format(L"{} MB", segmentInfo.DedicatedVideoMemorySize >> 20).c_str());
					m_Items.emplace_back(L"System Memory", std::format(L"{} MB", segmentInfo.DedicatedSystemMemorySize >> 20).c_str());
					m_Items.emplace_back(L"Shared System Memory", std::format(L"{} MB", segmentInfo.SharedSystemMemorySize >> 20).c_str());
				}
				D3DKMT_DRIVERVERSION driverVer;
				if (QueryAdapterInfo(item.hAdapter, KMTQAITYPE_DRIVERVERSION, driverVer)) {
					m_Items.emplace_back(L"Driver Version", std::format(L"{} ", (UINT)driverVer).c_str());
				}
				for (UINT src = 0;; src++) {
					D3DKMT_CURRENTDISPLAYMODE displayMode;
					displayMode.VidPnSourceId = src;
					if (QueryAdapterInfo(item.hAdapter, KMTQAITYPE_CURRENTDISPLAYMODE, displayMode)) {
						m_Items.emplace_back(std::format(L"Source {} Resolution", src).c_str(), std::format(L"{} X {}", displayMode.DisplayMode.Width, displayMode.DisplayMode.Height).c_str());
						m_Items.emplace_back(std::format(L"Source {} Format", src).c_str(), Helpers::DdiFormatToString(displayMode.DisplayMode.Format).c_str());
						m_Items.emplace_back(std::format(L"Source {} Refresh Rate", src).c_str(), std::format(L"{} Hz", displayMode.DisplayMode.IntegerRefreshRate).c_str());
					}
					else
						break;
				}
				D3DKMT_GETPRESENTHISTORY history;
				D3DKMT_PRESENTHISTORYTOKEN tokens[8];
				history.hAdapter = item.hAdapter;
				history.pTokens = tokens;
				history.ProvidedSize = sizeof(tokens);
				auto status = D3DKMTGetPresentHistory(&history);
				if (status == STATUS_SUCCESS) {
				}

			}
		}
		else if (CComQIPtr<IDXGIOutput1> output(item.spUnknown); output) {
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);
			m_Items.emplace_back(L"Device Name", CString(desc.DeviceName));
			m_Items.emplace_back(L"Desktop Coordinates", std::format(L"({},{}) - ({},{})",
				desc.DesktopCoordinates.left, desc.DesktopCoordinates.top, desc.DesktopCoordinates.right, desc.DesktopCoordinates.bottom).c_str(),
				std::format(L"{} x {}", desc.DesktopCoordinates.right - desc.DesktopCoordinates.left, desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top).c_str());
			m_Items.emplace_back(L"Attached to Desktop", desc.AttachedToDesktop ? L"Yes" : L"No");
			m_Items.emplace_back(L"Monitor Handle", std::format(L"{}", (PVOID)desc.Monitor).c_str());
			m_Items.emplace_back(L"Rotation", Helpers::RotationToString(desc.Rotation), L"Degrees");

		}
	}
	else {
		auto data = m_Tree.GetItemData(hItem);
		if (data >= TreeNodeFormat) {
			// display format
			auto hParent = m_Tree.GetParentItem(hItem);
			if (auto it = m_TreeNodes.find(hParent); it != m_TreeNodes.end()) {
				CComQIPtr<IDXGIOutput1> output(it->second.spUnknown);
				ATLASSERT(output);
				if (output) {
					UINT count = 0;
					output->GetDisplayModeList1((DXGI_FORMAT)(data - TreeNodeFormat), 0, &count, nullptr);
					m_Items.emplace_back(L"Display Modes", std::format(L"{}", count).c_str());
				}
			}
		}
		else if (data >= TreeNodeMode) {
			// TODO: display mode properties
		}
	}
	m_List.SetItemCount((int)m_Items.size());
}

