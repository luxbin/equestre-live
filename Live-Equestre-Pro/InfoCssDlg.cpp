// InfoCssDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Live-Equestre-Pro.h"
#include "InfoCssDlg.h"
#include "afxdialogex.h"



// CInfoCssDlg dialog

IMPLEMENT_DYNAMIC(CInfoCssDlg, CDialog)

CInfoCssDlg::CInfoCssDlg(CTabCtrl* pParent /*=NULL*/)
	: CDialog(IDD_INFO_CSS_DIALOG, pParent)
{

}
BOOL CInfoCssDlg::Create(CTabCtrl* pParentWnd) {
	return CDialog::Create(ATL_MAKEINTRESOURCE(IDD_INFO_CSS_DIALOG), pParentWnd);
}
CInfoCssDlg::~CInfoCssDlg()
{
}

void CInfoCssDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInfoCssDlg, CDialog)
	

	ON_COMMAND(IDCANCEL, &CInfoCssDlg::OnIdcancel)
	ON_COMMAND(IDOK, &CInfoCssDlg::OnIdok)
//	ON_WM_CREATE()

END_MESSAGE_MAP()


// CInfoCssDlg message handlers



void CInfoCssDlg::OnIdcancel()
{
	// TODO: Add your command handler code here
}


void CInfoCssDlg::OnIdok()
{
	// TODO: Add your command handler code here
}




BOOL CInfoCssDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

