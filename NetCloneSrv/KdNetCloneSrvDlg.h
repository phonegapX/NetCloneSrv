// KdNetCloneSrvDlg.h : header file
//

#if !defined(AFX_KDNETCLONESRVDLG_H__784FE5AA_9C07_4C1C_9E49_C10F05E15D67__INCLUDED_)
#define AFX_KDNETCLONESRVDLG_H__784FE5AA_9C07_4C1C_9E49_C10F05E15D67__INCLUDED_

#include "StdAfx.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvDlg dialog

class CKdNetCloneSrvDlg : public CDialog
{
// Construction
public:
	void SetCulSelNetInfo(NETCARD_INFO NetInfo);
	CKdNetCloneSrvDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CKdNetCloneSrvDlg)
	enum { IDD = IDD_KDNETCLONESRV_DIALOG };
	CButton	m_btnStart;
	CComboBox	m_SrcPartList;
	CButton	m_radioPart;
	CButton	m_radioDisk;
	CEdit	m_FilePath;
	CComboBox	m_PartList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKdNetCloneSrvDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CKdNetCloneSrvDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRadioDisk();
	afx_msg void OnRadioPart();
	afx_msg void OnButtonBrowse();
	afx_msg void OnButtonStart();
	afx_msg void OnButtonExit();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	NETCARD_INFO m_CurSelNetInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KDNETCLONESRVDLG_H__784FE5AA_9C07_4C1C_9E49_C10F05E15D67__INCLUDED_)
