
// Live-Equestre-ProDlg.h : header file
//

#pragma once
#include <string>
#include "afxcmn.h"
#include "afxwin.h"
#include "afxdlgs.h"
#include "ShortCut.h"

#include "StatusStatic.h"
#include "ImageStatic.h"

#include <afxdhtml.h>

#include "WebPage.h"

// CLiveEquestreProDlg dialog

#define MAX_COUNTRY_COUNT 300

class CLiveEquestreProDlg : public CDialog
{
// Construction
public:
	CLiveEquestreProDlg(CWnd* pParent = NULL);	// standard constructor
	~CLiveEquestreProDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIVEEQUESTREPRO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	std::string m_countryNames[MAX_COUNTRY_COUNT];
	std::string m_countryCodes[MAX_COUNTRY_COUNT];
	int m_countryCount;
	void loadCountries();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelchangeInformationTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSelchangingInformationTab(NMHDR *pNMHDR, LRESULT *pResult);
	
public:
	void CreateTabCtrl();
	void InitEdit();
	
public:
	// sub dialogs
	CInfoServerDlg m_Info_Server_Dlg;
	CInfoDataDlg m_Info_Data_Dlg;
	CInfoCssDlg m_Info_Css_Dlg;

	// main controls
	CTabCtrl m_Information_Tab;
	CButton m_Directory_Folder_Browse_BTN;
	CStatusStatic m_DB_Status_Static;
	CStatusStatic m_Server_Status_Static;
	CEdit m_DB_Directory_Edit;
	CEdit m_Server_Address_Edit;
	CEdit m_Server_Name_Edit;
	CButton m_Start_Btn;
	CStatusStatic m_Run_Status_Static;
	CButton m_Clear_Button;
	CButton m_Message_Button;

	CDHtmlDialog	m_HtmCtrl;

	CWebPage m_webPage;


	// main control values

	// props
	CFont		m_fontEdit;

	afx_msg void OnClickedDbDirectoryBrowseBtn();
	afx_msg void OnClickedDbDirectoryFolderbrowseBtn();
	CString m_strDB_Directory;
	CString m_strServer_Address;
	int m_nArena;
	afx_msg void OnBnClickedStartBtn();
	CString m_strServer_Name;
	void LoadConfig();
	void SaveConfig();
	afx_msg void OnChangeDbDirectoryEdit();
	afx_msg void OnClickedServerBrowseBtn();
	afx_msg void OnChangeServerAddressEdit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRadioArena();
	CStatic m_Logo_Static;

	CComboBox m_comboCountries;
	afx_msg void OnCbnSelchangeCountries();
	afx_msg void OnBnClickedBtnExport();
	CButton m_chkDnsWithNoResult;
	CComboBox m_comboDiscipline;
	afx_msg void OnCbnSelchangeDiscipline();
	afx_msg void OnBnClickedBtnUpload();
	CButton m_chkUpload;
	afx_msg void OnBnClickedBtnPdfBrowser();
	afx_msg void OnBnClickedBtnPdfUpload();
};
