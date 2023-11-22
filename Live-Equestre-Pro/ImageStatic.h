#pragma once
#include "afxwin.h"
class CImageStatic :
	public CStatic
{
public:
	CImageStatic();
	virtual ~CImageStatic();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void PreSubclassWindow();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	
	HBITMAP m_hBkgrBitmap;
	CSize m_sizeBkgrBitmap;

public:
	bool SetImage(int nResID);
};

