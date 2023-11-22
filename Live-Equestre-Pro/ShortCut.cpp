// ShortCut.cpp : implementation file
//

#include "stdafx.h"
#include "Live-Equestre-Pro.h"
#include "ShortCut.h"
#include "afxdialogex.h"


// CShortCut dialog

IMPLEMENT_DYNAMIC(CShortCut, CDialog)

CShortCut::CShortCut(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DLG_SHORTCUT, pParent)
{
	
}

CShortCut::~CShortCut()
{
}

void CShortCut::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_Btn_OK);
	DDX_Control(pDX, IDCANCEL, m_Btn_Cancel);
}


BEGIN_MESSAGE_MAP(CShortCut, CDialog)
	ON_BN_CLICKED(IDOK, &CShortCut::OnBnClickedOk)
END_MESSAGE_MAP()


// CShortCut message handlers


BOOL CShortCut::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO: consider comment lines below
	// m_Btn_OK.SetImage(IDB_CHECK);
	// m_Btn_Cancel.SetImage(IDB_CHECKNO);
	SetDlgItemText(IDC_TITLE_STATIC, m_strTitle);
	SetDlgItemText(IDC_SHORCUT_TEXT, m_strValue);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CShortCut::OnBnClickedOk()
{
	GetDlgItemText(IDC_SHORCUT_TEXT, m_strValue);
	OnOK();
}
