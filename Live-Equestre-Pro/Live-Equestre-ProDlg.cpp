
// Live-Equestre-ProDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Live-Equestre-Pro.h"
#include "Live-Equestre-ProDlg.h"
#include "afxdialogex.h"
#include <stdlib.h>
#include <string>

#include "Poco/Logger.h"
#include "Poco/SimpleFileChannel.h"
#include "Poco/AutoPtr.h"
#include "Poco/ErrorHandler.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

using Poco::Logger;
using Poco::SimpleFileChannel;
using Poco::AutoPtr;

#include "base/stringutils.h"
#include "EquestreDb.h"
#include "EquestreLogic.h"

#include "MsgProcessor.h"

#include <afxdhtml.h>

#include "CAboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CLiveEquestreProDlg::loadCountries() {
	CFile f;
	if (!PathFileExists(L"countries.txt")) {
		m_countryCount = 0;
		return;
	}
	f.Open(L"countries.txt", CFile::modeRead);
	int len = f.GetLength();
	char* pBuf = new char[len + 1];
	f.Read(pBuf, len);
	CString s(pBuf);
	pBuf[len] = 0;
	int i = 0;
	int subLen = 0;
	int untilTab = 0;
	char sBuf[100];
	bool tabFound = false;
	while (i < len) {
		sBuf[subLen] = pBuf[i];
		if (pBuf[i] == '\r') {
			sBuf[subLen] = 0;
			sBuf[untilTab - 1] = 0;
			m_countryNames[m_countryCount] = sBuf;
			m_countryCodes[m_countryCount] = sBuf + untilTab;
			m_countryCount++;
			untilTab = 0;
			subLen = 0;
			tabFound = false;
		}
		else {
			if (!tabFound) {
				untilTab++;
			}
			if (pBuf[i] == '\t') {
				tabFound = true;
			}
			subLen++;
		}
		i++;
	}
	delete pBuf;
}

// CLiveEquestreProDlg dialog

class MyErrorHandler : public Poco::ErrorHandler
{
public:
	void exception(const Poco::Exception& exc)
	{
		std::cerr << exc.displayText() << std::endl;
		bool a = MyEquestreLogic->IsWebSocketConnected();
		bool b = MyEquestreLogic->IsWebSocketThreadStarted();

		int k = 0;
	}
	void exception(const std::exception& exc)
	{
		std::cerr << exc.what() << std::endl;
	}
	void exception()
	{
		std::cerr << "unknown exception" << std::endl;
	}
};

MyErrorHandler	g_EH;

//////////////////////////////////////////////////////////////////////////


CLiveEquestreProDlg::CLiveEquestreProDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_LIVEEQUESTREPRO_DIALOG, pParent)
	, m_strDB_Directory(_T(""))
	, m_strServer_Address(_T(""))
	, m_nArena(0)
	, m_strServer_Name(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CLiveEquestreProDlg::~CLiveEquestreProDlg()
{
	SaveConfig();
}

void CLiveEquestreProDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Information_Tab, m_Information_Tab);
	DDX_Control(pDX, IDC_DB_Directory_FolderBrowse_BTN, m_Directory_Folder_Browse_BTN);
	DDX_Control(pDX, IDC_DB_Status_Static, m_DB_Status_Static);
	DDX_Control(pDX, IDC_Server_Status_Static, m_Server_Status_Static);
	DDX_Control(pDX, IDC_DB_Directory_Edit, m_DB_Directory_Edit);
	DDX_Control(pDX, IDC_Server_Address_Edit, m_Server_Address_Edit);
	DDX_Control(pDX, IDC_Server_Name_Edit, m_Server_Name_Edit);
	DDX_Control(pDX, IDC_START_BTN, m_Start_Btn);
	DDX_Text(pDX, IDC_DB_Directory_Edit, m_strDB_Directory);
	DDX_Text(pDX, IDC_Server_Address_Edit, m_strServer_Address);
	DDX_Control(pDX, IDC_Clear_BTN, m_Clear_Button);
	DDX_Control(pDX, IDC_Message_BTN, m_Message_Button);
	DDX_Text(pDX, IDC_Server_Name_Edit, m_strServer_Name);
	DDX_Radio(pDX, IDC_RADIO_ARENA, m_nArena);
	DDX_Control(pDX, IDC_RUN_STATUS_STATIC, m_Run_Status_Static);
	DDX_Control(pDX, IDC_LOGO_STATIC, m_Logo_Static);

	DDX_Control(pDX, IDC_COUNTRIES, m_comboCountries);
	DDX_Control(pDX, IDC_CHK_DNS, m_chkDnsWithNoResult);
	DDX_Control(pDX, IDC_DISCIPLINE, m_comboDiscipline);
	DDX_Control(pDX, IDC_CHK_UPLOAD, m_chkUpload);
}

