#pragma once

struct IconHelper {
	static HICON GetStockIcon(SHSTOCKICONID id, bool big = false);
	static HICON GetShieldIcon();
	static CComPtr<IImageList2> CreateImageList();
};
