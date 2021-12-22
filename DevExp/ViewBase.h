#pragma once

#include "Interfaces.h"

template<typename T>
struct CViewBase {
	CViewBase(IMainFrame* frame) : m_pFrame(frame) {}

protected:
	IMainFrame* GetFrame() {
		return m_pFrame;
	}

private:
	IMainFrame* m_pFrame;
};
