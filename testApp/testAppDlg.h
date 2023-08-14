
// testAppDlg.h : header file
//

#pragma once


// CtestAppDlg dialog
class CtestAppDlg : public CDialogEx
{
// Construction
public:
	CtestAppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTAPP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedTest();
	CString m_strProcessName;
	afx_msg void OnBnClickedGetprocessnumber();
	int m_nProcessNum;
	CString m_strFileName;
	LONGLONG m_lOffset;
	DWORD m_dwlength;
	afx_msg void OnBnClickedMakefile();
	afx_msg void OnBnClickedClearmemory();
	afx_msg void OnBnClickedStopclean();
	afx_msg void OnBnClickedButton2();
};
