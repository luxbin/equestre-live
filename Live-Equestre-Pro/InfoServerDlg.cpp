// InfoServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Live-Equestre-Pro.h"
#include "InfoServerDlg.h"
#include "afxdialogex.h"
#include "EquestreLogic.h"

// CInfoServerDlg dialog

IMPLEMENT_DYNAMIC(CInfoServerDlg, CDialog)


CInfoServerDlg::CInfoServerDlg(CTabCtrl* pParent /*=NULL*/)
	: CDialog(IDD_INFO_Server_DIALOG, pParent)
	, m_bDisplayFlag(FALSE)
	, m_bSoundOnError(FALSE)
	, m_strConnections(_T(""))
	, m_strServerName(_T(""))
	, m_nLanguage(0)
{

}

CInfoServerDlg::~CInfoServerDlg()
{
	SaveConfig();
}

BOOL CInfoServerDlg::Create(CTabCtrl* pParentWnd) {
	return CDialog::Create(ATL_MAKEINTRESOURCE(IDD_INFO_Server_DIALOG), pParentWnd);
}
void CInfoServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Server_Name_Edit, m_Server_Name_Edit);
	DDX_Control(pDX, IDC_WEBSOCK_STATUS_STATIC, m_Server_Status_Static);
	DDX_Control(pDX, IDC_STATUS_STATIC, m_Status_Static);
	DDX_Control(pDX, IDC_BTN_RESET, m_Reset_Button);
	DDX_Check(pDX, IDC_CHECK_FLAG, m_bDisplayFlag);
	DDX_Check(pDX, IDC_CHECK_SOUND, m_bSoundOnError);
	DDX_Text(pDX, IDC_EDIT_CONN, m_strConnections);
	DDX_Text(pDX, IDC_Server_Name_Edit, m_strServerName);
	DDX_CBIndex(pDX, IDC_COMBO_LANG, m_nLanguage);
	DDX_Control(pDX, IDC_Server_Browse_BTN, m_Server_Browser_Button);
}


BEGIN_MESSAGE_MAP(CInfoServerDlg, CDialog)
	ON_COMMAND(IDCANCEL, &CInfoServerDlg::OnIdcancel)
	ON_COMMAND(IDOK, &CInfoServerDlg::OnIdok)
	ON_BN_CLICKED(IDC_CHECK_FLAG, &CInfoServerDlg::UpdateFromUI)
	ON_BN_CLICKED(IDC_CHECK_SOUND, &CInfoServerDlg::UpdateFromUI)
	ON_CBN_SELCHANGE(IDC_COMBO_LANG, &CInfoServerDlg::OnSelchangeComboLang)
	ON_BN_CLICKED(IDC_Server_Browse_BTN, &CInfoServerDlg::OnClickedServerBrowseBtn)
END_MESSAGE_MAP()


// CInfoServerDlg message handlers


void CInfoServerDlg::OnIdcancel()
{
	// TODO: Add your command handler code here
}


void CInfoServerDlg::OnIdok()
{
	// TODO: Add your command handler code here
}


BOOL CInfoServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_strConnections = _T("Connections: 0");
	InitEdit();

	m_Server_Status_Static.SetStatus(1);
	m_Status_Static.SetStatus(1);

	LoadConfig();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CInfoServerDlg::SetStatus(CStatic *BPS, BOOL flag) {
	BPS->ModifyStyle(0, SS_BITMAP);
	if (!flag) {
		BPS->SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_Red)));
	}
	else {
		BPS->SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_Blue)));
	}
}


void CInfoServerDlg::LoadConfig()
{
	m_nLanguage = AfxGetApp()->GetProfileInt(_T("Config"), _T("Language"), 1);
	m_bDisplayFlag = AfxGetApp()->GetProfileInt(_T("Config"), _T("DisplayFlag"), 0) != 0;
	m_bSoundOnError = AfxGetApp()->GetProfileInt(_T("Config"), _T("SoundOnError"), 0) != 0;
}


void CInfoServerDlg::SaveConfig()
{
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("Language"), m_nLanguage);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("DisplayFlag"), m_bDisplayFlag?1:0);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("SoundOnError"), m_bSoundOnError?1:0);
}


void CInfoServerDlg::UpdateFromUI()
{
	UpdateData();
}


void CInfoServerDlg::OnSelchangeComboLang()
{
	UpdateData();
}

void CInfoServerDlg::UpdateUI()
{
 	m_strServerName = MyEquestreLogic->IsWebSocketConnected()? CString(MyEquestreLogic->GetWebSocketUri().c_str()):_T("N.C.");
 	m_Server_Status_Static.SetStatus(MyEquestreLogic->IsWebSocketConnected()?2:1);
	UpdateData(FALSE);
}

void CInfoServerDlg::EnableUI(bool bEnabled)
{

}


void CInfoServerDlg::InitEdit()
{
	LOGFONT lf;                        // Used to create the CFont.
	CFont *currentFont = GetFont();
	currentFont->GetLogFont(&lf);
	lf.lfHeight = 20;
	m_fontEdit.CreateFontIndirect(&lf);

	m_Server_Name_Edit.SetFont(&m_fontEdit);
}


void CInfoServerDlg::OnClickedServerBrowseBtn()
{
// 	UpdateData();
// 	CShortCut shortcutDlg;
// 	shortcutDlg.m_strTitle = m_strServer_Name;
// 	shortcutDlg.m_strValue = m_strServer_Address;
// 
// 	if (shortcutDlg.DoModal() == IDOK) {
// 		m_Server_Address_Edit.SetWindowText(shortcutDlg.m_strValue);
// 	}
}
