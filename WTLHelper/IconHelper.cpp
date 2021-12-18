#include "pch.h"
#include "IconHelper.h"

HICON IconHelper::GetShieldIcon() {
	return GetStockIcon(SIID_SHIELD);
}

CComPtr<IImageList2> IconHelper::CreateImageList() {
	CComPtr<IImageList2> spImages;
	ImageList_CoCreateInstance(CLSID_ImageList, nullptr, __uuidof(IImageList2), reinterpret_cast<void**>(&spImages));
	ATLASSERT(spImages);
	return spImages;
}

HICON IconHelper::GetStockIcon(SHSTOCKICONID id, bool big) {
	SHSTOCKICONINFO ssii = { sizeof(ssii) };
	if (FAILED(::SHGetStockIconInfo(id, (big ? 0 : SHGSI_SMALLICON) | SHGSI_ICON, &ssii)))
		return nullptr;

	return ssii.hIcon;
}
