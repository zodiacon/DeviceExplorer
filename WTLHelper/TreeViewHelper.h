#pragma once

template<typename T>
struct CTreeViewHelper {
	template<typename TData>
	HTREEITEM InsertTreeItem(CTreeViewCtrl& tree, PCWSTR text, int image, int selectedImage, TData const& data, HTREEITEM hParent = TVI_ROOT, HTREEITEM hAfter = TVI_LAST) {
		auto hItem = tree.InsertItem(text, image, selectedImage, hParent, hAfter);
		if(hItem)
			tree.SetItemData(hItem, static_cast<DWORD_PTR>(data));
		return hItem;
	}

	template<typename TData>
	HTREEITEM InsertTreeItem(CTreeViewCtrl& tree, PCWSTR text, int image, TData const& data, HTREEITEM hParent = TVI_ROOT, HTREEITEM hAfter = TVI_LAST) {
		return InsertTreeItem(tree, text, image, image, data, hParent, hAfter);
	}

	template<typename TData>
	TData GetItemData(CTreeViewCtrl& tree, HTREEITEM hItem) const {
		return static_cast<TData>(tree.GetItemData(hItem));
	}

protected:
	BEGIN_MSG_MAP(CTreeViewHelper)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
	ALT_MSG_MAP(1)
		REFLECTED_NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnSelChanged)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
	END_MSG_MAP()

	LRESULT OnSelChanged(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		auto pT = static_cast<T*>(this);
		auto tv = (NMTREEVIEW*)hdr;
		pT->OnTreeSelChanged(hdr->hwndFrom, tv->itemOld.hItem, tv->itemNew.hItem);
		return 0;
	}

	LRESULT OnRightClick(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		CPoint pt;
		::GetCursorPos(&pt);
		CPoint pt2(pt);
		CTreeViewCtrl tv(hdr->hwndFrom);
		tv.ScreenToClient(&pt);
		auto hItem = tv.HitTest(pt, nullptr);
		if (hItem)
			return static_cast<T*>(this)->OnTreeRightClick(tv, hItem, pt2);
		return 0;
	}

	LRESULT OnDoubleClick(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		CPoint pt;
		::GetCursorPos(&pt);
		CTreeViewCtrl tv(hdr->hwndFrom);
		tv.ScreenToClient(&pt);
		auto hItem = tv.HitTest(pt, nullptr);
		if (hItem)
			return static_cast<T*>(this)->OnTreeDoubleClick(tv, hItem);
		return 0;
	}

private:
	// overridables

	void OnTreeSelChanged(HWND tree, HTREEITEM hOld, HTREEITEM hNew) {}
	bool OnTreeRightClick(HWND tree, HTREEITEM hItem, POINT const& pt) {
		return false;
	}
	bool OnTreeDoubleClick(HWND tree, HTREEITEM hItem) {
		return false;
	}

};
