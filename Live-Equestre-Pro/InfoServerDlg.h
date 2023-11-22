#pragma once
#include "afxwin.h"
#include "StatusStatic.h"
// CInfoServerDlg dialog

class CInfoServerDlg : public CDialog
{
	DECLARE_DYNAMIC(CInfoServerDlg)

public:

	CInfoServerDlg(CTabCtrl* pParent = NULL);   // standard constructor
	BOOL Create(CTabCtrl* pParentWnd);
	virtual ~CInfoServerDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INFO_Server_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnIdcancel();
	afx_msg void OnIdok();
	virtual BOOL OnInitDialog();
public:
	void SetStatus(CStatic *BPS, BOOL flag);
	CEdit m_Server_Name_Edit;
	CStatusStatic m_Server_Status_Static;
	CStatusStatic m_Status_Static;
	CButton m_Reset_Button;

	CFont		m_fontEdit;

	BOOL m_bDisplayFlag;
	BOOL m_bSoundOnError;
	CString m_strConnections;
	CString m_strServerName;
	int m_nLanguage;

	void LoadConfig();
	void SaveConfig();
	afx_msg void UpdateFromUI();
	afx_msg void OnSelchangeComboLang();
	void UpdateUI();

	void EnableUI(bool bEnabled);
	void InitEdit();
	CButton m_Server_Browser_Button;
	afx_msg void OnClickedServerBrowseBtn();
};
