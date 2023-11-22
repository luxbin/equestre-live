// InfoServerData.cpp : implementation file
//

#include "stdafx.h"
#include "Live-Equestre-Pro.h"
#include "InfoDataDlg.h"
#include "afxdialogex.h"



// CInfoServerData dialog

IMPLEMENT_DYNAMIC(CInfoDataDlg, CDialog)

CInfoDataDlg::CInfoDataDlg(CTabCtrl* pParent /*=NULL*/)
	: CDialog(IDD_INFO_Data_DIALOG, pParent)
	, m_nRowAtStart(0)
	, m_nRowOnCourse(0)
	, m_nRowAtFinish(0)
	, m_nRowRanking(0)
	, m_bReverse(FALSE)
	, m_bLive(FALSE)
	, m_bStartList(FALSE)
	, m_bRanking(FALSE)
	, m_bLoaded(FALSE)
{

}
BOOL CInfoDataDlg::Create(CTabCtrl* pParentWnd) {
	return CDialog::Create(ATL_MAKEINTRESOURCE(IDD_INFO_Data_DIALOG), pParentWnd);
}
CInfoDataDlg::~CInfoDataDlg()
{
	SaveConfig();
}

void CInfoDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ATSTART, m_nRowAtStart);
	DDX_Text(pDX, IDC_EDIT_ONCOURSE, m_nRowOnCourse);
	DDX_Text(pDX, IDC_EDIT_ATFINISH, m_nRowAtFinish);
	DDX_Text(pDX, IDC_EDIT_RANKING, m_nRowRanking);
	DDX_Check(pDX, IDC_CHECK_REVERSE, m_bReverse);
	DDX_Check(pDX, IDC_CHECK_LIVE, m_bLive);
	DDX_Check(pDX, IDC_CHECK_STARTLIST, m_bStartList);
	DDX_Check(pDX, IDC_CHECK_RANKING, m_bRanking);
	DDX_Control(pDX, IDC_SPIN_ATSTART, m_spinAtStart);
	DDX_Control(pDX, IDC_SPIN_ONCOURSE, m_spinOnCourse);
	DDX_Control(pDX, IDC_SPIN_ATFINISH, m_spinAtFinish);
	DDX_Control(pDX, IDC_SPIN_RANKING, m_spinRanking);
}


BEGIN_MESSAGE_MAP(CInfoDataDlg, CDialog)
	ON_COMMAND(IDCANCEL, &CInfoDataDlg::OnIdcancel)
	ON_COMMAND(IDOK, &CInfoDataDlg::OnIdok)
	ON_BN_CLICKED(IDC_CHECK_REVERSE, &CInfoDataDlg::OnBnClickedCheckReverse)
	ON_BN_CLICKED(IDC_CHECK_LIVE, &CInfoDataDlg::OnBnClickedCheckLive)
	ON_BN_CLICKED(IDC_CHECK_STARTLIST, &CInfoDataDlg::OnBnClickedCheckStartlist)
	ON_BN_CLICKED(IDC_CHECK_RANKING, &CInfoDataDlg::OnBnClickedCheckRanking)
	ON_EN_CHANGE(IDC_EDIT_ATSTART, &CInfoDataDlg::OnEnChangeEditAtstart)
	ON_EN_CHANGE(IDC_EDIT_ONCOURSE, &CInfoDataDlg::OnEnChangeEditOncourse)
	ON_EN_CHANGE(IDC_EDIT_ATFINISH, &CInfoDataDlg::OnEnChangeEditAtfinish)
	ON_EN_CHANGE(IDC_EDIT_RANKING, &CInfoDataDlg::OnEnChangeEditRanking)
END_MESSAGE_MAP()




void CInfoDataDlg::OnIdcancel()
{
	// TODO: Add your command handler code here
}


void CInfoDataDlg::OnIdok()
{
	// TODO: Add your command handler code here
}


