#pragma once
#include "afxcmn.h"

// CInfoServerData dialog

class CInfoDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CInfoDataDlg)

public:
	CInfoDataDlg(CTabCtrl* pParent = NULL);   // standard constructor
	BOOL Create(CTabCtrl* pParentWnd);
	virtual ~CInfoDataDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INFO_Data_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnIdcancel();
	afx_msg void OnIdok();
	virtual BOOL OnInitDialog();
	int m_nRowAtStart;
	int m_nRowOnCourse;
	int m_nRowAtFinish;
	int m_nRowRanking;
	BOOL m_bReverse;
	BOOL m_bLive;
	BOOL m_bStartList;
	BOOL m_bRanking;
	void LoadConfig();
	void SaveConfig();
	afx_msg void OnBnClickedCheckReverse();
	afx_msg void OnBnClickedCheckLive();
	afx_msg void OnBnClickedCheckStartlist();
	afx_msg void OnBnClickedCheckRanking();
	afx_msg void OnEnChangeEditAtstart();
	afx_msg void OnEnChangeEditOncourse();
	afx_msg void OnEnChangeEditAtfinish();
	afx_msg void OnEnChangeEditRanking();
	void EnableUI(bool bEnabled);
	CSpinButtonCtrl m_spinAtStart;
	CSpinButtonCtrl m_spinOnCourse;
	CSpinButtonCtrl m_spinAtFinish;
	CSpinButtonCtrl m_spinRanking;
protected:
	BOOL m_bLoaded;
};