BEGIN_MESSAGE_MAP(CLiveEquestreProDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TCN_SELCHANGE, IDC_Information_Tab, &CLiveEquestreProDlg::OnSelchangeInformationTab)
	ON_NOTIFY(TCN_SELCHANGING, IDC_Information_Tab, &CLiveEquestreProDlg::OnSelchangingInformationTab)
	ON_BN_CLICKED(IDC_DB_Directory_Browse_BTN, &CLiveEquestreProDlg::OnClickedDbDirectoryBrowseBtn)
	ON_BN_CLICKED(IDC_DB_Directory_FolderBrowse_BTN, &CLiveEquestreProDlg::OnClickedDbDirectoryFolderbrowseBtn)
	ON_BN_CLICKED(IDC_START_BTN, &CLiveEquestreProDlg::OnBnClickedStartBtn)
	ON_EN_CHANGE(IDC_DB_Directory_Edit, &CLiveEquestreProDlg::OnChangeDbDirectoryEdit)
	ON_BN_CLICKED(IDC_Server_Browse_BTN, &CLiveEquestreProDlg::OnClickedServerBrowseBtn)
	ON_EN_CHANGE(IDC_Server_Address_Edit, &CLiveEquestreProDlg::OnChangeServerAddressEdit)
	ON_WM_TIMER()
	ON_COMMAND(IDC_RADIO_ARENA, &CLiveEquestreProDlg::OnRadioArena)
	ON_COMMAND(IDC_RADIO_ARENA2, &CLiveEquestreProDlg::OnRadioArena)
	ON_COMMAND(IDC_RADIO_ARENA3, &CLiveEquestreProDlg::OnRadioArena)
	ON_COMMAND(IDC_RADIO_ARENA4, &CLiveEquestreProDlg::OnRadioArena)
	ON_CBN_SELCHANGE(IDC_COUNTRIES, &CLiveEquestreProDlg::OnCbnSelchangeCountries)
	ON_BN_CLICKED(IDC_BTN_EXPORT, &CLiveEquestreProDlg::OnBnClickedBtnExport)
	ON_CBN_SELCHANGE(IDC_DISCIPLINE, &CLiveEquestreProDlg::OnCbnSelchangeDiscipline)
	ON_BN_CLICKED(IDC_BTN_UPLOAD, &CLiveEquestreProDlg::OnBnClickedBtnUpload)
	ON_BN_CLICKED(IDC_BTN_PDF_BROWSER, &CLiveEquestreProDlg::OnBnClickedBtnPdfBrowser)
	ON_BN_CLICKED(IDC_BTN_PDF_UPLOAD, &CLiveEquestreProDlg::OnBnClickedBtnPdfUpload)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CLiveEquestreProDlg message handlers

