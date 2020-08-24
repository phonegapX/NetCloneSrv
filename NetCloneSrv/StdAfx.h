// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__AFC40021_5F53_471D_8115_51272AA07BC5__INCLUDED_)
#define AFX_STDAFX_H__AFC40021_5F53_471D_8115_51272AA07BC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxsock.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <process.h>
#include <tlhelp32.h>
#include <Shlwapi.h>

#include "resource.h"		// main symbols

typedef struct tagNETCARD_INFO
{
	CHAR strDeviceName[256];
	CHAR strIP[32];
	CHAR strNetmask[32];
	CHAR strDeviceDesc[256];
	BOOL bEnableDHCP;
} NETCARD_INFO, *PNETCARD_INFO;

extern CHAR         g_szTempPath[MAX_PATH];
extern CHAR         g_szCmdLine[MAX_PATH];
extern CHAR         g_szBootFileName[MAX_PATH];
extern HANDLE       g_hGhostProcess;

void DeleteDirectory(CString strDir);

BOOL ExpandResource(int nId, PCHAR lpResType, PCHAR lpFileName);

void ShellDHCPServiceWrap(LPVOID lParam);

void ShellPXEServiceWrap(LPVOID lParam);

void ShellTFTPServerWrap(LPVOID lParam);

void StartupServiceInit();

void StartupServiceFree();

ULONG WINAPI CreateProcessWorkThread(LPVOID lpParam);

BOOL RegGetIP(LPCTSTR lpszAdapterName, PNETCARD_INFO lpNetInfo);

BOOL RegGetConnectionName(LPCTSTR lpszAdapterName, PCHAR lpConnectionName);

BOOL RegSetIP(LPCTSTR strAdapterName, LPCTSTR strIPAddress, LPCTSTR strNetMask, LPCTSTR strNetGate, LPCTSTR strDNS, BOOL bIsAuto);

BOOL NotifyIPChange(LPCTSTR strAdapterName, int nIndex, LPCTSTR strIPAddress, LPCTSTR strNetMask);

extern BOOL g_bIsWriteErrlog;

void WriteErrorLogFile(ULONG uError, PCHAR lpError);

#define LOGERROR(Err, Text) WriteErrorLogFile(Err, Text)

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__AFC40021_5F53_471D_8115_51272AA07BC5__INCLUDED_)