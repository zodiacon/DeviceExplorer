#include "pch.h"
#include "TreeListViewCtrl.h"

HTREEITEM CTreeListViewCtrl::InsertItem(PCWSTR text, HTREEITEM hParent, HTREEITEM hAfter) {
	return m_Tree.InsertItem(text, hParent, hAfter);
}

HTREEITEM CTreeListViewCtrl::InsertItem(PCWSTR text, int image, int selectedImage, HTREEITEM hParent, HTREEITEM hAfter) {
	return m_Tree.InsertItem(text, image, selectedImage, hParent, hAfter);
}

bool CTreeListViewCtrl::SetItemText(HTREEITEM hItem, PCWSTR text) {
	return m_Tree.SetItemText(hItem, text);
}

int CTreeListViewCtrl::InsertColumn(int index, PCWSTR text, int width, int format, DWORD_PTR param) {
	HDITEM hdItem;
	hdItem.pszText = (PWSTR)text;
	hdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT | HDI_LPARAM;
	hdItem.cxy = width;
	hdItem.lParam = param;
	hdItem.fmt = HDF_STRING | format;
	int column = m_Header.InsertItem(index, &hdItem);
	ATLASSERT(column >= 0);
	if (column == 0) {
		CRect rc, rcHeader;
		HDLAYOUT hl;
		WINDOWPOS wp;
		GetClientRect(&rc);
		hl.prc = &rc;
		hl.pwpos = &wp;
		m_Header.Layout(&hl);
		m_Header.SetWindowPos(wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, wp.flags);
		if (m_TreeHeader.GetItemCount() == 0)
			m_TreeHeader.InsertItem(0, &hdItem);
	}
	return column;
}

bool CTreeListViewCtrl::DeleteAllItems() {
	return m_Tree.DeleteAllItems();
}

bool CTreeListViewCtrl::DeleteItem(HTREEITEM hItem) {
	return m_Tree.DeleteItem(hItem);
}

bool CTreeListViewCtrl::SelectItem(HTREEITEM hItem) {
	return m_Tree.SelectItem(hItem);
}

bool CTreeListViewCtrl::Expand(HTREEITEM hItem, UINT code) {
	return m_Tree.Expand(hItem, code);
}

void CTreeListViewCtrl::SetSortColumn(int column, bool ascending) {
	ATLASSERT(column < m_Header.GetItemCount());
	ClearSortColumn();

	m_SortColumn = column;
	auto header = m_SortColumn == 0 ? m_TreeHeader : m_Header;
	HDITEM h;
	h.mask = HDI_FORMAT;
	header.GetItem(m_SortColumn, &h);
	h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING | (ascending ? HDF_SORTUP : HDF_SORTDOWN);
	header.SetItem(m_SortColumn, &h);
}

void CTreeListViewCtrl::ClearSortColumn() {
	if (m_SortColumn < 0)
		return;

	auto header = m_SortColumn == 0 ? m_TreeHeader : m_Header;
	HDITEM h;
	h.mask = HDI_FORMAT;
	header.GetItem(m_SortColumn, &h);
	h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
	header.SetItem(m_SortColumn, &h);
	m_SortColumn = -1;
}

bool CTreeListViewCtrl::EnsureVisible(HTREEITEM hItem) {
	return m_Tree.EnsureVisible(hItem);
}

