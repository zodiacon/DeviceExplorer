#include "pch.h"
#include "ResourcesPropertyPage.h"
#include "DeviceManager.h"
#include <ColumnManager.h>
#include "Helpers.h"

CResourcesPropertyPage::CResourcesPropertyPage(std::vector<DeviceResource> const& resources) 
	: CPropertyPageImpl(L"Resources"), m_Items(resources) {
}

CString CResourcesPropertyPage::GetColumnText(HWND, int row, int col) const {
	auto& item = m_Items[row];

	switch (col) {
		case 0: return Helpers::ResourceTypeToString(item.Type);
		case 1: return ResourceSettingToString(item).c_str();
	}
	return CString();
}

int CResourcesPropertyPage::GetRowImage(HWND, int row, int) const {
	switch (m_Items[row].Type) {
		case ResourceType::Memory:
		case ResourceType::LargeMemory:
			return 0;
		case ResourceType::Interrupt:
			return 1;
		case ResourceType::IO:
			return 2;
	}
	return 3;
}

std::wstring CResourcesPropertyPage::ResourceSettingToString(DeviceResource const& res) {
	switch (res.Type) {
		case ResourceType::Memory:
		case ResourceType::LargeMemory:
		{
			auto& header = res.Memory().MEM_Header;
			return std::format(L"0x{:016X} - 0x{:016X} (0x{:X})",
				header.MD_Alloc_Base, header.MD_Alloc_End, header.MD_Alloc_End - header.MD_Alloc_Base + 1);
		}

		case ResourceType::Interrupt:
		{
			auto& irq = res.Interrupt().IRQ_Header.IRQD_Alloc_Num;
			return std::format(L"IRQ: 0x{:X} ({})", irq, (LONG)irq).c_str();
		}

		case ResourceType::IO:
			auto& io = res.IO().IO_Header;
			return std::format(L"0x{:04X} - 0x{:04X} (0x{:X})", 
				io.IOD_Alloc_Base, io.IOD_Alloc_End, io.IOD_Alloc_End - io.IOD_Alloc_Base + 1);
	}
	return L"";
}

LRESULT CResourcesPropertyPage::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	m_List.SetItemCount((int)m_Items.size());

	CImageList images;
	images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 4);
	UINT icons[] = { IDI_MEMORY, IDI_INTERRUPT, IDI_PORT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Resource Type", LVCFMT_LEFT, 100);
	cm->AddColumn(L"Setting", LVCFMT_LEFT, 300);
	cm->UpdateColumns();

	return 0;
}
