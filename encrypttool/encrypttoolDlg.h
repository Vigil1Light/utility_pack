
// encrypttoolDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CencrypttoolDlg dialog
class CencrypttoolDlg : public CDialogEx
{
// Construction
public:
	CencrypttoolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ENCRYPTTOOL_DIALOG };

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
	CString m_strDevNumber;
	CEdit m_ctrlDevNumber;
	CButton m_ctrlEncDec;
	afx_msg void OnBnClickedEncdec();

private:
	LRESULT OnUpdateData(WPARAM wParam, LPARAM);
	void WriteEncStaus(CString str);
	BOOL IsEncrypted();
	BOOL IsShowenable();
public:
	CString m_strShow;
	afx_msg void OnBnClickedClear();
	CEdit m_ctrlShow;
	afx_msg void OnBnClickedDecrypt();
};
