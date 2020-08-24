// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KdNetCloneSrv.h"
#include "ServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerDlg dialog


CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	//{{AFX_MSG_MAP(CServerDlg)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerDlg message handlers

void CServerDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	AfxGetMainWnd()->EnableWindow(TRUE);
	AfxGetMainWnd()->SendMessage(WM_CLOSE);
	CDialog::OnClose();
}


void CServerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	// TODO: Add your message handler code here
	delete this;	
}


void CServerDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here

 	HWND hChildWnd = ::FindWindowEx(m_hWnd, NULL, "#32770", NULL);
	if (hChildWnd != NULL) 
	{
		::RedrawWindow(hChildWnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
	}

	// Do not call CDialog::OnPaint() for painting messages
}
