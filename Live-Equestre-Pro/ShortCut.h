#pragma once


// CShortCut dialog

class CShortCut : public CDialog
{
	DECLARE_DYNAMIC(CShortCut)

public:
	CShortCut(CWnd* pParent = NULL);
	virtual ~CShortCut();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SHORTCUT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_Btn_OK;
	CButton m_Btn_Cancel;

	CString m_strTitle;
	CString m_strValue;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
