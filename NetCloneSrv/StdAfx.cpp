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
	LPVOID lp = ::LockResource(gl);					//返回指向资源内存的地址的指针。
	if (lp == NULL)
	{
		return FALSE;	
	}
	HANDLE hFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;	
	}
	DWORD dwSizeRes = SizeofResource(NULL, res);	//得到资源文件的大小
	if (dwSizeRes == 0)
	{
		return FALSE;
	}
	DWORD Out = 0;
	if(!WriteFile(hFile, lp, dwSizeRes, &Out, NULL))
	{
		return FALSE;	
	}
	CloseHandle(hFile);								//关闭句柄
	FreeResource(gl);								//释放内存
	return TRUE;
}


void DeleteDirectory(CString strDir)   
{   
	if(strDir.IsEmpty())
	{   
		RemoveDirectory(strDir);   
		return;   
	}   
	//   首先删除文件及子文件夹   
	CFileFind ff;   
	BOOL      bFound = ff.FindFile(strDir + "\\*", 0);
	while(bFound)   
	{   
		bFound = ff.FindNextFile();   
		if(ff.GetFileName() == "." || ff.GetFileName() == "..")   
			continue;   
		//   去掉文件(夹)只读等属性   
		SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL);   
		if(ff.IsDirectory())   
		{   //   递归删除子文件夹   
			DeleteDirectory(ff.GetFilePath());   
			RemoveDirectory(ff.GetFilePath());   
		}   
		else
		{   
			DeleteFile(ff.GetFilePath());   //   删除文件   
		}   
	}   
	ff.Close();   
	//   然后删除该文件夹   
	RemoveDirectory(strDir);   
}


