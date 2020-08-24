// stdafx.cpp : source file that includes just the standard includes
//	KdNetCloneSrv.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#pragma comment(lib, "Shlwapi.lib")

#define CommMmMapName  TEXT("__KDNETCLONE_COMM_MEMMAP__")

BOOL         g_bIsTerminateThread   = FALSE;
CHAR         g_szTempPath[MAX_PATH];
CHAR         g_szCmdLine[MAX_PATH];
CHAR         g_szBootFileName[MAX_PATH];
HANDLE       g_hCommMmMap;
PVOID        g_pCommMmMap;
HANDLE       g_hGhostProcess = NULL;


BOOL ExpandResource(int nId, PCHAR lpResType, PCHAR lpFileName)
{
	PCHAR Name = MAKEINTRESOURCE(nId);
	HRSRC res = ::FindResource(NULL, Name, lpResType);
	if (res == NULL)
	{
		return FALSE;
	}
	HGLOBAL gl = ::LoadResource(NULL,res);
	if (gl == NULL)
	{
		return FALSE;
	}
	LPVOID lp = ::LockResource(gl);					//����ָ����Դ�ڴ�ĵ�ַ��ָ�롣
	if (lp == NULL)
	{
		return FALSE;	
	}
	HANDLE hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;	
	}
	DWORD dwSizeRes = SizeofResource(NULL, res);	//�õ���Դ�ļ��Ĵ�С
	if (dwSizeRes == 0)
	{
		return FALSE;
	}
	DWORD Out = 0;
	if(!WriteFile(hFile, lp, dwSizeRes, &Out, NULL))
	{
		return FALSE;	
	}
	CloseHandle(hFile);								//�رվ��
	FreeResource(gl);								//�ͷ��ڴ�
	return TRUE;
}


void DeleteDirectory(CString strDir)   
{   
	if(strDir.IsEmpty())
	{   
		RemoveDirectory(strDir);   
		return;   
	}   
	//   ����ɾ���ļ������ļ���   
	CFileFind ff;   
	BOOL      bFound = ff.FindFile(strDir + "\\*", 0);
	while(bFound)   
	{   
		bFound = ff.FindNextFile();   
		if(ff.GetFileName() == "." || ff.GetFileName() == "..")   
			continue;   
		//   ȥ���ļ�(��)ֻ��������   
		SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL);   
		if(ff.IsDirectory())   
		{   //   �ݹ�ɾ�����ļ���   
			DeleteDirectory(ff.GetFilePath());   
			RemoveDirectory(ff.GetFilePath());   
		}   
		else
		{   
			DeleteFile(ff.GetFilePath());   //   ɾ���ļ�   
		}   
	}   
	ff.Close();   
	//   Ȼ��ɾ�����ļ���   
	RemoveDirectory(strDir);   
}


BOOL NetCardIsExist(PCHAR lpIPAddress)
{
	BOOL             bResult = FALSE;
	char             szHostName[128];      //�����������ƴ���һά����,��������ΪszHostName
	struct hostent * pHost;                //����ṹ�� hostent
	int              i;                    //�������i
	if(gethostname(szHostName, 128) == 0)  //������������Ʋ鵽��������������List�ؼ�
	{
		pHost = gethostbyname(szHostName); 
		for(i = 0; pHost != NULL && pHost->h_addr_list[i] != NULL; i++) 	
		{
			LPCSTR IPAddress=inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
			if (inet_addr(lpIPAddress) == (*(struct in_addr *)pHost->h_addr_list[i]).S_un.S_addr) 
			{
				bResult = TRUE;
				break;
			}
		}
	}
	return bResult;
}

