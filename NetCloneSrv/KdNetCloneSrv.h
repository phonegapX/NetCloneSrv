// KdNetCloneSrv.h : main header file for the KDNETCLONESRV application
//

#if !defined(AFX_KDNETCLONESRV_H__71430032_B24E_4812_880C_F92B52B1958D__INCLUDED_)
#define AFX_KDNETCLONESRV_H__71430032_B24E_4812_880C_F92B52B1958D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvApp:
// See KdNetCloneSrv.cpp for the implementation of this class
//

class CKdNetCloneSrvApp : public CWinApp
{
public:
	CKdNetCloneSrvApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKdNetCloneSrvApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CKdNetCloneSrvApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HANDLE m_hMutex;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KDNETCLONESRV_H__71430032_B24E_4812_880C_F92B52B1958D__INCLUDED_)