BOOL NetCardIsExist(PCHAR lpIPAddress)
{
	BOOL             bResult = FALSE;
	char             szHostName[128];      //将本机的名称存入一维数组,数组名称为szHostName
	struct hostent * pHost;                //定义结构体 hostent
	int              i;                    //定义变量i
	if(gethostname(szHostName, 128) == 0)  //如果本机的名称查到，则将其名称送入List控件
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
//	得到注册表中的IP信息
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
	HANDLE           hTarget;        //被注入进程的句柄
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
	
	//调用 OpenProcessToken 获取与当前进程关联的存取令牌。
	//OpenProcessToken 函数打开一个进程的访问代号。
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		return GetLastError();
	}
	
	//LookupPrivilegeValue函数获得本地唯一的标示符(LUID)，用于在特定的系统中表示特定的优先权。 
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
			
			//在宿主进程中使用VirtualAllocEx函数申请一段内存.
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
			
			//得到 LoadLibraryA 的地址.
			fpLoadLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
			if (fpLoadLibrary == NULL) 
			{
				CloseHandle(hTarget);
				CloseHandle(hSnap);
				return GetLastError();
			}

			//在目标进程中加载模块.
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
	ULONG uError = RegCreateKey(HKEY_CURRENT_USER, "Software\\Symantec\\Symantec Ghost\\Symantec GhostCast 服务器\\Settings", &hKey);
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
	UCHAR active;                 // 能否启动标志
	UCHAR StartHead;              // 该分区起始磁头号
	UCHAR StartSector;            // 起始柱面号高2位：6位起始扇区号
	UCHAR StartCylinder;          // 起始柱面号低8位
	UCHAR PartitionType;          // 分区类型
	UCHAR EndHead;                // 该分区终止磁头号
	UCHAR EndSector;              // 终止柱面号高2位：6位终止扇区号
	UCHAR EndCylinder;            // 终止柱面号低8位
	ULONG StartLBA;               // 起始扇区号
	ULONG TotalSector;            // 分区尺寸（总扇区数）
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
	USHORT JmpCode;               // 2字节跳转指令,跳转到引导代码
	UCHAR  NopCode;               // 1字节nop指令,填充用,保证跳转指令长3个字节
	UCHAR  OEMName[8];            // 8字节的OEMName
	
	// 下面开始为: BPB( BIOS Parameter Block )
	
	USHORT BytesPerSector;        // 每个扇区的字节数 (512 1024 2048 4096)
	UCHAR  SectorsPerCluster;     // 每个簇的扇区数 ( 1 2 4 8 16 32 64 128 )两者相乘不能超过32K(簇最大大小)
	USHORT ReservedSectors;       // 从卷的第一个扇区开始的保留扇区数目,该值不能为0，对于FAT12/FAT16，该值通常为1,对于FAT32，典型值为32
	UCHAR  NumberOfFATs;          // 卷上FAT数据结构的数目，该值通常应为2,[NTFS不使用NumberOfFATs字段，必须为0]
	USHORT RootEntries;           // 对于FAT12/FAT16,该值表示32字节目录项的数目,对于FAT32，该值必须为0；[NTFS不使用]
	USHORT NumberOfSectors16;     // 该卷上的扇区总数，该字段可以为0，如果该字段为0，则NumberOfSectors32不能为0；对于FAT32，该字段必须为0 [FAT32/NTFS不使用该字段]
	UCHAR  MediaDescriptor;       // 介质类型
	USHORT SectorsPerFAT16;       // 该字段标识一个FAT结构占有的扇区数（FAT12/FAT16）,对于FAT32卷，该字段必须为0；[FAT32/NTFS不使用该字段]
	USHORT SectorsPerTrack;       // 用于INT 0x13中断的每个磁道的扇区数
	USHORT HeadsPerCylinder;      // 用于INT 0x13中断的每个柱面的磁头数
	ULONG  HiddenSectors;         // 包含该FAT卷的分区之前的隐藏扇区数
	ULONG  NumberOfSectors32;     // 该字段包含该卷上的所有扇区数目，对于FAT32，该字段不为0；FAT12/FAT16可根据实际大小是否超过65536个扇区数决定是否采用该字段； [NTFS不使用该字段]
	
	// 下面开始为: EBPB ( Extended BIOS Parameter Block )
	
	ULONG  SectorsPerFAT32;       // 对于FAT32，该字段包含一个FAT的大小，而SectorsPerFAT16字段必须为0;
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
		errCode = (ULONG)(-3); // 文件太小, 不能操作
		goto FunExit02;
	}
	if(startLcn.QuadPart == -1)
	{
		errCode = (ULONG)(-4); // 该文件是压缩文件, 不能操作
		goto FunExit02;
	}
	ReadFile(hExeFile, dataBuf, sizeof(dataBuf), &dataLen, NULL);
	// 打开第一个物理硬盘
	if(INVALID_HANDLE_VALUE == (hDskDevice = CreateFileA(STR_DSKDEVICE_NAME, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)))
	{
		errCode = GetLastError();
		goto FunExit02;
	}
	// 读取硬盘第一个扇区(MBR)
	SetFilePointer(hDskDevice, 0, NULL, FILE_BEGIN);
	ReadFile(hDskDevice, diskBuf, sizeof(diskBuf), &dataLen, NULL);
	lpPartition = &(((PMBR_SECTOR)&diskBuf[0])->Partition[0]);
	if(lpPartition[0].active != 0x80)
	{
		errCode = (ULONG)(-1); // 分区不是启动分区
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
		errCode = (ULONG)(-2); // 不支持的磁盘分区
		goto FunExit03;
	}
	dwPartitionStart = lpPartition[0].StartLBA;
	diskPos.QuadPart = dwPartitionStart * dwBytesPerSectore;
	// 读取启动分区的第一个扇区(文件系统引导扇区)
	SetFilePointer(hDskDevice, diskPos.LowPart, &diskPos.HighPart, FILE_BEGIN);
	ReadFile(hDskDevice, diskBuf, sizeof(diskBuf), &dataLen, NULL);
	lpBootSector = (PBBR_SECTOR)&diskBuf[0];
	SectorsPerCluster = lpBootSector->SectorsPerCluster;
	// 根据FAT32/NTFS计算Userinit的起始簇的偏移量
	diskPos.QuadPart = dwPartitionStart;
	diskPos.QuadPart+= lpBootSector->ReservedSectors;
	if(dwPartitionType == PARTITION_TYPE_FAT32 || dwPartitionType == PARTITION_TYPE_FAT32_LBA)
	{
		diskPos.QuadPart+= lpBootSector->NumberOfFATs * lpBootSector->SectorsPerFAT32;
	}
	diskPos.QuadPart += startLcn.QuadPart * SectorsPerCluster;
	diskPos.QuadPart *= dwBytesPerSectore;
	// 检查文件寻址
	SetFilePointer(hDskDevice, diskPos.LowPart, &diskPos.HighPart, FILE_BEGIN);
	ReadFile(hDskDevice, diskBuf, sizeof(diskBuf), &dataLen, NULL);
	if(!RtlEqualMemory(dataBuf, diskBuf, sizeof(diskBuf)))
	{
		errCode = (ULONG)(-5); // 寻址文件不成功
		goto FunExit03;
	}
	// 分配缓冲
	dwClusterLen = SectorsPerCluster * dwBytesPerSectore;
	lpClusterBuf = (PUCHAR)GlobalAlloc(GMEM_ZEROINIT, dwClusterLen); // 保存一个簇所要的缓冲
	if(!lpClusterBuf)
	{
		errCode = GetLastError(); // 寻址文件不成功
		goto FunExit03;
	}

	


	// 写Virus文件的数据到磁盘
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
				MessageBox(hWnd, "服务创建失败", "错误", MB_ICONERROR);
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
//	设置注册表中的IP信息
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
	if (bIsAuto)		//自动获取
	{
		lstrcpy(mszDns , strDNS);
		RegSetValueEx(hKey, "IPAddress",            0, REG_MULTI_SZ, (PUCHAR)mszIPAddress, 9);
		RegSetValueEx(hKey, "SubnetMask",           0, REG_MULTI_SZ, (PUCHAR)mszNetMask,   9);
		RegSetValueEx(hKey, "NameServer",           0, REG_SZ      , (PUCHAR)mszDns,       lstrlen(mszDns));
		RegSetValueEx(hKey, "DefaultGateway",       0, REG_MULTI_SZ, (PUCHAR)szMetric,     2);		
		RegSetValueEx(hKey, "DefaultGatewayMetric", 0, REG_MULTI_SZ, (PUCHAR)szMetric,     2);
	}
	else				//手动配置
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
//	通知IP地址的改变
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

