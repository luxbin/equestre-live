#pragma once


// CInfoCssDlg dialog

class CInfoCssDlg : public CDialog
{
	DECLARE_DYNAMIC(CInfoCssDlg)

public:
	CInfoCssDlg(CTabCtrl* pParent = NULL);   // standard constructor
	BOOL Create(CTabCtrl* pParentWnd);
	virtual ~CInfoCssDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INFO_CSS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnIdcancel();
	afx_msg void OnIdok();
	virtual BOOL OnInitDialog();
	
};