void CTreeListViewCtrl::Refresh() {
	m_Tree.RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

bool CTreeListViewCtrl::Invalidate(BOOL erase) {
	m_Tree.Invalidate(erase);
	return m_HScrollBar.Invalidate(erase);
}

bool CTreeListViewCtrl::UpdateWindow() {
	return m_Tree.UpdateWindow();
}

HTREEITEM CTreeListViewCtrl::GetRootItem() const {
	return m_Tree.GetRootItem();
}

HTREEITEM CTreeListViewCtrl::GetParentItem(HTREEITEM hItem) const {
	return m_Tree.GetParentItem(hItem);
}

HTREEITEM CTreeListViewCtrl::GetSelectedItem() const {
	return m_Tree.GetSelectedItem();
}

HTREEITEM CTreeListViewCtrl::GetNextSiblingItem(HTREEITEM hItem) const {
	return m_Tree.GetNextSiblingItem(hItem);
}

HTREEITEM CTreeListViewCtrl::GetPrevSiblingItem(HTREEITEM hItem) const {
	return m_Tree.GetPrevSiblingItem(hItem);
}

CImageList CTreeListViewCtrl::SetImageList(HIMAGELIST hil, int type) {
	return m_Tree.SetImageList(hil, type);
}

int CTreeListViewCtrl::GetItemCount() const {
	return m_Tree.GetCount();
}

int CTreeListViewCtrl::GetColumnCount() const {
	return m_Header.GetItemCount();
}

void CTreeListViewCtrl::SetTreeMode(bool treeMode) {
	if (treeMode != m_TreeMode) {
		m_TreeMode = treeMode;
		Invalidate(FALSE);
	}
}

bool CTreeListViewCtrl::IsTreeMode() const {
	return m_TreeMode;
}

DWORD CTreeListViewCtrl::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CTreeListViewCtrl::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	if (cd->hdr.hwndFrom != m_Tree)
		return DoHeaderItemPrePaint(cd);

	CRect rc;
	if (!::IsRectEmpty(&cd->rc) && m_Header.GetItemRect(0, &rc)) {
		auto tv = (NMTVCUSTOMDRAW*)cd;
		m_di.hItem = (HTREEITEM)tv->nmcd.dwItemSpec;
		m_di.TextColor = m_Tree.GetTextColor();
		m_di.BackColor = m_Tree.GetBkColor();
		m_di.lParam = cd->lItemlParam;
		m_di.column = 0;
		GetParent().SendMessage(WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&m_di));
		tv->clrTextBk = (cd->uItemState & CDIS_FOCUS) == 0 ? m_di.BackColor : ::GetSysColor(COLOR_HIGHLIGHT);
		tv->clrText = (cd->uItemState & CDIS_FOCUS) == 0 ? m_di.TextColor : ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		DrawItem(cd);
		CRect r(rc.right, cd->rc.top, 30000, cd->rc.bottom);
		CDCHandle(cd->hdc).ExcludeClipRect(&r);
	}
	return CDRF_SKIPPOSTPAINT;
}

DWORD CTreeListViewCtrl::OnItemPostPaint(int, LPNMCUSTOMDRAW cd) {
	return CDRF_DODEFAULT;
}

