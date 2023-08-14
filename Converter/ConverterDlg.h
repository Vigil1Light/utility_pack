
// ConverterDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CConverterDlg dialog
class CConverterDlg : public CDialogEx
{
// Construction
public:
	CConverterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CONVERTER_DIALOG };

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
	CString m_strResult;
	afx_msg void OnBnClickedConvert();
	CEdit m_ctrlShow;
private:
	LRESULT OnUpdateData(WPARAM wParam, LPARAM);
public:
	afx_msg void OnBnClickedClear();
	BOOL m_bEva;
	BOOL m_bEvs;
	BOOL m_bDat;
};