BOOL CLiveEquestreProDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_chkDnsWithNoResult.SetCheck(1);
	m_chkUpload.SetCheck(1);

	AutoPtr<SimpleFileChannel> pChannel(new SimpleFileChannel);
	pChannel->setProperty("path", "sample.log");
	pChannel->setProperty("rotation", "20 M");
	Logger::root().setChannel(pChannel);

	Poco::ErrorHandler::set(&g_EH);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	InitEdit();

	CPngImage pngImage;
	if (pngImage.Load(MAKEINTRESOURCE(IDB_PNG_LOGO))) {
		// TODO: consider comment line below
		// pngImage.SaveToFile(_T("c:\\test.png"));
		HBITMAP hBkgrBitmap = (HBITMAP)pngImage.Detach();
		m_Logo_Static.SetBitmap(hBkgrBitmap);
	}

	// TODO: consider comment line below
	// m_Directory_Folder_Browse_BTN.SetImage(IDB_BITMAP_Folder);
	
	m_Server_Status_Static.SetStatus(1);
	m_DB_Status_Static.SetStatus(1);
	m_Run_Status_Static.SetStatus(1);

	CreateTabCtrl();
	//theme functions
	m_countryCount = 1;
	m_countryNames[0] = "International";
	m_countryCodes[0] = "INTERNATIONAL";
	loadCountries();
	for (int i = 0; i < m_countryCount; i++) {
		const char* countryName = m_countryNames[i].c_str();
		m_comboCountries.InsertString(i, CString(countryName));
	}
	m_comboCountries.SetCurSel(1);
	MyEquestreDb->SetCountry(m_countryCodes[1]);

	char* pDisciplines[] = { "Auto", "Jumping", "Cross", "Dressage" };

	for (int i = 0; i < 4; i++) {
		m_comboDiscipline.InsertString(i, CString(pDisciplines[i]));
	}
	m_comboDiscipline.SetCurSel(0);



	m_strServer_Name = _T("PC - Chronom¨¦trage (Server) :");
	LoadConfig();

	UpdateData(FALSE);

	OnChangeDbDirectoryEdit();
	OnChangeServerAddressEdit();

	MyEquestreLogic->StartClientSocketThread();
	MyEquestreLogic->StartWebSocketThread();

	SetTimer(1, 1000, NULL);	// Update UI
	SetTimer(2, 1000, NULL);	// Auto Start
	SetTimer(3, 3000, NULL);	// Beep On Error

	VERIFY(m_HtmCtrl.SubclassDlgItem(IDC_STC_BROWSER, this));
	m_HtmCtrl.OnInitDialog();
	//m_HtmCtrl.Navigate(_T("D:\\test.html"));
	m_HtmCtrl.LoadFromResource(IDR_HTML1);
	
	HMENU pSysMenu = ::GetSystemMenu(this->GetSafeHwnd(), FALSE);
	if (pSysMenu)
	{
		::InsertMenu(pSysMenu, 0, MF_BYPOSITION | MF_STRING, IDM_ABOUT, L"About...");
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLiveEquestreProDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLiveEquestreProDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


HBRUSH CLiveEquestreProDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

// 	switch(pWnd->GetDlgCtrlID())
// 	{
// 	case IDC_DB_Directory_Static:
// 	case IDC_Server_Static:
// 	case IDC_STATIC1:
// 	case IDC_STATIC2:
// 		pDC->SetBkMode(TRANSPARENT);
// 		hbr = CreateSolidBrush(RGB(255, 255, 255));
// 		break;
// 	default:
// 		break;
// 	}
	return hbr;
}

void CLiveEquestreProDlg::CreateTabCtrl() {
	
	
	m_Info_Server_Dlg.Create(&m_Information_Tab);
	m_Info_Data_Dlg.Create(&m_Information_Tab);
	m_Info_Css_Dlg.Create(&m_Information_Tab);

	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	tcItem.pszText = _T("Server");
	m_Information_Tab.InsertItem(0, &tcItem);
	tcItem.pszText = _T("Data");
	m_Information_Tab.InsertItem(1, &tcItem);
	tcItem.pszText = _T("CSS");
	m_Information_Tab.InsertItem(2, &tcItem);
	m_Info_Server_Dlg.ShowWindow(TRUE);
}

void CLiveEquestreProDlg::OnSelchangeInformationTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	int i = TabCtrl_GetCurSel(pNMHDR->hwndFrom);
	if (i == 0) {
		m_Info_Server_Dlg.ShowWindow(TRUE);
		m_Info_Data_Dlg.ShowWindow(FALSE);
		m_Info_Css_Dlg.ShowWindow(FALSE);
	}
	else if (i == 1) {
		m_Info_Server_Dlg.ShowWindow(FALSE);
		m_Info_Data_Dlg.ShowWindow(TRUE);
		m_Info_Css_Dlg.ShowWindow(FALSE);
	}
	else if (i == 2) {
		m_Info_Server_Dlg.ShowWindow(FALSE);
		m_Info_Data_Dlg.ShowWindow(FALSE);
		m_Info_Css_Dlg.ShowWindow(TRUE);
	}
}


void CLiveEquestreProDlg::OnSelchangingInformationTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