void CTreeListViewCtrl::DrawItem(LPNMCUSTOMDRAW cd) {
	auto tv = (NMTVCUSTOMDRAW*)cd;
	CDCHandle dc(cd->hdc);
	auto hItem = reinterpret_cast<HTREEITEM>(cd->dwItemSpec);
	ATLASSERT(hItem);

	int offset = 0;
	if (m_HScrollBar.IsWindowVisible()) {
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE;
		m_HScrollBar.GetScrollInfo(&si);
		offset = si.nPos - si.nMin;
	}

	CRect rc;
	m_Tree.GetItemRect(hItem, &rc, TRUE);

	CRect rc2;
	HDITEM hdItem;
	hdItem.mask = HDI_LPARAM | HDI_FORMAT;

	auto defaultTextColor = tv->clrText;
	auto defaultBackColor = tv->clrTextBk;

	dc.SetBkMode(TRANSPARENT);

	CBrush back;
	back.CreateSolidBrush(tv->clrTextBk);

	CRect rcHeader;
	m_Header.GetClientRect(&rcHeader);
	rcHeader.top = cd->rc.top;
	rcHeader.bottom = cd->rc.bottom;
	rcHeader.left = rc.right;
	dc.FillRect(&rcHeader, back);

	auto columns = m_Header.GetItemCount();
	for (int c = 1; c < columns; c++) {
		m_di.column = c;
		m_Text[0] = L'\0';
		m_di.TextColor = defaultTextColor;
		m_di.BackColor = defaultBackColor;

		ATLVERIFY(m_Header.GetItem(c, &hdItem));
		m_di.headerParam = hdItem.lParam;
		GetParent().SendMessage(WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&m_di));
		dc.SetTextColor(m_di.TextColor);

		m_Header.GetItemRect(c, &rc2);
		CRect rcDraw(rc2.left, rc.top, rc2.right, rc.bottom);
		rcDraw.OffsetRect(-offset, 0);
		if (m_di.BackColor != defaultBackColor)
			dc.FillSolidRect(&rcDraw, m_di.BackColor);

		DWORD format = DT_LEFT;
		if ((hdItem.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
			format = DT_RIGHT;
		else if ((hdItem.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_CENTER)
			format = DT_CENTER;

		rcDraw.DeflateRect(2, 0);
		dc.SetBkColor(m_di.BackColor);
		dc.DrawText(m_Text, -1, &rcDraw, DT_SINGLELINE | format | DT_END_ELLIPSIS);
	}
}

void CTreeListViewCtrl::DrawItem(CDCHandle dc, HTREEITEM hItem) {
	ATLASSERT(false);
	int offset = 0;
	if (m_HScrollBar.IsWindowVisible()) {
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE;
		m_HScrollBar.GetScrollInfo(&si);
		offset = si.nPos - si.nMin;
	}

	auto columns = m_Header.GetItemCount();
	CRect rc;
	m_Tree.GetItemRect(hItem, &rc, TRUE);

	CRect rc2;
	HDITEM hdItem;
	hdItem.mask = HDI_LPARAM | HDI_FORMAT;
	dc.SetBkMode(TRANSPARENT);

	auto defaultTextColor = m_Tree.GetTextColor();
	auto defaultBackColor = m_Tree.GetBkColor();

	auto hSelected = m_Tree.GetSelectedItem();

	auto backColor = hSelected != hItem ? m_di.BackColor : ::GetSysColor(COLOR_HIGHLIGHT);
	dc.SetTextColor(hSelected != hItem ? m_di.TextColor : ::GetSysColor(COLOR_HIGHLIGHTTEXT));
	CBrush back;
	back.CreateSolidBrush(backColor);

	CRect rcHeader, rcItem;
	m_Header.GetClientRect(&rcHeader);
	m_Tree.GetItemRect(hItem, &rcItem, TRUE);
	rcHeader.top = rcItem.top;
	rcHeader.bottom = rcItem.bottom;
	rcHeader.left = rc.right;
	dc.FillRect(&rcHeader, back);
	m_di.lParam = m_Tree.GetItemData(hItem);
	m_di.hItem = hItem;

	for (int c = 1; c < columns; c++) {
		m_di.column = c;
		m_Text[0] = L'\0';
		m_di.TextColor = defaultTextColor;
		m_di.BackColor = defaultBackColor;

		ATLVERIFY(m_Header.GetItem(c, &hdItem));
		m_di.headerParam = hdItem.lParam;
		GetParent().SendMessage(WM_NOTIFY, 0, reinterpret_cast<LPARAM>(&m_di));

		m_Header.GetItemRect(c, &rc2);
		CRect rcDraw(rc2.left, rc.top, rc2.right, rc.bottom);
		rcDraw.OffsetRect(-offset, 0);

		DWORD format = DT_LEFT;
		if ((hdItem.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
			format = DT_RIGHT;
		else if ((hdItem.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_CENTER)
			format = DT_CENTER;

		rcDraw.DeflateRect(2, 0);
		dc.DrawText(m_Text, -1, &rcDraw, DT_SINGLELINE | format | DT_END_ELLIPSIS);
	}
}

void CTreeListViewCtrl::UpdateScrollBar() {
	// calculate logical width (exclude column 0)
	auto count = m_Header.GetItemCount();
	CRect rcLast, rcFirst, rc;
	GetClientRect(&rc);
	m_Header.GetItemRect(count - 1, &rcLast);
	m_Header.GetItemRect(0, &rcFirst);
	int xmin = rcFirst.right, xmax = rcLast.right;
	int width = rc.right - rcFirst.right;

	if (width <= 0 || width >= xmax - xmin)
		m_HScrollBar.SetWindowPos(nullptr, &rcDefault, SWP_HIDEWINDOW | SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	else {
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.nMax = xmax - 1;
		si.nMin = xmin;
		si.nPage = width;
		si.nPos = si.nMin;
		si.fMask = SIF_PAGE | SIF_RANGE;
		ATLTRACE(L"Scroll range: %d-%d (page: %d)\n", si.nMin, si.nMax, si.nPage);
		m_HScrollBar.SetScrollInfo(&si);
		m_HScrollBar.SetWindowPos(HWND_TOP, rcFirst.right, rc.bottom - ::GetSystemMetrics(SM_CYHSCROLL),
			rc.Width() - rcFirst.right - ::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CYHSCROLL), SWP_NOACTIVATE | SWP_SHOWWINDOW);
	}
}

DWORD CTreeListViewCtrl::DoHeaderItemPrePaint(LPNMCUSTOMDRAW cd) {
	if (cd->dwItemSpec == 0 && cd->hdr.hwndFrom == m_Header) {
		return CDRF_SKIPDEFAULT;
	}
	return CDRF_DODEFAULT;
}

LRESULT CTreeListViewCtrl::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_Tree.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_FULLROWSELECT | TVS_NOHSCROLL | TVS_SHOWSELALWAYS);
	m_Tree.SetExtendedStyle(TVS_EX_DOUBLEBUFFER, 0);
	::SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, 0);

	m_Header.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | HDS_BUTTONS | HDS_HORZ, 0, HeaderId);
	m_Header.SetFont(m_Tree.GetFont());
	m_TreeHeader.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | HDS_BUTTONS | HDS_HORZ);
	m_TreeHeader.SetFont(m_Tree.GetFont());
	m_Header.SetWindowPos(m_TreeHeader, CRect(0, 0, 0, 0), SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	m_HScrollBar.Create(*this, rcDefault, nullptr, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | SBS_HORZ);

	m_di.hdr.hwndFrom = m_hWnd;
	m_di.hdr.code = TLN_GETDISPINFO;
	m_di.cchText = _countof(m_Text);
	m_di.text = m_Text;

	return 0;
}

LRESULT CTreeListViewCtrl::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&) {
	return 1;
}

