#if !defined(AFX_NETCARDLISTDIALOG_H__CD5B6D17_D0AD_4501_9E19_7B9E0D2A3331__INCLUDED_)
#define AFX_NETCARDLISTDIALOG_H__CD5B6D17_D0AD_4501_9E19_7B9E0D2A3331__INCLUDED_

#include "StdAfx.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NetCardListDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNetCardListDialog dialog

class CNetCardListDialog : public CDialog
{
// Construction
public:
	NETCARD_INFO GetCurSelNetInfo();
	CNetCardListDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNetCardListDialog)
	enum { IDD = IDD_DIALOG_NETCARDLIST };
	CStatic	m_stcDHCPType;
	CStatic	m_stcDeviceDesc;
	CStatic	m_stcNetmask;
	CStatic	m_stcIPAddress;
	CComboBox	m_cbNetCardList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetCardListDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNetCardListDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonOk();
	afx_msg void OnSelchangeComboNetcardlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	NETCARD_INFO m_NetInfoList[10];
	NETCARD_INFO m_CurSelNetInfo;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETCARDLISTDIALOG_H__CD5B6D17_D0AD_4501_9E19_7B9E0D2A3331__INCLUDED_)