void CLiveEquestreProDlg::OnClickedDbDirectoryBrowseBtn()
{
	UpdateData();
	CShortCut shortcutDlg;
	shortcutDlg.m_strTitle = CString("Directory Equestre Pro:");
	shortcutDlg.m_strValue = m_strDB_Directory;

	if (shortcutDlg.DoModal() == IDOK) {
		m_DB_Directory_Edit.SetWindowText(shortcutDlg.m_strValue);
	}
}


void CLiveEquestreProDlg::OnClickedDbDirectoryFolderbrowseBtn()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog dlg;
	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		m_DB_Directory_Edit.SetWindowText(str);
	}
}

void CLiveEquestreProDlg::InitEdit()
{
	LOGFONT lf;                        // Used to create the CFont.
	CFont *currentFont = GetFont();
	currentFont->GetLogFont(&lf);
	lf.lfHeight = 20;
	m_fontEdit.CreateFontIndirect(&lf);
	
	m_Start_Btn.SetFont(&m_fontEdit); // Set Start Button Font
	
	m_DB_Directory_Edit.SetFont(&m_fontEdit);
	// m_Server_Name_Edit.SetFont(&m_fontEdit);
	m_Server_Address_Edit.SetFont(&m_fontEdit);
}

void CLiveEquestreProDlg::OnBnClickedStartBtn()
{
	if (MyEquestreLogic->IsStarted()) {
		MyEquestreLogic->Stop();
	}
	else {
		MyEquestreLogic->Start();
	}
	
	m_Start_Btn.SetWindowText(MyEquestreLogic->IsStarted() ? _T("STOP") : _T("START"));

	m_Info_Data_Dlg.EnableUI(!MyEquestreLogic->IsStarted());
	m_Info_Server_Dlg.m_Reset_Button.EnableWindow(MyEquestreLogic->IsStarted());

	m_Clear_Button.EnableWindow(!MyEquestreLogic->IsStarted());
	m_Message_Button.EnableWindow(MyEquestreLogic->IsStarted());
	m_Run_Status_Static.SetStatus(MyEquestreLogic->IsStarted() ? 2 : 1);
}


void CLiveEquestreProDlg::LoadConfig()
{
	m_strDB_Directory = AfxGetApp()->GetProfileString(_T("Config"), _T("EquestreSQLiteDB"), _T(""));
	m_strServer_Address = AfxGetApp()->GetProfileString(_T("Config"), _T("ServerAddress"), _T("localhost"));
	m_nArena = AfxGetApp()->GetProfileInt(_T("Config"), _T("Arena"), 0);
}


void CLiveEquestreProDlg::SaveConfig()
{
	AfxGetApp()->WriteProfileString(_T("Config"), _T("EquestreSQLiteDB"), m_strDB_Directory);
	AfxGetApp()->WriteProfileString(_T("Config"), _T("ServerAddress"), m_strServer_Address);
	AfxGetApp()->WriteProfileInt(_T("Config"), _T("Arena"), m_nArena);
}


void CLiveEquestreProDlg::OnChangeDbDirectoryEdit()
{
	UpdateData();
	std::string folder = ns_base::tostring(wstring(m_strDB_Directory.GetBuffer()), CP_UTF8);

	bool bLoaded = MyEquestreDb->Load(folder);
	MyEquestreDb->UpdateRanking();

	m_DB_Status_Static.SetStatus(bLoaded ? 2 : 1);

}


void CLiveEquestreProDlg::OnClickedServerBrowseBtn()
{
	UpdateData();
	CShortCut shortcutDlg;
	shortcutDlg.m_strTitle = m_strServer_Name;
	shortcutDlg.m_strValue = m_strServer_Address;

	if (shortcutDlg.DoModal() == IDOK) {
		m_Server_Address_Edit.SetWindowText(shortcutDlg.m_strValue);
	}
}


void CLiveEquestreProDlg::OnChangeServerAddressEdit()
{
	UpdateData();
	std::string szAddress = ns_base::tostring(m_strServer_Address.GetBuffer());
	MyEquestreLogic->SetClientSocketServerAddr(szAddress);
}


void CLiveEquestreProDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		// Update UI
		m_Server_Status_Static.SetStatus(MyEquestreLogic->IsClientSocketConnected() ? 2 : 1);
		m_Info_Server_Dlg.UpdateUI();
	}
	else if (nIDEvent == 2) {
		OnBnClickedStartBtn();
		KillTimer(2);
	}
	else if (nIDEvent == 3) {
		// Beep on Error
		if (m_Info_Server_Dlg.m_bSoundOnError) {
			if (MyEquestreLogic->IsStarted() && MyEquestreLogic->IsWebSocketConnected() == false) {
				Beep(523, 500);
			}
		}
	}

	CDialog::OnTimer(nIDEvent);
}