LRESULT CTreeListViewCtrl::OnHeaderNcCalcSize(UINT, WPARAM wp, LPARAM lp, BOOL&) {
	if (wp && m_HScrollBar.IsWindowVisible()) {
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS | SIF_RANGE;
		m_HScrollBar.GetScrollInfo(&si);
		auto params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lp);
		params->rgrc[0].left -= si.nPos - si.nMin;
		return WVR_ALIGNTOP;
	}
	return DefWindowProc();
}

LRESULT CTreeListViewCtrl::OnSize(UINT, WPARAM, LPARAM lParam, BOOL& bHandled) {
	auto cx = GET_X_LPARAM(lParam), cy = GET_Y_LPARAM(lParam);
	if (m_Header) {
		CRect rc;
		m_Header.GetClientRect(&rc);
		m_Header.MoveWindow(0, 0, cx, rc.bottom);
		m_Tree.MoveWindow(0, rc.bottom, cx, cy - rc.bottom);
		if (m_Header.GetItemCount() > 0) {
			m_Header.GetItemRect(0, &rc);
			m_TreeHeader.MoveWindow(&rc);
		}
		UpdateScrollBar();
	}
	return DefWindowProc();
}

LRESULT CTreeListViewCtrl::OnPaintTree(UINT, WPARAM, LPARAM lParam, BOOL& bHandled) {
	m_Tree.DefWindowProc();
	//if(m_HScrollBar.IsWindowVisible())
	//	m_HScrollBar.Invalidate();
	//CClientDC dc( m_Tree );
	//dc.SelectFont( m_Tree.GetFont() );
	//for( auto hItem = m_Tree.GetFirstVisibleItem(); hItem; hItem = m_Tree.GetNextVisibleItem( hItem ) ) {
	//	DrawItem( dc.m_hDC, hItem );
	//}
	//CRect rc;
	//m_Tree.GetClientRect( &rc );
	//m_Tree.ValidateRect( &rc );

	return 0;
}