//-----------------------------------------------------------------
//	�õ�ע����е�IP��Ϣ
//
//-----------------------------------------------------------------
BOOL RegGetIP(LPCTSTR lpszAdapterName, PNETCARD_INFO lpNetInfo)
{
	HKEY    hKey;
	BOOL    bEnableDHCP;
	CString strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
	strKeyName += lpszAdapterName;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKeyName.GetBuffer(NULL), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;	
	}
	unsigned char szData[256];
	DWORD         dwDataType, dwBufSize;
	DWORD         dwData;
	dwBufSize = 4;
	if(RegQueryValueEx(hKey, "EnableDHCP", 0, &dwDataType, (LPBYTE)&dwData, &dwBufSize) == ERROR_SUCCESS)
	{
		bEnableDHCP = (BOOL)dwData;	
	}
	lpNetInfo->bEnableDHCP = bEnableDHCP;
	if (bEnableDHCP) 
	{
		ZeroMemory(szData, sizeof(szData));
		dwBufSize = 256;
		if(RegQueryValueEx(hKey, "DhcpIPAddress", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
		{
			lstrcpy(lpNetInfo->strIP, (LPCTSTR)szData);		
		}
		ZeroMemory(szData, sizeof(szData));	
		dwBufSize = 256;
		if(RegQueryValueEx(hKey, "DhcpSubnetMask", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
		{
			lstrcpy(lpNetInfo->strNetmask, (LPCTSTR)szData);		
		}
	}
	else
	{
		ZeroMemory(szData, sizeof(szData));
		dwBufSize = 256;
		if(RegQueryValueEx(hKey, "IPAddress", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
		{
			lstrcpy(lpNetInfo->strIP, (LPCTSTR)szData);		
		}
		ZeroMemory( szData,sizeof(szData) );
		dwBufSize = 256;
		if(RegQueryValueEx(hKey, "SubnetMask", 0, &dwDataType, szData, &dwBufSize) == ERROR_SUCCESS)
		{
			lstrcpy(lpNetInfo->strNetmask, (LPCTSTR)szData);		
		}
	}
	RegCloseKey(hKey);
	return NetCardIsExist(lpNetInfo->strIP);
}


BOOL RegGetConnectionName(LPCTSTR lpszAdapterName, PCHAR lpConnectionName)
{
	HKEY    hKey;
	CString strKeyName = "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\";
	strKeyName += lpszAdapterName;
	strKeyName += "\\Connection";
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKeyName.GetBuffer(NULL), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	DWORD         dwDataType;
	DWORD         dwBufSize   = 256;
	UCHAR         szData[256] = {0};
	if(RegQueryValueEx(hKey, "Name", 0, &dwDataType, (LPBYTE)&szData, &dwBufSize) == ERROR_SUCCESS)
	{
		lstrcpy(lpConnectionName, (PCHAR)szData);
		return TRUE;
	}
	return FALSE;
}


typedef ULONG (CALLBACK * HOOKROC)(PVOID);


ULONG StartHookApi()
{
	HANDLE           hSnap;
	HANDLE           hTarget;        //��ע����̵ľ��
	PROCESSENTRY32   pe; 
	BOOL             bNext;
	HANDLE           hToken;
	TOKEN_PRIVILEGES stTokenPriv;
	LUID             Luid;
	LPVOID           lpBuffer;
	FARPROC          fpLoadLibrary;
	CHAR             strDllPath[MAX_PATH];
	
	GetTempPath(sizeof(strDllPath), strDllPath);
	lstrcat(strDllPath, "KeydoneNetClone\\ProtectHook.dll");
	
	//���� OpenProcessToken ��ȡ�뵱ǰ���̹����Ĵ�ȡ���ơ�
	//OpenProcessToken ������һ�����̵ķ��ʴ��š�
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		return GetLastError();
	}
	
	//LookupPrivilegeValue������ñ���Ψһ�ı�ʾ��(LUID)���������ض���ϵͳ�б�ʾ�ض�������Ȩ�� 
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid))
	{
		return GetLastError();
	}
	
	stTokenPriv.PrivilegeCount           = 1;
	stTokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	stTokenPriv.Privileges[0].Luid       = Luid;
	if (!AdjustTokenPrivileges(hToken, FALSE, &stTokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		return GetLastError();
	}
	
	pe.dwSize = sizeof(pe);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	bNext = Process32First(hSnap, &pe); 
	while(bNext) 
	{
		if(stricmp(pe.szExeFile, "KdGhostSrv.exe") == 0)
		{
			hTarget = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_WRITE|PROCESS_VM_OPERATION, TRUE, pe.th32ProcessID);
			if (hTarget == NULL) 
			{
				CloseHandle(hSnap);
				return GetLastError();
			}
			
			//������������ʹ��VirtualAllocEx��������һ���ڴ�.
			lpBuffer = VirtualAllocEx(hTarget, NULL, lstrlen(strDllPath), MEM_COMMIT, PAGE_READWRITE);
			if (lpBuffer == NULL) 
			{
				CloseHandle(hTarget);
				CloseHandle(hSnap);
				return GetLastError();
			}
			
			if (!WriteProcessMemory(hTarget, lpBuffer, strDllPath, lstrlen(strDllPath), NULL)) 
			{
				CloseHandle(hTarget);
				CloseHandle(hSnap);
				return GetLastError();
			}
			
			//�õ� LoadLibraryA �ĵ�ַ.
			fpLoadLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
			if (fpLoadLibrary == NULL) 
			{
				CloseHandle(hTarget);
				CloseHandle(hSnap);
				return GetLastError();
			}

			//��Ŀ������м���ģ��.
			HOOKROC fpHookProc = *((HOOKROC*)(PULONG)&fpLoadLibrary);
			if (CreateRemoteThread(hTarget, NULL, 0, fpHookProc, lpBuffer, 0, NULL) == NULL) 
			{
				CloseHandle(hTarget);
				CloseHandle(hSnap);
				return GetLastError();
			}
			CloseHandle(hTarget);
		}
		bNext = Process32Next(hSnap, &pe);
	}
	CloseHandle(hSnap);
	return ERROR_SUCCESS;
}


void SetDefalutReg()
{
	HKEY hKey;
	ULONG uError = RegCreateKey(HKEY_CURRENT_USER, "Software\\Symantec\\Symantec Ghost\\Symantec GhostCast ������\\Settings", &hKey);
	if (uError == ERROR_SUCCESS)
	{
		DWORD dwValue = 0;
		RegSetValueEx(hKey, "ExtraOptions", 0, REG_DWORD, (PBYTE)&dwValue, sizeof(dwValue));
		dwValue = 1;
		RegSetValueEx(hKey, "InhibitSendReminder", 0, REG_DWORD, (PBYTE)&dwValue, sizeof(dwValue));
		RegCloseKey(hKey);
	}
}
/*
//=========================================================================================
//=========================================================================================
#define _WIN32_WINNT 0x0500
#include <winioctl.h>

#define dwBytesPerSectore 512

#include <pshpack1.h>

typedef struct _PARTITION_ENTRY
{
	UCHAR active;                 // �ܷ�������־
	UCHAR StartHead;              // �÷�����ʼ��ͷ��
	UCHAR StartSector;            // ��ʼ����Ÿ�2λ��6λ��ʼ������
	UCHAR StartCylinder;          // ��ʼ����ŵ�8λ
	UCHAR PartitionType;          // ��������
	UCHAR EndHead;                // �÷�����ֹ��ͷ��
	UCHAR EndSector;              // ��ֹ����Ÿ�2λ��6λ��ֹ������
	UCHAR EndCylinder;            // ��ֹ����ŵ�8λ
	ULONG StartLBA;               // ��ʼ������
	ULONG TotalSector;            // �����ߴ磨����������
} PARTITION_ENTRY, *PPARTITION_ENTRY;

//==============================================================================
typedef struct _MBR_SECTOR
{
	UCHAR            BootCode[446];
	PARTITION_ENTRY  Partition[4];
	USHORT           Signature;
} MBR_SECTOR, *PMBR_SECTOR;

//==============================================================================
typedef struct _BBR_SECTOR
{
	USHORT JmpCode;               // 2�ֽ���תָ��,��ת����������
	UCHAR  NopCode;               // 1�ֽ�nopָ��,�����,��֤��תָ�3���ֽ�
	UCHAR  OEMName[8];            // 8�ֽڵ�OEMName
	
	// ���濪ʼΪ: BPB( BIOS Parameter Block )
	
	USHORT BytesPerSector;        // ÿ���������ֽ��� (512 1024 2048 4096)
	UCHAR  SectorsPerCluster;     // ÿ���ص������� ( 1 2 4 8 16 32 64 128 )������˲��ܳ���32K(������С)
	USHORT ReservedSectors;       // �Ӿ�ĵ�һ��������ʼ�ı���������Ŀ,��ֵ����Ϊ0������FAT12/FAT16����ֵͨ��Ϊ1,����FAT32������ֵΪ32
	UCHAR  NumberOfFATs;          // ����FAT���ݽṹ����Ŀ����ֵͨ��ӦΪ2,[NTFS��ʹ��NumberOfFATs�ֶΣ�����Ϊ0]
	USHORT RootEntries;           // ����FAT12/FAT16,��ֵ��ʾ32�ֽ�Ŀ¼�����Ŀ,����FAT32����ֵ����Ϊ0��[NTFS��ʹ��]
	USHORT NumberOfSectors16;     // �þ��ϵ��������������ֶο���Ϊ0��������ֶ�Ϊ0����NumberOfSectors32����Ϊ0������FAT32�����ֶα���Ϊ0 [FAT32/NTFS��ʹ�ø��ֶ�]
	UCHAR  MediaDescriptor;       // ��������
	USHORT SectorsPerFAT16;       // ���ֶα�ʶһ��FAT�ṹռ�е���������FAT12/FAT16��,����FAT32�����ֶα���Ϊ0��[FAT32/NTFS��ʹ�ø��ֶ�]
	USHORT SectorsPerTrack;       // ����INT 0x13�жϵ�ÿ���ŵ���������
	USHORT HeadsPerCylinder;      // ����INT 0x13�жϵ�ÿ������Ĵ�ͷ��
	ULONG  HiddenSectors;         // ������FAT��ķ���֮ǰ������������
	ULONG  NumberOfSectors32;     // ���ֶΰ����þ��ϵ�����������Ŀ������FAT32�����ֶβ�Ϊ0��FAT12/FAT16�ɸ���ʵ�ʴ�С�Ƿ񳬹�65536�������������Ƿ���ø��ֶΣ� [NTFS��ʹ�ø��ֶ�]
	
	// ���濪ʼΪ: EBPB ( Extended BIOS Parameter Block )
	
	ULONG  SectorsPerFAT32;       // ����FAT32�����ֶΰ���һ��FAT�Ĵ�С����SectorsPerFAT16�ֶα���Ϊ0;
} BBR_SECTOR, *PBBR_SECTOR;

#include <poppack.h>

#define PARTITION_TYPE_NTFS         0x07
#define PARTITION_TYPE_FAT32        0x0B
#define PARTITION_TYPE_FAT32_LBA    0x0C

#define STR_DSKDEVICE_NAME          TEXT("\\\\.\\PhysicalDrive0")

DWORD ProtectExeFile(LPCTSTR strExeFile)
{
	STARTING_VCN_INPUT_BUFFER  iVcnBuf;
	UCHAR                      oVcnBuf[272];
	PRETRIEVAL_POINTERS_BUFFER lpVcnBuf;
	DWORD                      dwVcnExtents;
	LARGE_INTEGER              startLcn;
	PUCHAR                     lpClusterBuf;
	DWORD                      dwClusterLen;
	UCHAR                      dataBuf[dwBytesPerSectore];
	UCHAR                      diskBuf[dwBytesPerSectore];
	DWORD                      dataLen;
	LARGE_INTEGER              diskPos;
	PPARTITION_ENTRY           lpPartition;
	ULONG                      dwPartitionStart;
	ULONG                      dwPartitionType;
	PBBR_SECTOR                lpBootSector;
	DWORD                      SectorsPerCluster;
	HANDLE                     hDskDevice;
	HANDLE                     hExeFile;
	DWORD                      errCode = ERROR_SUCCESS;
	CHAR                       szFilePath[MAX_PATH];

	lstrcpy(szFilePath, strExeFile);
	*(strstr(szFilePath, "\\KdGhostSrv.exe") + lstrlen("\\KdGhostSrv.exe")) = 0;

	if(INVALID_HANDLE_VALUE == (hExeFile = CreateFileA(szFilePath, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL)))
	{
		errCode = GetLastError();
		goto FunExit01;
	}
	iVcnBuf.StartingVcn.QuadPart = 0;
	RtlZeroMemory(oVcnBuf, sizeof(oVcnBuf));
	if(!DeviceIoControl(hExeFile, FSCTL_GET_RETRIEVAL_POINTERS, &iVcnBuf, sizeof(iVcnBuf), &oVcnBuf[0], sizeof(oVcnBuf), &dataLen, NULL))
	{
		errCode = GetLastError();
		goto FunExit02;
	}
	lpVcnBuf = (PRETRIEVAL_POINTERS_BUFFER)&oVcnBuf[0];
	dwVcnExtents = lpVcnBuf->ExtentCount;
	startLcn     = lpVcnBuf->Extents[0].Lcn;
	if(!dwVcnExtents)
	{
		errCode = (ULONG)(-3); // �ļ�̫С, ���ܲ���
		goto FunExit02;
	}
	if(startLcn.QuadPart == -1)
	{
		errCode = (ULONG)(-4); // ���ļ���ѹ���ļ�, ���ܲ���
		goto FunExit02;
	}
	ReadFile(hExeFile, dataBuf, sizeof(dataBuf), &dataLen, NULL);
	// �򿪵�һ������Ӳ��
	if(INVALID_HANDLE_VALUE == (hDskDevice = CreateFileA(STR_DSKDEVICE_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)))
	{
		errCode = GetLastError();
		goto FunExit02;
	}
	// ��ȡӲ�̵�һ������(MBR)
	SetFilePointer(hDskDevice, 0, NULL, FILE_BEGIN);
	ReadFile(hDskDevice, diskBuf, sizeof(diskBuf), &dataLen, NULL);
	lpPartition = &(((PMBR_SECTOR)&diskBuf[0])->Partition[0]);
	if(lpPartition[0].active != 0x80)
	{
		errCode = (ULONG)(-1); // ����������������
		goto FunExit03;
	}
	dwPartitionType = lpPartition[0].PartitionType;
	if(
		dwPartitionType != PARTITION_TYPE_FAT32
		&&
		dwPartitionType != PARTITION_TYPE_FAT32_LBA
		&&
		dwPartitionType != PARTITION_TYPE_NTFS
	)
	{
		errCode = (ULONG)(-2); // ��֧�ֵĴ��̷���
		goto FunExit03;
	}
	dwPartitionStart = lpPartition[0].StartLBA;
	diskPos.QuadPart = dwPartitionStart * dwBytesPerSectore;
	// ��ȡ���������ĵ�һ������(�ļ�ϵͳ��������)
	SetFilePointer(hDskDevice, diskPos.LowPart, &diskPos.HighPart, FILE_BEGIN);
	ReadFile(hDskDevice, diskBuf, sizeof(diskBuf), &dataLen, NULL);
	lpBootSector = (PBBR_SECTOR)&diskBuf[0];
	SectorsPerCluster = lpBootSector->SectorsPerCluster;
	// ����FAT32/NTFS����Userinit����ʼ�ص�ƫ����
	diskPos.QuadPart = dwPartitionStart;
	diskPos.QuadPart+= lpBootSector->ReservedSectors;
	if(dwPartitionType == PARTITION_TYPE_FAT32 || dwPartitionType == PARTITION_TYPE_FAT32_LBA)
	{
		diskPos.QuadPart+= lpBootSector->NumberOfFATs * lpBootSector->SectorsPerFAT32;
	}
	diskPos.QuadPart += startLcn.QuadPart * SectorsPerCluster;
	diskPos.QuadPart *= dwBytesPerSectore;
	// ����ļ�Ѱַ
	SetFilePointer(hDskDevice, diskPos.LowPart, &diskPos.HighPart, FILE_BEGIN);
	ReadFile(hDskDevice, diskBuf, sizeof(diskBuf), &dataLen, NULL);
	if(!RtlEqualMemory(dataBuf, diskBuf, sizeof(diskBuf)))
	{
		errCode = (ULONG)(-5); // Ѱַ�ļ����ɹ�
		goto FunExit03;
	}
	// ���仺��
	dwClusterLen = SectorsPerCluster * dwBytesPerSectore;
	lpClusterBuf = (PUCHAR)GlobalAlloc(GMEM_ZEROINIT, dwClusterLen); // ����һ������Ҫ�Ļ���
	if(!lpClusterBuf)
	{
		errCode = GetLastError(); // Ѱַ�ļ����ɹ�
		goto FunExit03;
	}

	


	// дVirus�ļ������ݵ�����
	SetFilePointer(hDskDevice, diskPos.LowPart, &diskPos.HighPart, FILE_BEGIN);
	WriteFile(hDskDevice, lpClusterBuf, dwClusterLen, &dataLen, NULL);
	FlushFileBuffers(hDskDevice);
	FlushFileBuffers(hExeFile);
	errCode = ERROR_SUCCESS;
FunExit04:
	GlobalFree(lpClusterBuf);
FunExit03:
	CloseHandle(hDskDevice);
FunExit02:
	CloseHandle(hExeFile);
FunExit01:
FunExit00:
	return errCode;
}

//=========================================================================================
//=========================================================================================
*/
ULONG WINAPI CreateProcessWorkThread(LPVOID lpParam)
{
	HWND                hWnd = (HWND)lpParam;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
	g_hCommMmMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4, CommMmMapName);
	if(g_hCommMmMap != NULL)
	{
		g_pCommMmMap = MapViewOfFile(g_hCommMmMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(g_pCommMmMap != NULL)
		{
			*((PULONG)g_pCommMmMap) = GetCurrentProcessId();
			SHDeleteKey(HKEY_CURRENT_USER, "Software\\Symantec");
			SetDefalutReg();
			if(CreateProcess(NULL, g_szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
			{
				g_hGhostProcess = pi.hProcess;
				Sleep(500);
				StartHookApi();
	//			ProtectExeFile(g_szCmdLine);
				WaitForSingleObject(pi.hProcess, INFINITE);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
			else
			{
				MessageBox(hWnd, "���񴴽�ʧ��", "����", MB_ICONERROR);
			}
		}
	}
	DeleteDirectory(CString(g_szTempPath));
	ExitProcess(0);
    return 0;
}


BOOL g_bIsWriteErrlog = TRUE;


void WriteErrorLogFile(ULONG uError, PCHAR lpError)
{
	CHAR       szErrorMessage[1024];
	CHAR       szTime[100];
	CHAR       szFilePath[MAX_PATH];
	DWORD      dwByteOfWritten;
	SYSTEMTIME stSystemTime;
	PCHAR      x;
	HANDLE     hFile;
	if (!g_bIsWriteErrlog)
	{
		return;
	}
	GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));
	x = strrchr(szFilePath, '\\') + 1;
	*x = 0;
	lstrcat(szFilePath, "KdNetCloneSrv.txt");
	hFile = CreateFile(szFilePath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	GetLocalTime(&stSystemTime);
	wsprintf(szTime, "%u-%02u-%02u %02u:%02u:%02u", stSystemTime.wYear, stSystemTime.wMonth, stSystemTime.wDay, stSystemTime.wHour, stSystemTime.wMinute, stSystemTime.wSecond);
	SetFilePointer(hFile, 0, NULL, FILE_END);
	wsprintf(szErrorMessage, "%s  ErrorCode: %5u  %s\r\n", szTime, uError, lpError);
	WriteFile(hFile, szErrorMessage, lstrlen(szErrorMessage), &dwByteOfWritten, NULL);
	CloseHandle(hFile);
}

//-----------------------------------------------------------------
//	����ע����е�IP��Ϣ
//-----------------------------------------------------------------
BOOL RegSetIP(LPCTSTR strAdapterName, LPCTSTR strIPAddress, LPCTSTR strNetMask, LPCTSTR strNetGate, LPCTSTR strDNS, BOOL bIsAuto)
{
	HKEY    hKey;
	PCHAR   pTemp      = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\%s";
	CString strKeyName = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
	strKeyName        += strAdapterName;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKeyName.GetBuffer(0), 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	CHAR mszIPAddress[100];
	CHAR mszNetMask[100];
	CHAR mszNetGate[100];
	CHAR mszDns[100];
	CHAR szMetric[50];
	ZeroMemory(szMetric,     sizeof(szMetric));
	ZeroMemory(mszIPAddress, sizeof(mszIPAddress));
	ZeroMemory(mszNetMask,   sizeof(mszNetMask));
	lstrcpy(mszIPAddress, "0.0.0.0");
	lstrcpy(mszNetMask,   "0.0.0.0");
	if (bIsAuto)		//�Զ���ȡ
	{
		lstrcpy(mszDns , strDNS);
		RegSetValueEx(hKey, "IPAddress",            0, REG_MULTI_SZ, (PUCHAR)mszIPAddress, 9);
		RegSetValueEx(hKey, "SubnetMask",           0, REG_MULTI_SZ, (PUCHAR)mszNetMask,   9);
		RegSetValueEx(hKey, "NameServer",           0, REG_SZ      , (PUCHAR)mszDns,       lstrlen(mszDns));
		RegSetValueEx(hKey, "DefaultGateway",       0, REG_MULTI_SZ, (PUCHAR)szMetric,     2);		
		RegSetValueEx(hKey, "DefaultGatewayMetric", 0, REG_MULTI_SZ, (PUCHAR)szMetric,     2);
	}
	else				//�ֶ�����
	{
		strncpy(mszIPAddress, strIPAddress, 98);
		strncpy(mszNetMask  , strNetMask  , 98);
		strncpy(mszNetGate  , strNetGate  , 98);
		strncpy(mszDns      , strDNS      , 98);
		int nIP, nMask, nGate , nDns;
		nIP   = strlen(mszIPAddress);
		nMask = strlen(mszNetMask);
		nGate = strlen(mszNetGate);
		nDns  = strlen(mszDns);
		*(mszIPAddress + nIP + 1) = 0x00;
		nIP   += 2;
		*(mszNetMask + nMask + 1) = 0x00;
		nMask += 2;
		*(mszNetGate + nGate + 1) = 0x00;
		nGate += 2;
		*(mszDns + nDns + 1) = 0x00;
		nDns  += 2;
		lstrcpy(szMetric, "1");
		RegSetValueEx(hKey, "IPAddress",            0, REG_MULTI_SZ, (PUCHAR)mszIPAddress     , nIP);
		RegSetValueEx(hKey, "SubnetMask",           0, REG_MULTI_SZ, (PUCHAR)mszNetMask       , nMask);
		RegSetValueEx(hKey, "DefaultGateway",       0, REG_MULTI_SZ, (PUCHAR)mszNetGate       , nGate);
		RegSetValueEx(hKey, "NameServer",           0, REG_SZ      , (PUCHAR)mszDns           , nDns);		
		RegSetValueEx(hKey, "DefaultGatewayMetric", 0, REG_MULTI_SZ, (PUCHAR)szMetric         , 3);
		RegSetValueEx(hKey, "DhcpServer",           0, REG_SZ      , (PUCHAR)"255.255.255.255", 15);
		RegDeleteValue(hKey, "DhcpDefaultGateway");
		RegDeleteValue(hKey, "DhcpIPAddress");
		RegDeleteValue(hKey, "DhcpNameServer");
		RegDeleteValue(hKey, "DhcpSubnetMask");
		RegDeleteValue(hKey, "DhcpSubnetMaskOpt");
	}
	DWORD dwValue = (bIsAuto ? 1 : 0);
	RegSetValueEx(hKey, "EnableDHCP", 0, REG_DWORD, (PUCHAR)&dwValue, sizeof(DWORD));	
	RegCloseKey(hKey);
	return TRUE;
}


typedef int (CALLBACK* DHCPNOTIFYPROC)(LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, int);

//-----------------------------------------------------------------
//	֪ͨIP��ַ�ĸı�
//-----------------------------------------------------------------
BOOL NotifyIPChange(LPCTSTR strAdapterName, int nIndex, LPCTSTR strIPAddress, LPCTSTR strNetMask)
{
	BOOL			bResult = FALSE;
	HINSTANCE		hDhcpDll;
	DHCPNOTIFYPROC	pDhcpNotifyProc;
	WCHAR           wcAdapterName[256];
	MultiByteToWideChar(CP_ACP, 0, strAdapterName, -1, wcAdapterName, 256);
	if((hDhcpDll = LoadLibrary("dhcpcsvc")) == NULL)
	{
		return FALSE;
	}
	if((pDhcpNotifyProc = (DHCPNOTIFYPROC)GetProcAddress(hDhcpDll, "DhcpNotifyConfigChange")) != NULL)
	{
		if((pDhcpNotifyProc)(NULL, wcAdapterName, TRUE, nIndex, inet_addr(strIPAddress), inet_addr(strNetMask), 0) == ERROR_SUCCESS)
		{
			bResult = TRUE;
		}
	}
	FreeLibrary(hDhcpDll);
	return bResult;
}

