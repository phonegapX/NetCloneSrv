// NetCardListDialog.cpp : implementation file
//

#include "stdafx.h"
#include "KdNetCloneSrv.h"
#include "NetCardListDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNetCardListDialog dialog


CNetCardListDialog::CNetCardListDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNetCardListDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNetCardListDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNetCardListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetCardListDialog)
	DDX_Control(pDX, IDC_STATIC_DHCPTYPE, m_stcDHCPType);
	DDX_Control(pDX, IDC_STATIC_DEVICEDESC, m_stcDeviceDesc);
	DDX_Control(pDX, IDC_STATIC_NETMASK, m_stcNetmask);
	DDX_Control(pDX, IDC_STATIC_IPADDRESS, m_stcIPAddress);
	DDX_Control(pDX, IDC_COMBO_NETCARDLIST, m_cbNetCardList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNetCardListDialog, CDialog)
	//{{AFX_MSG_MAP(CNetCardListDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_CBN_SELCHANGE(IDC_COMBO_NETCARDLIST, OnSelchangeComboNetcardlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetCardListDialog message handlers

BOOL CNetCardListDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	// 这里的代码适合WINDOWS2000，对于NT需要读取HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\NetworkCards
	HKEY hKey, hSubKey, hNdiIntKey;
	
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"System\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}",
		0,
		KEY_READ,
		&hKey) != ERROR_SUCCESS)
	{
		return FALSE;	
	}
	
	DWORD dwNetCardIndex = 0;
	DWORD dwIndex        = 0;
	DWORD dwBufSize      = 256;
	DWORD dwDataType;
	CHAR  szSubKey[256];
	UCHAR szData[256];

	ZeroMemory(m_NetInfoList, sizeof(m_NetInfoList));
	
	while(RegEnumKeyEx(hKey, dwIndex++, szSubKey, &dwBufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		if(RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
		{	
			if(RegOpenKeyEx(hSubKey, "Ndi\\Interfaces", 0, KEY_READ, &hNdiIntKey) == ERROR_SUCCESS)
			{
				dwBufSize = 256;
				if(RegQueryValueEx(hNdiIntKey, "LowerRange", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
				{
					if(strcmp((char*)szData, "ethernet") == 0)		//	判断是不是以太网卡
					{
						dwBufSize = 256;
						if(RegQueryValueEx(hSubKey, "NetCfgInstanceID", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
						{
							CHAR szConnectionName[256] = {0};
							if(RegGetConnectionName((LPCTSTR)szData, szConnectionName) && RegGetIP((LPCTSTR)szData, &m_NetInfoList[dwNetCardIndex]))
							{
								lstrcpy(m_NetInfoList[dwNetCardIndex].strDeviceName, (PCHAR)szData);
								dwBufSize = 256;
								if(RegQueryValueEx(hSubKey, "DriverDesc", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
								{
									lstrcpy(m_NetInfoList[dwNetCardIndex].strDeviceDesc, (PCHAR)szData);
								}
								m_cbNetCardList.InsertString(dwNetCardIndex, szConnectionName);
								m_cbNetCardList.SetItemDataPtr(dwNetCardIndex, &m_NetInfoList[dwNetCardIndex]);
								dwNetCardIndex++;
							}
						}
					}
				}
				RegCloseKey(hNdiIntKey);
			}
			RegCloseKey(hSubKey);
		}
		
		dwBufSize = 256;
	}	/* end of while */
	
	RegCloseKey(hKey);

	if (m_cbNetCardList.GetCount() != 0) 
	{
		m_cbNetCardList.SetCurSel(0);
		OnSelchangeComboNetcardlist();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL IPInRange(PCHAR strIPAddress)
{
	BOOL bResult = FALSE;
	if(strnicmp(strIPAddress, "192.168.88.", strlen("192.168.88.")) == 0)
	{
		CHAR strLastSeg[32];
		int  nLastSeg;
		lstrcpy(strLastSeg, strrchr(strIPAddress, '.')+sizeof(CHAR));
		nLastSeg = atoi(strLastSeg);
		if (nLastSeg >= 2 && nLastSeg <= 255) 
		{
			bResult = TRUE;
		}
	}
	return bResult;
}


void CNetCardListDialog::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	PNETCARD_INFO lpNetInfo = NULL;
	int           uCurSel   = m_cbNetCardList.GetCurSel();
	if (uCurSel == CB_ERR)
	{
		MessageBox("请选择网克所使用的网卡", "错误", MB_ICONERROR);
		return;
	}
	lpNetInfo = (PNETCARD_INFO)m_cbNetCardList.GetItemDataPtr(uCurSel);

	{
		BOOL          bIsExist     = FALSE;
		PNETCARD_INFO existNetInfo = NULL;
		if (m_cbNetCardList.GetCount() > 1) 
		{
			for(int i = 0; i < m_cbNetCardList.GetCount(); i++)
			{
				PNETCARD_INFO lpTempNetInfo = (PNETCARD_INFO)m_cbNetCardList.GetItemDataPtr(i);
				if (stricmp(lpTempNetInfo->strIP, "192.168.88.1") == 0) 
				{
					bIsExist     = TRUE;
					existNetInfo = lpTempNetInfo;
					break;
				}
			}
			if (bIsExist && lpNetInfo != existNetInfo) 
			{
				CHAR szErrorMsg[256];
				wsprintf(szErrorMsg, "网卡:(%s) 已经占用了IP地址192.168.88.1, 不能再对另一块网卡进行设置!!!", existNetInfo->strDeviceDesc);
				MessageBox(szErrorMsg, "错误", MB_ICONERROR);
				return;			
			}
		}
	}

	if (
		strnicmp(lpNetInfo->strIP, "192.168", strlen("192.168")) != 0 ||
		stricmp(lpNetInfo->strNetmask, "255.255.0.0")            != 0 ||
		IPInRange(lpNetInfo->strIP)
		) 
	{
		int nResult = MessageBox("IP必须设置为192.168.XXX.XXX,子网掩码必须设置为255.255.0.0,此网卡不符合此设置,程序将自动将IP设置为192.168.88.1,子网掩码设置为255.255.0.0,是否继续", "提示", MB_ICONQUESTION|MB_YESNO);
		if (nResult != IDYES) 
		{
			return;
		}
		BeginWaitCursor();
		if(
			!RegSetIP(lpNetInfo->strDeviceName, "192.168.88.1", "255.255.0.0", "", "", FALSE) ||
			!NotifyIPChange(lpNetInfo->strDeviceName, 0, "192.168.88.1", "255.255.0.0")
			)
		{
			EndWaitCursor();
			MessageBox("设置失败", "错误", MB_ICONERROR);
			return;
		}
		EndWaitCursor();
		ZeroMemory(lpNetInfo->strIP     , sizeof(lpNetInfo->strIP));
		ZeroMemory(lpNetInfo->strNetmask, sizeof(lpNetInfo->strNetmask));
		lstrcpy(lpNetInfo->strIP     , "192.168.88.1");
		lstrcpy(lpNetInfo->strNetmask, "255.255.0.0");
	}
	m_CurSelNetInfo = *lpNetInfo;
	CDialog::OnOK();
}


void CNetCardListDialog::OnSelchangeComboNetcardlist() 
{
	// TODO: Add your control notification handler code here
	PNETCARD_INFO lpNetInfo = (PNETCARD_INFO)m_cbNetCardList.GetItemDataPtr(m_cbNetCardList.GetCurSel());
	m_stcDeviceDesc.SetWindowText(lpNetInfo->strDeviceDesc);
	m_stcDHCPType.SetWindowText((lpNetInfo->bEnableDHCP ? "IP获取方式:DHCP动态获取" : "IP获取方式:手动设置"));
	m_stcIPAddress.SetWindowText("IP地址    :" + CString(lpNetInfo->strIP));
	m_stcNetmask.  SetWindowText("子网掩码  :" + CString(lpNetInfo->strNetmask));
}


NETCARD_INFO CNetCardListDialog::GetCurSelNetInfo()
{
	return m_CurSelNetInfo;
}
