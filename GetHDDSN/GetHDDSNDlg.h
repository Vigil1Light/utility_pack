
// GetHDDSNDlg.h : header file
//

#pragma once


// CGetHDDSNDlg dialog
class CGetHDDSNDlg : public CDialogEx
{
// Construction
public:
	CGetHDDSNDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GETHDDSN_DIALOG };

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
	CString m_strHDDSN;
	afx_msg void OnBnClickedGet();
};
