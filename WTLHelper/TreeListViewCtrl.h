#pragma once

#define TLN_GETDISPINFO (TVN_FIRST - 30)

struct TLNMDISPINFO {
	NMHDR hdr;
	HTREEITEM hItem;
	LPARAM lParam;
	int column;
	PWSTR text;
	DWORD cchText;
	DWORD_PTR headerParam;
	COLORREF TextColor;
	COLORREF BackColor;
};

class CTreeListViewCtrl :
	public CWindowImpl<CTreeListViewCtrl>,
	public CCustomDraw<CTreeListViewCtrl> {
public:
	friend class CCustomDraw<CTreeListViewCtrl>;

	DECLARE_WND_CLASS(L"WTL_TreeListView")

	enum { HeaderId = 123 };
	BEGIN_MSG_MAP(CTreeListViewCtrl)
		NOTIFY_CODE_HANDLER(TVN_GETDISPINFO, OnTreeGetDispInfo)
		MESSAGE_HANDLER(WM_SETREDRAW, OnSetRedraw)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelChanged)
		NOTIFY_CODE_HANDLER(HDN_ITEMCHANGED, OnHeaderItemChanged)
		NOTIFY_CODE_HANDLER(HDN_TRACK, OnTrackHeader)
		NOTIFY_CODE_HANDLER(HDN_BEGINTRACK, OnBeginTrackHeader)
		NOTIFY_CODE_HANDLER(HDN_ITEMCLICK, OnHeaderItemClick)
		NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDING, OnItemExpanding)
		NOTIFY_CODE_HANDLER(TVN_ITEMEXPANDED, OnItemExpanded)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		CHAIN_MSG_MAP(CCustomDraw<CTreeListViewCtrl>)
		FORWARD_NOTIFICATIONS()
		ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_PAINT, OnPaintTree)
	END_MSG_MAP()

	CTreeListViewCtrl() : m_Tree(this, 1) {}

	// attributes
	int GetItemCount() const;
	int GetColumnCount() const;
	void SetTreeMode(bool treeMode);
	bool IsTreeMode() const;
	CImageList SetImageList(HIMAGELIST hil, int type);
	HTREEITEM GetRootItem() const;
	template<typename T>
	bool SetItemData(HTREEITEM hItem, const T& data) {
		return m_Tree.SetItemData(hItem, reinterpret_cast<DWORD_PTR>(data));
	}
	template<typename T>
	T GetItemData(HTREEITEM hItem) const {
		return reinterpret_cast<T>(m_Tree.GetItemData(hItem));
	}
	HTREEITEM GetParentItem(HTREEITEM hItem) const;
	HTREEITEM GetSelectedItem() const;
	HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const;
	HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const;

	// operations
	HTREEITEM InsertItem(PCWSTR text, HTREEITEM hParent = TVI_ROOT, HTREEITEM hAfter = TVI_LAST);
	HTREEITEM InsertItem(PCWSTR text, int image, int selectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hAfter = TVI_LAST);
	bool SetItemText(HTREEITEM hItem, PCWSTR text);
	int InsertColumn(int index, PCWSTR text, int width, int format = LVCFMT_LEFT, DWORD_PTR param = 0);
	bool DeleteAllItems();
	bool DeleteItem(HTREEITEM hItem);
	bool SelectItem(HTREEITEM hItem);
	bool Expand(HTREEITEM hItem, UINT code);
	void SetSortColumn(int column, bool ascending);
	void ClearSortColumn();
	bool EnsureVisible(HTREEITEM hItem);
	void Refresh();

	bool Invalidate(BOOL erase = TRUE);
	bool UpdateWindow();

protected:
	// custom draw
	DWORD OnPrePaint(int, LPNMCUSTOMDRAW cd);
	DWORD OnItemPrePaint(int, LPNMCUSTOMDRAW cd);
	//	DWORD OnItemPostPaint( int, LPNMCUSTOMDRAW cd );
	DWORD OnItemPostPaint(int, LPNMCUSTOMDRAW cd);

	void DrawItem(LPNMCUSTOMDRAW cd);
	void DrawItem(CDCHandle dc, HTREEITEM hItem);

private:
	void UpdateScrollBar();
	DWORD DoHeaderItemPrePaint(LPNMCUSTOMDRAW cd);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHeaderNcCalcSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaintTree(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetRedraw(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeSelChanged(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnHeaderItemChanged(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnTrackHeader(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnBeginTrackHeader(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnHeaderItemClick(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnTreeGetDispInfo(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnItemExpanding(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);
	LRESULT OnItemExpanded(int /*idCtrl*/, LPNMHDR /*nymph*/, BOOL& /*bHandled*/);

	CHeaderCtrl m_Header;
	CHeaderCtrl m_TreeHeader;
	CContainedWindowT<CTreeViewCtrl> m_Tree;
	CScrollBar m_HScrollBar;
	TLNMDISPINFO m_di;
	WCHAR m_Text[260]{ 0 };
	int m_SortColumn{ -1 };
	bool m_TreeMode{ true };
};
