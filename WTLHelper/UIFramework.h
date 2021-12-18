#pragma once

#include <vector>
#include <atlstr.h>
#include <atlbase.h>
#include <atlcom.h>
#include "ColumnManager.h"

namespace UI {
	enum class ColumnAlignment {
		Left = LVCFMT_LEFT,
		Center = LVCFMT_CENTER,
		Right = LVCFMT_RIGHT,
	};

	struct ColumnInfo {
		CString Name;
		uint32_t Tag;
		ColumnAlignment Alignment;
		ColumnFlags Flags;
		int Width;
	};

	struct SortData {
		int Column;
		bool Ascending;
	};

	struct __declspec(uuid("B6F4FF2B-6F33-42FB-98C7-DC17CF3B2207")) IWindow {
		virtual HWND GetHwnd() const = 0;
	};

	struct __declspec(uuid("1C068FA2-7B7E-431A-939B-9ECE8BC82A70")) IMainFrame : IWindow {
		virtual bool TrackPopupMenu(HMENU hMenu, DWORD flags, int x, int y) = 0;
	};

	struct __declspec(uuid("6A364267-AB97-40FD-B3AA-E42A248C59A3")) IAsyncOperation {
		virtual void Completed(bool success) = 0;
	};

	struct __declspec(uuid("55F550A7-19FF-4CE4-AE41-CAE881CD0DDF")) IServiceHost {
		virtual bool GetService(REFIID riid, void** ppv) = 0;
	};

	struct __declspec(uuid("4EB2F1DE-2DFA-416C-9341-870BD8235AF6")) IViewHost {
		virtual bool Init(IServiceHost* pSvcProvider) = 0;
	};

	struct __declspec(uuid("35A19838-FC04-4FA0-9D85-446BC3AE4E2C")) IViewProvider {
		virtual bool Refresh() = 0;
		virtual bool RefreshAsync(IAsyncOperation* operation) {
			return false;
		}
	};

	struct __declspec(uuid("4EB2F1DE-2DFA-416C-9341-870BD8235AF5")) IListViewProvider : IViewProvider {
		virtual int GetItemCount() = 0;
		virtual CString GetColumnText(int row, int col) = 0;
		virtual PCWSTR GetConstColumnText(int row, int col) const {
			return nullptr;
		}
		virtual bool OnContextMenu(int row, int col, POINT const& pt) {
			return false;
		}

		virtual bool OnDoubleClick(int row, int col, POINT const& pt) {
			return false;
		}

		virtual std::vector<ColumnInfo> GetColumns() = 0;
		virtual bool CanSort(int col) const {
			return true;
		}
		virtual void Sort(SortData const& si) {}
	};

	struct ITreeViewHost abstract {
	};

	struct IListViewHost abstract {
	};

	struct __declspec(uuid("7A02302E-7425-48F6-AF6F-AF09ADF28290")) ITreeViewProvider : IViewProvider {
		virtual bool OnContextMenu(HTREEITEM hItem, POINT const& pt) {
			return false;
		}
	};

	struct IViewInit abstract {
		virtual bool Init(IViewHost* host) = 0;
	};

	struct IExplorerViewProvider abstract : IViewInit {
		virtual IListViewProvider* GetListViewProvider() {
			return nullptr;
		}
		virtual ITreeViewProvider* GetTreeViewProvider() {
			return nullptr;
		}
	};

}