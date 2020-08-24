// KdNetCloneSrvDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KdNetCloneSrv.h"
#include "KdNetCloneSrvDlg.h"
#include "ServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TimeEventId   0x1234
#define TimeEventId2  0x1235

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvDlg dialog

CKdNetCloneSrvDlg::CKdNetCloneSrvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKdNetCloneSrvDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKdNetCloneSrvDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CKdNetCloneSrvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKdNetCloneSrvDlg)
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_COMBO_GHOPARTLIST, m_SrcPartList);
	DDX_Control(pDX, IDC_RADIO_PART, m_radioPart);
	DDX_Control(pDX, IDC_RADIO_DISK, m_radioDisk);
	DDX_Control(pDX, IDC_EDIT_PATH, m_FilePath);
	DDX_Control(pDX, IDC_COMBO_PARTLIST, m_PartList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CKdNetCloneSrvDlg, CDialog)
	//{{AFX_MSG_MAP(CKdNetCloneSrvDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_RADIO_DISK, OnRadioDisk)
	ON_BN_CLICKED(IDC_RADIO_PART, OnRadioPart)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, OnButtonExit)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKdNetCloneSrvDlg message handlers

BOOL CKdNetCloneSrvDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	m_SrcPartList.InsertString(0, "镜像包中第 1 个分区");
	m_SrcPartList.SetItemData (0, 1);
	m_SrcPartList.InsertString(1, "镜像包中第 2 个分区");
	m_SrcPartList.SetItemData (1, 2);
	m_SrcPartList.InsertString(2, "镜像包中第 3 个分区");
	m_SrcPartList.SetItemData (2, 3);
	m_SrcPartList.InsertString(3, "镜像包中第 4 个分区");
	m_SrcPartList.SetItemData (3, 4);
	m_SrcPartList.SetCurSel(0);

	m_PartList.InsertString(0, "工作站 C 盘");
	m_PartList.SetItemData (0, 1);
	m_PartList.InsertString(1, "工作站 D 盘");
	m_PartList.SetItemData (1, 2);
	m_PartList.InsertString(2, "工作站 E 盘");
	m_PartList.SetItemData (2, 3);
	m_PartList.InsertString(3, "工作站 F 盘");
	m_PartList.SetItemData (3, 4);
	m_PartList.SetCurSel(0);

	m_radioPart.SetCheck(BST_CHECKED);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CKdNetCloneSrvDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


HCURSOR CKdNetCloneSrvDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CKdNetCloneSrvDlg::OnRadioDisk() 
{
	// TODO: Add your control notification handler code here
	m_PartList.EnableWindow(FALSE);
	m_SrcPartList.EnableWindow(FALSE);
}


void CKdNetCloneSrvDlg::OnRadioPart() 
{
	// TODO: Add your control notification handler code here
	m_PartList.EnableWindow();
	m_SrcPartList.EnableWindow();
}


void CKdNetCloneSrvDlg::OnButtonBrowse() 
{
	// TODO: Add your control notification handler code here
	CString filt="GHO文件 (*.GHO)|*.GHO||";
	CFileDialog fileDlg(TRUE, NULL, NULL, NULL, filt, this);
	fileDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;
	fileDlg.m_ofn.lpstrTitle = "打开";
	if (fileDlg.DoModal() == IDOK)
	{
		HANDLE hFile = CreateFile(fileDlg.GetPathName().GetBuffer(0), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			ULARGE_INTEGER uFileSize;
			uFileSize.u.LowPart = GetFileSize(hFile, &uFileSize.u.HighPart);
			if (uFileSize.QuadPart > (1024 * 1024))
			{
				CloseHandle(hFile);
				m_FilePath.SetWindowText(fileDlg.GetPathName());
				return;
			}
			CloseHandle(hFile);
		}
		MessageBox("请选择一个有效的镜像文件", "错误", MB_ICONERROR);
	}
}


