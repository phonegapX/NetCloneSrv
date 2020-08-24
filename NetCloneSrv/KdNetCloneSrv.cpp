// KdNetCloneSrv.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "KdNetCloneSrv.h"
#include "KdNetCloneSrvDlg.h"
#include "NetCardListDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvApp

BEGIN_MESSAGE_MAP(CKdNetCloneSrvApp, CWinApp)
	//{{AFX_MSG_MAP(CKdNetCloneSrvApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvApp construction

CKdNetCloneSrvApp::CKdNetCloneSrvApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CKdNetCloneSrvApp object

CKdNetCloneSrvApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvApp initialization
void ErrorExitProcess()
{
	ExitProcess(NULL);
}


LONG WINAPI ExceptionFilter(_EXCEPTION_POINTERS * ExceptionInfo)
{
	PEXCEPTION_RECORD ExceptionRecord = ExceptionInfo->ExceptionRecord;
	PCONTEXT          ContextRecord   = ExceptionInfo->ContextRecord; 
	CHAR              szErrorMsg[256];
	wsprintf(szErrorMsg, "异常发生位置:%08X,异常代码:%08X", ContextRecord->Eip, ExceptionRecord->ExceptionCode);
	LOGERROR(0, szErrorMsg);
	ContextRecord->Eip = (ULONG)ErrorExitProcess;
	return EXCEPTION_CONTINUE_EXECUTION;
}


BOOL CKdNetCloneSrvApp::InitInstance()
{
	// Standard initialization
	SetUnhandledExceptionFilter(ExceptionFilter);

	m_hMutex = ::CreateMutex(NULL, TRUE, "__KDNETCLONE_MUTEX__");
	if(m_hMutex != NULL && ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		return FALSE;
	}

	WORD    wVersionRequested;
	WSADATA wsaData;
	int     err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return FALSE;
	}

	GetTempPath(sizeof(g_szTempPath), g_szTempPath);
	lstrcat(g_szTempPath, "KeydoneNetClone\\");
	CreateDirectory(g_szTempPath, NULL);

	StartupServiceInit();

	CNetCardListDialog NetCardListDlg;
	int nResponse = NetCardListDlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
		return FALSE;
	}
	NETCARD_INFO      NetInfo = NetCardListDlg.GetCurSelNetInfo();
	CKdNetCloneSrvDlg Maindlg;
	Maindlg.SetCulSelNetInfo(NetInfo);
	m_pMainWnd = &Maindlg;
	nResponse = Maindlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


extern BOOL g_bIsTerminateThread;	//外部变量


int CKdNetCloneSrvApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	g_bIsTerminateThread = TRUE;
	CloseHandle(m_hMutex);
	StartupServiceFree();
	WSACleanup();
	if(g_hGhostProcess != NULL)
	{
		for(int i = 0; i < 3; i++)
		{
			TerminateProcess(g_hGhostProcess, 0);
			if(WaitForSingleObject(g_hGhostProcess, 1000) != WAIT_TIMEOUT)
			{
				g_hGhostProcess = NULL;
				break;
			}
		}
	}
	DeleteDirectory(CString(g_szTempPath));
	return CWinApp::ExitInstance();
}