BOOL CInfoDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	m_spinAtStart.SetRange(0, 10);
	m_spinAtFinish.SetRange(0, 4);
	m_spinOnCourse.SetRange(0, 4);
	m_spinRanking.SetRange(0, 40);

	LoadConfig();

	UpdateData(FALSE);
	
	m_bLoaded = true;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CInfoDataDlg::LoadConfig()
{
	m_nRowAtStart = AfxGetApp()->GetProfileInt(_T("Config"), _T("RowAtStart"), 0);
	m_nRowOnCourse = AfxGetApp()->GetProfileInt(_T("Config"), _T("RowOnCourse"), 0);
	m_nRowAtFinish = AfxGetApp()->GetProfileInt(_T("Config"), _T("RowAtFinish"), 0);
	m_nRowRanking = AfxGetApp()->GetProfileInt(_T("Config"), _T("RowRanking"), 0);

	m_bReverse = (AfxGetApp()->GetProfileInt(_T("Config"), _T("ReverseData"), 0) != 0);
	m_bLive = (AfxGetApp()->GetProfileInt(_T("Config"), _T("LiveData"), 1) != 0);
	m_bStartList = (AfxGetApp()->GetProfileInt(_T("Config"), _T("StartListData"), 0) != 0);
	m_bRanking = (AfxGetApp()->GetProfileInt(_T("Config"), _T("RankingData"), 0) != 0);
}


void CInfoDataDlg::SaveConfig()
{
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("RowAtStart"), m_nRowAtStart);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("RowOnCourse"), m_nRowOnCourse);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("RowAtFinish"), m_nRowAtFinish);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("RowRanking"), m_nRowRanking);

	AfxGetApp()->WriteProfileInt(_T("Config"), _T("ReverseData"), m_bReverse ? 1 : 0);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("LiveData"), m_bLive ? 1 : 0);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("StartListData"), m_bStartList ? 1 : 0);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("RankingData"), m_bRanking ? 1 : 0);

}


void CInfoDataDlg::OnBnClickedCheckReverse()
{
	UpdateData();
}


void CInfoDataDlg::OnBnClickedCheckLive()
{
	UpdateData();
}


void CInfoDataDlg::OnBnClickedCheckStartlist()
{
	UpdateData();
}


void CInfoDataDlg::OnBnClickedCheckRanking()
{
	UpdateData();
}


void CInfoDataDlg::OnEnChangeEditAtstart()
{
	if (!m_bLoaded)
		return;

	UpdateData();
	if (m_nRowAtStart < 0) {
		m_nRowAtStart = 0;
	}
	else if (m_nRowAtStart > 10) {
		m_nRowAtStart = 10;
	}
	UpdateData(FALSE);
}


void CInfoDataDlg::OnEnChangeEditOncourse()
{
	if (!m_bLoaded)
		return;
	UpdateData();
	if (m_nRowOnCourse < 0) {
		m_nRowOnCourse = 0;
	}
	else if (m_nRowOnCourse > 4) {
		m_nRowOnCourse = 4;
	}
	UpdateData(FALSE);
}


void CInfoDataDlg::OnEnChangeEditAtfinish()
{
	if (!m_bLoaded)
		return;
	UpdateData();
	if (m_nRowAtFinish < 0) {
		m_nRowAtFinish = 0;
	}
	else if (m_nRowAtFinish > 4) {
		m_nRowAtFinish = 4;
	}
	UpdateData(FALSE);
}


void CInfoDataDlg::OnEnChangeEditRanking()
{
	if (!m_bLoaded)
		return;
	UpdateData();
	if (m_nRowRanking < 0) {
		m_nRowRanking = 0;
	}
	else if (m_nRowRanking > 40) {
		m_nRowRanking = 40;
	}
	UpdateData(FALSE);
}


void CInfoDataDlg::EnableUI(bool bEnabled)
{
	GetDlgItem(IDC_CHECK_REVERSE)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHECK_STARTLIST)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHECK_RANKING)->EnableWindow(bEnabled);

	GetDlgItem(IDC_EDIT_ATSTART)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_ATFINISH)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_ONCOURSE)->EnableWindow(bEnabled);
	GetDlgItem(IDC_EDIT_RANKING)->EnableWindow(bEnabled);

	GetDlgItem(IDC_SPIN_ATSTART)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SPIN_ATFINISH)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SPIN_ONCOURSE)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SPIN_RANKING)->EnableWindow(bEnabled);

}