void CKdNetCloneSrvDlg::OnButtonStart() 
{
	// TODO: Add your control notification handler code here

	CHAR    szGHOPathName[MAX_PATH];	
	CHAR    szFullFileName[MAX_PATH];
	CHAR    szPxeFileName[MAX_PATH];
	CHAR    szImgFileName[MAX_PATH];
	int     uPxeResID;
	int     uImgResID;
	HANDLE  hThread;
	ULONG   dwThreadId;
	CString strGHOPathName;

	m_btnStart.EnableWindow(FALSE);

	m_FilePath.GetWindowText(strGHOPathName);
	strGHOPathName.TrimLeft();
	strGHOPathName.TrimRight();
	if (strGHOPathName.GetLength() == 0)
	{
		MessageBox("请选择GHO镜像文件", "错误", MB_ICONERROR);
		goto __ERROR;
	}
	lstrcpy(szGHOPathName, strGHOPathName.GetBuffer(0));
	CreateDirectory(g_szTempPath, NULL);
	if (m_radioDisk.GetCheck() == 0)
	{
		ULONG uData = m_PartList.GetItemData(m_PartList.GetCurSel());
		switch(uData)
		{
		case 1:
			lstrcpy(szPxeFileName, g_szTempPath);
			lstrcat(szPxeFileName, "netghost-c.pxe");
			lstrcpy(g_szBootFileName, "netghost-c.pxe");
			uPxeResID = IDR_PXE_C;
			lstrcpy(szImgFileName, g_szTempPath);
			lstrcat(szImgFileName, "netghos-c.img");
			uImgResID = IDR_IMG_C;
			break;
		case 2:
			lstrcpy(szPxeFileName, g_szTempPath);
			lstrcat(szPxeFileName, "netghost-d.pxe");
			lstrcpy(g_szBootFileName, "netghost-d.pxe");
			uPxeResID = IDR_PXE_D;
			lstrcpy(szImgFileName, g_szTempPath);
			lstrcat(szImgFileName, "netghos-d.img");
			uImgResID = IDR_IMG_D;
			break;
		case 3:
			lstrcpy(szPxeFileName, g_szTempPath);
			lstrcat(szPxeFileName, "netghost-e.pxe");
			lstrcpy(g_szBootFileName, "netghost-e.pxe");
			uPxeResID = IDR_PXE_E;
			lstrcpy(szImgFileName, g_szTempPath);
			lstrcat(szImgFileName, "netghos-e.img");
			uImgResID = IDR_IMG_E;
			break;
		case 4:
			lstrcpy(szPxeFileName, g_szTempPath);
			lstrcat(szPxeFileName, "netghost-f.pxe");
			lstrcpy(g_szBootFileName, "netghost-f.pxe");
			uPxeResID = IDR_PXE_F;
			lstrcpy(szImgFileName, g_szTempPath);
			lstrcat(szImgFileName, "netghos-f.img");
			uImgResID = IDR_IMG_F;
			break;
		}
	}
	else if (m_radioPart.GetCheck() == 0)
	{
		lstrcpy(szPxeFileName, g_szTempPath);
		lstrcat(szPxeFileName, "netghost-hd.pxe");
		lstrcpy(g_szBootFileName, "netghost-hd.pxe");
		uPxeResID = IDR_PXE_HD;
		lstrcpy(szImgFileName, g_szTempPath);
		lstrcat(szImgFileName, "netghos-hd.img");
		uImgResID = IDR_IMG_HD;
	}
	if (!ExpandResource(uPxeResID, "PXE", szPxeFileName) || !ExpandResource(uImgResID, "IMG", szImgFileName)) 
	{
		MessageBox("文件无法展开", "错误", MB_ICONERROR);
		goto __ERROR;
	}
	lstrcpy(szFullFileName, g_szTempPath);
	lstrcat(szFullFileName, "ProtectHook.dll");
	if(!ExpandResource(IDR_SRV_DLL, "SRV", szFullFileName))
	{
		MessageBox("文件无法展开", "错误", MB_ICONERROR);
		goto __ERROR;
	}
	lstrcpy(szFullFileName, g_szTempPath);
	lstrcat(szFullFileName, "KdGhostSrv.exe");
	if(!ExpandResource(IDR_SRV, "SRV", szFullFileName))
	{
		MessageBox("文件无法展开", "错误", MB_ICONERROR);
		goto __ERROR;
	}
	if (m_radioDisk.GetCheck() == 0)
	{
		ULONG uData = m_SrcPartList.GetItemData(m_SrcPartList.GetCurSel());
		wsprintf(g_szCmdLine, "%s \"%s\" keydonesrv -p%u", szFullFileName, szGHOPathName, uData);
	}
	else if (m_radioPart.GetCheck() == 0)
	{
		wsprintf(g_szCmdLine, "%s \"%s\" keydonesrv", szFullFileName, szGHOPathName);
	}
	hThread = CreateThread(NULL, 0, CreateProcessWorkThread, m_hWnd, 0, &dwThreadId);
	if (hThread != NULL) 
	{
		CloseHandle(hThread);
	}
	SetTimer(TimeEventId,  100, NULL);
	SetTimer(TimeEventId2, 100, NULL);
	_beginthread(ShellDHCPServiceWrap, 0, &m_CurSelNetInfo);
	_beginthread(ShellPXEServiceWrap,  0, &m_CurSelNetInfo);
	_beginthread(ShellTFTPServerWrap,  0, &m_CurSelNetInfo);
	return;
__ERROR:
	m_btnStart.EnableWindow(TRUE);
	return;
}