LRESULT CTreeListViewCtrl::OnHScroll(UINT, WPARAM wParam, LPARAM, BOOL&) {
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
	m_HScrollBar.GetScrollInfo(&si);
	int pos = si.nPos;

	switch (LOWORD(wParam)) {
		case SB_LINERIGHT:
			si.nPos += 10;
			break;

		case SB_LINELEFT:
			si.nPos -= 10;
			break;

		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;

		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			si.nPos = HIWORD(wParam);
			break;

		case SB_LEFT:
			si.nPos = si.nMin;
			break;

		case SB_RIGHT:
			si.nPos = si.nMax;
			break;
	}
	si.fMask = SIF_POS;
	m_HScrollBar.SetScrollInfo(&si);
	m_HScrollBar.GetScrollInfo(&si);
	if (pos != si.nPos) {
		int scroll = (-pos + si.nPos);
		ATLTRACE(L"Position: %d\n", si.nPos);

		CRect rc, rcFirst, rcLast;
		m_Tree.GetClientRect(&rc);
		m_Header.GetItemRect(0, &rcFirst);
		rc.left = rcFirst.right;
		m_Header.GetItemRect(m_Header.GetItemCount() - 1, &rcLast);
		//rc.OffsetRect(-scroll, 0);
		if (scroll > 0 && rc.left < rcFirst.right + scroll)
			rc.left = rcFirst.right + scroll;
		else if (rc.right > rcLast.right)
			rc.right = rcLast.right;

		m_Tree.ScrollWindowEx(-scroll, 0, SW_INVALIDATE | SW_ERASE, &rc);
		m_Tree.UpdateWindow();

		rc.top = 0;
		rc.bottom = rcFirst.bottom;
		//m_Header.ScrollWindowEx(-scroll, 0, SW_INVALIDATE | SW_ERASE, &rc);
		m_Header.GetClientRect(&rc);
		rc.left -= si.nPos - si.nMin;
		m_Header.MoveWindow(&rc);
	}

	return 0;
}

LRESULT CTreeListViewCtrl::OnSetRedraw(UINT, WPARAM wParam, LPARAM, BOOL&) {
	m_Tree.SetRedraw((BOOL)wParam);
	m_Header.SetRedraw((BOOL)wParam);
	m_TreeHeader.SetRedraw((BOOL)wParam);

	return 0;
}

LRESULT CTreeListViewCtrl::OnTreeSelChanged(int, LPNMHDR, BOOL&) {
	//Invalidate(FALSE);
	return 0;
}

LRESULT CTreeListViewCtrl::OnTrackHeader(int, LPNMHDR hdr, BOOL&) {
	auto item = (NMHEADER*)hdr;
	ATLASSERT(item->pitem->mask & HDI_WIDTH);
	m_Header.SetItem(item->iItem, item->pitem);
	if (item->iItem == 0) {
		m_TreeHeader.MoveWindow(0, 0, item->pitem->cxy, 24);
		m_TreeHeader.SetItem(item->iItem, item->pitem);
	}
	UpdateScrollBar();

	m_Tree.RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN);

	return 0;
}

LRESULT CTreeListViewCtrl::OnBeginTrackHeader(int, LPNMHDR hdr, BOOL&) {
	auto item = (NMHEADER*)hdr;
	//
	// prevent tree header from being resized
	//
	if (hdr->idFrom != HeaderId)
		return TRUE;

	ATLASSERT(item->pitem->mask & HDI_WIDTH);

	return FALSE;
}

LRESULT CTreeListViewCtrl::OnHeaderItemClick(int, LPNMHDR hdr, BOOL&) {
	//
	// forward item click notification to parent
	//
	hdr->hwndFrom = m_hWnd;
	hdr->idFrom = GetWindowLongPtr(GWLP_ID);
	auto item = (NMHEADER*)hdr;
	HDITEM hdItem;
	hdItem.mask = HDI_LPARAM;
	m_Header.GetItem(item->iItem, &hdItem);
	item->iButton = (int)hdItem.lParam;

	return GetParent().SendMessage(WM_NOTIFY, hdr->idFrom, reinterpret_cast<LPARAM>(hdr));
}

LRESULT CTreeListViewCtrl::OnTreeGetDispInfo(int id, LPNMHDR hdr, BOOL&) {
	return GetParent().SendMessage(WM_NOTIFY, id, reinterpret_cast<LPARAM>(hdr));
}

LRESULT CTreeListViewCtrl::OnItemExpanding(int, LPNMHDR, BOOL&) {
	m_Tree.SetRedraw(FALSE);
	return 0;
}

LRESULT CTreeListViewCtrl::OnItemExpanded(int, LPNMHDR, BOOL&) {
	m_Tree.SetRedraw(TRUE);
	return 0;
}

LRESULT CTreeListViewCtrl::OnHeaderItemChanged(int, LPNMHDR, BOOL&) {
	m_Tree.RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN);

	return 0;
}
