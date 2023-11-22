#include "stdafx.h"
#include "ImageStatic.h"
#include "Live-Equestre-Pro.h"
#include <GdiPlus.h>
using namespace Gdiplus;

CImageStatic::CImageStatic()
{
	m_hBkgrBitmap = NULL;
}


CImageStatic::~CImageStatic()
{
}

BEGIN_MESSAGE_MAP(CImageStatic, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CImageStatic::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CStatic::OnPaint() for painting messages

// 	CRect rect;
// 	GetClientRect(&rect);
// 
// 
// 	CPngImage image;
// 	
// 	BOOL loaded = image.Load(IDB_PNG_LOGO, AfxGetInstanceHandle());
// 	CSize size = image.GetBitmapDimension();
// 
// 	// EDIT: Must cast here
// 	//m_pictureLogo.SetBitmap(image.Detach());
// 	HBITMAP bitmap = (HBITMAP)image.Detach();
// 	
// 	CDC m_DC;
// 
// 	m_DC.CreateCompatibleDC(&dc);
// 	m_DC.SelectObject(&bitmap);
// 
// 	dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &m_DC, 0, 0, size.cx, size.cy, SRCCOPY);
// 
// 	
// 	m_Logo_Static.ModifyStyle(0, SS_BITMAP);
// 	m_Logo_Static.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PNG_LOGO)));

}


void CImageStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect;
	GetClientRect(&rect);

	if (m_hBkgrBitmap != NULL) {

		CDC m_DC;
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		m_DC.CreateCompatibleDC(&dc);
		m_DC.SelectObject(&m_hBkgrBitmap);

		dc.SelectStockObject(BLACK_BRUSH);
		dc.Rectangle(&rect);

		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &m_DC, 0, 0, m_sizeBkgrBitmap.cx, m_sizeBkgrBitmap.cy, SRCCOPY);
	}

}


void CImageStatic::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	CStatic::PreSubclassWindow();
	ModifyStyle(0, SS_OWNERDRAW);

}


BOOL CImageStatic::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);
	CStatic::OnEraseBkgnd(pDC);

	if (m_hBkgrBitmap != NULL) {

		CDC m_DC;

		m_DC.CreateCompatibleDC(pDC);
		m_DC.SelectObject(&m_hBkgrBitmap);

		pDC->SelectStockObject(BLACK_BRUSH);
		pDC->Rectangle(&rect);

		pDC->StretchBlt(0, 0, rect.Width(), rect.Height(), &m_DC, 0, 0, m_sizeBkgrBitmap.cx, m_sizeBkgrBitmap.cy, SRCCOPY);
	}
	return FALSE;
		
}


bool CImageStatic::SetImage(int uiBmpResId)
{
	//CBCGPPngImage pngImage;
	CPngImage pngImage;
	if (pngImage.Load(MAKEINTRESOURCE(uiBmpResId)))
	{
		//pngImage.SaveToFile(_T("c:\\test.png"));

		m_hBkgrBitmap = (HBITMAP)pngImage.Detach();
	}
	else
	{
		m_hBkgrBitmap = ::LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(uiBmpResId));
	}
	

	BITMAP bmp;
	::GetObject(m_hBkgrBitmap, sizeof(BITMAP), (LPVOID)&bmp);
	m_sizeBkgrBitmap = CSize(bmp.bmWidth, bmp.bmHeight);

	Invalidate();
	UpdateWindow();

	return false;
}
