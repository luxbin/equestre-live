#include "stdafx.h"
#include "StatusStatic.h"
#include "Live-Equestre-Pro.h"

CStatusStatic::CStatusStatic()
{
	m_nStatus = 0;
	
}


CStatusStatic::~CStatusStatic()
{
}

void CStatusStatic::SetStatus(int status)
{
	if (m_nStatus == status)
		return;

	ModifyStyle(0, SS_BITMAP);
	if (status == 1) {
		HBITMAP hBitmap = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_Red));
		SetBitmap(hBitmap);
		//DeleteObject(hBitmap);
	}
	else if(status == 2) {
		HBITMAP hBitmap = ::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_Blue));
		SetBitmap(hBitmap);
		//DeleteObject(hBitmap);
	}

	m_nStatus = status;
}