void CLiveEquestreProDlg::OnRadioArena()
{
	UpdateData();
}




void CLiveEquestreProDlg::OnCbnSelchangeCountries()
{
	// TODO: Add your control notification handler code here
	int iSel = m_comboCountries.GetCurSel();
	MyEquestreDb->SetCountry(this->m_countryCodes[iSel]);
}

wstring utf8toUtf16(const string& str)
{
	if (str.empty())
		return wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
		str.data(), (int)str.size(), &buffer[0], buffer.size());
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");

	return wstring(&buffer[0], charsConverted);
}

#include <iostream>
#include <fstream>
#include <Poco/Net/HTTPMessage.h>

char* WC2MB(const WCHAR* wstr, char* strTo)
{

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, (int)wcslen(wstr), NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_UTF8, 0, wstr, (int)wcslen(wstr), strTo, size_needed, NULL, NULL);

	return strTo;
}

void CLiveEquestreProDlg::OnBnClickedBtnExport()
{

	if (!MyEquestreDb->m_bLoaded) {
		MessageBox(_T("No export Data available! Please check the path to Equestre Pro"));
		return;
	}

	const TCHAR szFilter[] = _T("XLS Files (*.xls)|*.xls|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_HIDEREADONLY, szFilter, this);

	CString sXlsFilePath;
	CString sXlsFileName;

	if (dlg.DoModal() == IDOK)
	{
		sXlsFilePath = dlg.GetPathName();
		sXlsFileName = dlg.GetFileName();
	}
	else
	{
		return;
	}

	//********************************************************************************
	// Read XLS file
	//********************************************************************************
	CStdioFile f1;
	CString chLine;

	if (!f1.Open(sXlsFilePath, CFile::modeRead | CFile::typeText))
	{
		MessageBox(_T("Can't open xls file. Please close xls file and try again."));
		return;
	}

	f1.SeekToBegin();
	
	int nRow = 0;
	vector<vector<CString>> csvDatas;
	vector<int> rossonNums;
	while (f1.ReadString(chLine))
	{
		vector<CString> vecLine;

		int nStartPos = 0;

		int nCol = 1;
		bool bFlg = true;
		while (bFlg) {

			int nPos = chLine.Find('\t', nStartPos);

			CString strCell = _T("");

			if (nPos == -1) {

				bFlg = false;
				strCell = chLine.Mid(nStartPos);
			}
			else {
				strCell = chLine.Mid(nStartPos, nPos - nStartPos);
			}

			nStartPos = nPos + 1;

			vecLine.push_back(strCell);

			if (nCol++ == 1 && nRow != 0) {
				rossonNums.push_back(_wtoi(strCell.GetBuffer()));
			}
		}

		csvDatas.push_back(vecLine);

		nRow++;
		
	}
	f1.Close();


	//********************************************************************************
	// Read Score from database
	//********************************************************************************
	string params = MsgProcessorInst->GetRankingList();

	wstring wparams = utf8toUtf16(params);

	CComPtr<IHTMLDocument2> ppHtmlDoc2;
	m_HtmCtrl.GetDHtmlDocument(&ppHtmlDoc2);
	m_webPage.SetDocument(m_HtmCtrl.m_spHtmlDoc);

	CComVariant varResult;

	m_webPage.CallJScript(_T("generateRanking"), wparams.c_str(), &varResult);

	wstring ws(varResult.bstrVal, SysStringLen(varResult.bstrVal));

	string json(ws.begin(), ws.end());

	Poco::JSON::Parser parser;

	Poco::Dynamic::Var result = parser.parse(json);
	Array::Ptr pObject = result.extract<Array::Ptr>();

	vector<vector<string>> vecResult;
	for (int i = 0; i < pObject->size(); i++) {		
		vector<string> row;

		Poco::Dynamic::Var pVar = pObject->get(i);
		Array::Ptr pSubArr = pVar.extract<Array::Ptr>();

		for (int j = 0; j < pSubArr->size(); j++) {
			string ele = pSubArr->get(j).convert<string>();
			row.push_back(ele);
		}
		vecResult.push_back(row);
	}
	

	//********************************************************************************
	// Past Score to XLS 
	//********************************************************************************
	int     roundCount = MyEquestreDb->m_roundNumber;
	int     jumpoffCount = MyEquestreDb->m_jumpoffNumber;
	int		m_twoPhaseDiffered = MyEquestreDb->m_twoPhaseDiffered;

	int COL = 5;
	int ROUND1_COL = 9;
	int ROUND2_COL = 12;
	int STATUS_COL = 11;
	int JUMPOFF_COL = 14;
	int NONSTARTER_COL = 23;
	int HC_COL = 22;

	if (m_twoPhaseDiffered == 1) ROUND2_COL = 14;

	for (int i = 0; i < rossonNums.size(); i++) {
		int rNum = rossonNums[i];

		bool bFound = false;
		for (int row = 0; row < vecResult.size(); row++) {
			if (vecResult[row][1] == std::to_string(rNum)) {

				if (roundCount == 1) {
					for (int k = 0; k < 2; k++) {
						csvDatas[i + 1][ROUND1_COL + k] = utf8toUtf16(vecResult[row][k + COL]).c_str();
					}
				}
				else if (roundCount == 2) {
					for (int k = 0; k < 2; k++) {
						csvDatas[i + 1][ROUND1_COL + k] = utf8toUtf16(vecResult[row][k + COL]).c_str();
					}
					for (int k = 0; k < 2; k++) {
						csvDatas[i + 1][ROUND2_COL + k] = utf8toUtf16(vecResult[row][k + 2 + COL]).c_str();
					}
				}

				if (jumpoffCount > 0) {
					for (int k = 0; k < jumpoffCount * 2; k++) {
						csvDatas[i + 1][JUMPOFF_COL + k] = utf8toUtf16(vecResult[row][k + roundCount * 2 + COL]).c_str();
					}
				}

				csvDatas[i + 1][STATUS_COL] = utf8toUtf16(vecResult[row][2]).c_str();

				if (csvDatas[i + 1][STATUS_COL] == _T("DNS")) {
					csvDatas[i + 1][NONSTARTER_COL] = _T("ns");
				}

				if (csvDatas[i + 1][STATUS_COL] == _T("HC")) {
					csvDatas[i + 1][HC_COL] = _T("hc");
				}

				if (csvDatas[i + 1][STATUS_COL] == _T("NPR")) {
					csvDatas[i + 1][NONSTARTER_COL] = _T("ns");
				}

				break;
			}
		}
	}


	if (m_chkDnsWithNoResult.GetCheck()) {

		for (int row = 0; row < csvDatas.size(); row++) {

			if (!csvDatas[row][0].IsEmpty() && csvDatas[row][STATUS_COL].IsEmpty()) {
				csvDatas[row][NONSTARTER_COL] = _T("ns");
				csvDatas[row][STATUS_COL] = _T("DNS");
			}


		}
	}

	//********************************************************************************
	// Rewrite XLS file. 
	//********************************************************************************

	if (!f1.Open(sXlsFilePath, CFile::modeRead | CFile::modeWrite | CFile::typeText))
	{
		MessageBox(_T("Can't write xls file. Please close xls file and try again."));
		return;
	}
	for (int row = 0; row < csvDatas.size(); row++) {
		CString strLine = _T("");
		for (int col = 0; col < csvDatas[row].size(); col++) {
			strLine += (col == 0 ? _T("") : _T("\t")) + csvDatas[row][col];
		}
		strLine += _T("\n");
		f1.WriteString(strLine);
	}
	f1.Close();


	ShellExecute(0, NULL, sXlsFilePath, NULL, NULL, SW_NORMAL);

	if (m_chkUpload.GetCheck() == 0) return;

	char szFileName[MAX_PATH];
	memset(szFileName, 0, MAX_PATH);

	WC2MB(sXlsFileName.GetBuffer(), szFileName);


	if (!f1.Open(sXlsFilePath, CFile::modeRead | CFile::typeText))
	{
		MessageBox(_T("Can't open xls file. Please close xls file and try again."));
		return;
	}

	f1.SeekToBegin();

	UINT nFileSize = f1.GetLength();

	char* pBuf = new char[nFileSize];
	memset(pBuf, 0, nFileSize);

	f1.Read(pBuf, nFileSize);


	MsgProcessorInst->SendXlsData(szFileName, pBuf);

	f1.Close();

	delete[] pBuf;

}


void CLiveEquestreProDlg::OnCbnSelchangeDiscipline()
{
	MyEquestreDb->m_manualDiscipline = m_comboDiscipline.GetCurSel();

	MyEquestreDb->Reload();
}

void CLiveEquestreProDlg::OnBnClickedBtnUpload()
{
	const TCHAR szFilter[] = _T("XLS Files (*.xls)|*.xls|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_HIDEREADONLY, szFilter, this);

	CString sXlsFilePath;
	CString sXlsFileName;

	if (dlg.DoModal() == IDOK)
	{
		sXlsFilePath = dlg.GetPathName();
		sXlsFileName = dlg.GetFileName();
	}
	else
	{
		return;
	}

	
	char szFileName[MAX_PATH];
	memset(szFileName, 0, MAX_PATH);

	WC2MB(sXlsFileName.GetBuffer(), szFileName);

	CStdioFile f1;

	if (!f1.Open(sXlsFilePath, CFile::modeRead | CFile::typeText))
	{
		MessageBox(_T("Can't open xls file. Please close xls file and try again."));
		return;
	}

	f1.SeekToBegin();

	UINT nFileSize = f1.GetLength();

	char* pBuf = new char[nFileSize];
	memset(pBuf, 0, nFileSize);

	f1.Read(pBuf, nFileSize);


	MsgProcessorInst->SendXlsData(szFileName, pBuf);

	f1.Close();

	delete[] pBuf;
}


void CLiveEquestreProDlg::OnBnClickedBtnPdfBrowser()
{
	const TCHAR szFilter[] = _T("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_HIDEREADONLY, szFilter, this);

	CString sPdfFilePath;
	CString sPdfFileName;

	if (dlg.DoModal() == IDOK)
	{
		sPdfFilePath = dlg.GetPathName();
		sPdfFileName = dlg.GetFileName();
	}
	else
	{
		return;
	}

	GetDlgItem(IDC_EDT_PDF_PATH)->SetWindowText(sPdfFilePath);
}

std::string getFileNameFromPath(const std::string& path) {
	size_t lastSlashIndex = path.find_last_of("/\\");
	if (lastSlashIndex != std::string::npos) {
		return path.substr(lastSlashIndex + 1);
	}
	return path;
}

#include <fstream>
#include "Poco/Base64Encoder.h"
void CLiveEquestreProDlg::OnBnClickedBtnPdfUpload()
{
	char sPdfFilePath[MAX_PATH] = {0};
	char sEventId[MAX_PATH] = {0};

	HWND hPdfPathWnd = GetDlgItem(IDC_EDT_PDF_PATH)->m_hWnd;
	HWND hEventIdWnd = GetDlgItem(IDC_EDT_EVENTID)->m_hWnd;
	
	GetWindowTextA(hPdfPathWnd, sPdfFilePath, MAX_PATH);
	GetWindowTextA(hEventIdWnd, sEventId, MAX_PATH);

	if (sPdfFilePath[0] == 0) {
		MessageBox(_T("Please select the pdf file to upload."));
		return;
	}

	if (sEventId[0] == 0) {
		MessageBox(_T("Please input the Event ID."));
		return;
	}

	std::ifstream in(sPdfFilePath, ios::binary); //
//std::istringstream in(source);
	std::ostringstream out;
	Poco::Base64Encoder b64out(out);

	//std::copy(std::istream_iterator<char>{in},
	//	std::istream_iterator<char>{},
	//	std::ostream_iterator<char>(b64out)
	//);


	while (!in.eof()) {
		char ch;
		in.get(ch);
		b64out << ch;
	}

	b64out.close(); // always call this at the end!

	std::string contents = out.str();

	std::string path = sPdfFilePath;
	std::string filename = getFileNameFromPath(path);

	MsgProcessorInst->SendPdf(sEventId, filename, contents);

	SetWindowTextA(hPdfPathWnd, "");
	//SetWindowTextA(hEventIdWnd, "");

}


void CLiveEquestreProDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == IDM_ABOUT) {
		CAboutDlg dlg;

		dlg.DoModal();
	}

	CDialog::OnSysCommand(nID, lParam);
}