void CKdNetCloneSrvDlg::OnButtonExit()
{
	// TODO: Add your control notification handler code here
	DestroyWindow();
}


void CKdNetCloneSrvDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	static BOOL bWindowCreateOK = FALSE;
	if (nIDEvent == TimeEventId && !bWindowCreateOK)
	{
		HWND hGhostSrvWnd = ::FindWindow(NULL, "keydonesrv - Symantec GhostCast 服务器");
		if (hGhostSrvWnd != NULL)
		{
			HWND hChildWnd = ::FindWindowEx(hGhostSrvWnd, NULL, "#32770", NULL);
			if (hChildWnd != NULL)
			{
				HWND hMoreWnd = ::FindWindowEx(hChildWnd, NULL, "Button", "更多选项(&O) >>");
				if (hMoreWnd != NULL)
				{
					::EnableWindow(hMoreWnd, FALSE);
					::ShowWindow(hMoreWnd, SW_HIDE);
				}
				::ShowWindow(hGhostSrvWnd, SW_HIDE);
				RECT Rect;
				::GetWindowRect(hChildWnd, &Rect);
				CServerDlg * ServerDlg = new CServerDlg;
				ServerDlg->Create(IDD_DIALOG_SERVER, this);
				ServerDlg->ShowWindow(SW_SHOW);
				::EnableWindow(m_hWnd, FALSE);			//模拟模式对话框
				Rect.bottom += 20;
				ServerDlg->MoveWindow(&Rect);
				Rect.bottom -= 20;
				::SetParent(hChildWnd, ServerDlg->m_hWnd);
				::MoveWindow(hChildWnd, 0, 0, Rect.right-Rect.left, Rect.bottom-Rect.top, TRUE);
				bWindowCreateOK = TRUE;
				KillTimer(TimeEventId);
			}
		}
	}
	else if (nIDEvent == TimeEventId2) 
	{
		HWND hWnd = ::FindWindow("#32770", "Symantec GhostCast 服务器");
		if (hWnd != NULL)
		{
			if (::FindWindowEx(hWnd, NULL, "Static", "请求的分区无效。") != NULL)
			{
				::SetForegroundWindow(hWnd);
			}
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CKdNetCloneSrvDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
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
}


void CKdNetCloneSrvDlg::SetCulSelNetInfo(NETCARD_INFO NetInfo)
{
	m_CurSelNetInfo = NetInfo;
}
