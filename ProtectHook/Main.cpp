
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <winioctl.h>

#define CommMmMapName  TEXT("__KDNETCLONE_COMM_MEMMAP__")


void ErrorExitProcess()
{
	ExitProcess(NULL);
}


LONG WINAPI ExceptionFilter( _EXCEPTION_POINTERS *ExceptionInfo)
{
	PEXCEPTION_RECORD ExceptionRecord = ExceptionInfo->ExceptionRecord;
	PCONTEXT ContextRecord = ExceptionInfo->ContextRecord; 
	ContextRecord->Eip = (ULONG)ErrorExitProcess;
	return EXCEPTION_CONTINUE_EXECUTION;
}


HANDLE g_hMutex = NULL;


void DeleteDirectory(const PCHAR szPath)
{
	WIN32_FIND_DATA stData;
	HANDLE          hSearch;
    CHAR            szNewPath[MAX_PATH];
	
	strcpy(szNewPath, szPath);
	if (szNewPath[lstrlen(szNewPath)-1] != '\\')
	{
		strcat(szNewPath, "\\");
	}
    strcat(szNewPath, "*.*");
	hSearch = ::FindFirstFile(szNewPath, &stData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		return;	
	}
	do
	{
		if (!strcmp(stData.cFileName, "..") || !strcmp(stData.cFileName, "."))
		{
			continue;		
		}
		if (stData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{	//是目录
			strcpy(szNewPath, szPath);
			if (szNewPath[lstrlen(szNewPath)-1] != '\\')
			{
				strcat(szNewPath, "\\");	
			}
			strcat(szNewPath, stData.cFileName);
			strcat(szNewPath, "\\");
			SetFileAttributes(szNewPath, FILE_ATTRIBUTE_NORMAL);  
			DeleteDirectory(szNewPath);
		}
		else
		{	//是文件
			strcpy(szNewPath, szPath);
			if (szNewPath[lstrlen(szNewPath)-1] != '\\')
			{
				strcat(szNewPath, "\\");	
			}
			strcat(szNewPath, stData.cFileName);
			SetFileAttributes(szNewPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szNewPath);
		}
	}
	while (::FindNextFile(hSearch, &stData));
	::FindClose(hSearch);
	RemoveDirectory(szPath);
}


void ThreadProc(PVOID param)
{
	g_hMutex = ::CreateMutex(NULL, TRUE, "__KDNETCLONE_DLL_MUTEX__");
	if(g_hMutex != NULL && ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		ErrorExitProcess();
		return;
	}
	SetUnhandledExceptionFilter(ExceptionFilter);
	HANDLE hFileMappingObject = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, CommMmMapName);
	if (hFileMappingObject == NULL)
	{
		return;
	}
	// 共享内存映射开始地址
	PULONG lpProcessID = (PULONG)MapViewOfFile(hFileMappingObject, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (lpProcessID == NULL)
	{
		CloseHandle(hFileMappingObject);
		return;
	}
	ULONG uProcessID = *lpProcessID;
	UnmapViewOfFile(lpProcessID);
	CloseHandle(hFileMappingObject);
	HANDLE           hToken;
	TOKEN_PRIVILEGES stTokenPriv;
	LUID             Luid;
	//调用 OpenProcessToken 获取与当前进程关联的存取令牌。
	//OpenProcessToken 函数打开一个进程的访问代号。
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		return;
	}
	//LookupPrivilegeValue函数获得本地唯一的标示符(LUID)，用于在特定的系统中表示特定的优先权。 
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid))
	{
		return;
	}
	stTokenPriv.PrivilegeCount           = 1;
	stTokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	stTokenPriv.Privileges[0].Luid       = Luid;
	if (!AdjustTokenPrivileges(hToken, FALSE, &stTokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		return;
	}
	HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, uProcessID);
	WaitForSingleObject(hProcess, INFINITE);
	CloseHandle(hProcess);
	CHAR szTempPath[MAX_PATH];
	GetTempPath(sizeof(szTempPath), szTempPath);
	lstrcat(szTempPath, "KeydoneNetClone\\");
	DeleteDirectory(szTempPath);
	ErrorExitProcess();
}

//-------------------------------------------------------主函数开始
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		_beginthread(ThreadProc, 0, NULL);
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		if (g_hMutex != NULL)
		{
			CloseHandle(g_hMutex);
		}
	}
	return TRUE;
}
