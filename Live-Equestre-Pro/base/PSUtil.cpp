#include "StdAfx.h"
#include "PSUtil.h"

namespace ns_base {

	void CreateHierarchyFolder(CString strDirPath)
	{
		strDirPath.Replace(_T('\\'), _T('/'));

		CString strSubDir;
		int	nBeginPos = 0;
		int nEndPos = 0;

		while(true)
		{
			nEndPos = strDirPath.Find(_T('/'), nBeginPos);
			if( nEndPos == -1 ) break;

			strSubDir = strDirPath.Left(nEndPos);
			nBeginPos = nEndPos + 1;
			CreateDirectory(strSubDir.GetBuffer(0), NULL);
		}
	}

	HMODULE	GetCurrentModule()
	{
		HMODULE hModule = NULL;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			(LPCTSTR)GetCurrentModule,
			&hModule);

		return hModule;
	}

	CString GetProcessPath()
	{
		TCHAR szFName[MAX_PATH];
		CString strName;

		DWORD dwLen = GetModuleFileName(GetModuleHandle(NULL), szFName, MAX_PATH);
		strName.Format( _T("%s"),szFName);
		return strName;
	}

	CString GetFolderName(CString strFilename)
	{
		return strFilename.Left(strFilename.ReverseFind(_T('\\')));
	}

	CString GetModulePath()
	{
		HMODULE hModule;
		TCHAR chModulePath[MAX_PATH] = { 0, };
		CString strName;
		hModule = GetCurrentModule();
		GetModuleFileName(hModule, chModulePath, MAX_PATH);
		strName.Format( _T("%s"),chModulePath);
		return strName;
	}

	void DoEvents() {
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
				DispatchMessage(&msg);
		}
	}

	BOOL	SetPrivilege()
	{
		TOKEN_PRIVILEGES tp;
		HANDLE hToken;
		LUID luid;

		if( !OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)){
			return FALSE;
		}

		if( !LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid) ){
			return FALSE; 
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		// Enable the privilege or disable all privileges.
		if( !AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), 
			(PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL) ){ 
				return FALSE; 
		}

		if( GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
			return FALSE;
		} 

		return TRUE;
	}

	BOOL EnablePrivilege(LPCTSTR lpName, HANDLE hProcess)
	{

		TOKEN_PRIVILEGES priv = { 1, {0, 0, SE_PRIVILEGE_ENABLED} };
		LookupPrivilegeValue(0, lpName, &priv.Privileges[0].Luid);

		HANDLE hToken;
		OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);

		AdjustTokenPrivileges(hToken, FALSE, &priv, sizeof(TOKEN_PRIVILEGES), 0, 0);

		BOOL rv = GetLastError() == ERROR_SUCCESS;

		CloseHandle(hToken);


		return rv;
	}
// 
// 	BOOL IsVistaOrLater()
// 	{
// 		OSVERSIONINFO osvi;
// 
// 		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
// 		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
// 
// 		GetVersionEx(&osvi);
// 		if( osvi.dwMajorVersion >= 6 )
// 			return TRUE;
// 
// 		return FALSE;
// 	}


	BOOL GetCodeSection( DWORD &dwSectAddr, DWORD &dwSectSize)
	{
		LPBYTE pStartAddr = (LPBYTE)GetModuleHandleA( NULL);	

		DWORD dwImageBase = (DWORD)pStartAddr;
		if( *(unsigned short*) pStartAddr != IMAGE_DOS_SIGNATURE){					
			return FALSE;
		}

		pStartAddr += ((PIMAGE_DOS_HEADER)pStartAddr)->e_lfanew;  	
		if( *(DWORD*)pStartAddr != IMAGE_NT_SIGNATURE){  // "PE" test
			return FALSE;
		}

		//Point to FileHeader
		pStartAddr += sizeof(DWORD);
		//Point to OptionalHeader
		pStartAddr += sizeof( IMAGE_FILE_HEADER);
		pStartAddr += sizeof(IMAGE_OPTIONAL_HEADER);
		PIMAGE_SECTION_HEADER lpISH = (PIMAGE_SECTION_HEADER)pStartAddr;

		dwSectAddr = dwImageBase + lpISH->VirtualAddress;
		dwSectSize = lpISH->SizeOfRawData;

		return TRUE;
	}

	int EnumProcesses(ENUM_PROCESS_PROC proc, LPARAM lParam)
	{
		int nProcessCount = 0;

		HANDLE hProcessSnap = INVALID_HANDLE_VALUE; 
		PROCESSENTRY32 pe32; 

		// Take a snapshot of all running threads  
		hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 ); 
		if( hProcessSnap == INVALID_HANDLE_VALUE ) 
			return	nProcessCount;

		// Fill in the size of the structure before using it. 
		pe32.dwSize = sizeof(PROCESSENTRY32 ); 

		// Retrieve information about the first thread,
		// and exit if unsuccessful
		if( !Process32First( hProcessSnap, &pe32 ) ) 
		{
			CloseHandle( hProcessSnap );     // Must clean up the snapshot object!
			return nProcessCount;
		}

		// Now walk the thread list of the system,
		// and display information about each thread
		// associated with the specified process
		do 
		{ 
			if(proc) {
				if(proc(&pe32, lParam) == TRUE) {
					nProcessCount++;
				}
			} else {
				nProcessCount++;
			}
		} while( Process32Next(hProcessSnap, &pe32 ) );

		//  Don't forget to clean up the snapshot object.
		CloseHandle( hProcessSnap );
		return nProcessCount;
	}

	DWORD	FindProcess(CString fileName)
	{
		fileName.MakeLower();
		DWORD dwProcessId = 0;

		HANDLE hProcessSnap = INVALID_HANDLE_VALUE; 
		PROCESSENTRY32 pe32; 

		// Take a snapshot of all running threads  
		hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 ); 
		if( hProcessSnap == INVALID_HANDLE_VALUE ) 
			return	0;

		// Fill in the size of the structure before using it. 
		pe32.dwSize = sizeof(PROCESSENTRY32 ); 

		// Retrieve information about the first thread,
		// and exit if unsuccessful
		if( !Process32First( hProcessSnap, &pe32 ) ) 
		{
			CloseHandle( hProcessSnap );     // Must clean up the snapshot object!
			return 0;
		}

		// Now walk the thread list of the system,
		// and display information about each thread
		// associated with the specified process
		do 
		{ 
			CString strExeName = pe32.szExeFile;
			strExeName.MakeLower();

			if(strExeName.Find(fileName) >= 0) {
				dwProcessId = pe32.th32ProcessID;
				break;
			}
		} while( Process32Next(hProcessSnap, &pe32 ) );

		//  Don't forget to clean up the snapshot object.
		CloseHandle( hProcessSnap );
		return dwProcessId;
	}

	typedef bool (__stdcall *WSTP)(HANDLE hServer,DWORD dwProcessID,int nExitcode);
	void myTerminateProcess(DWORD dwPid)
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);

		if(hProcess)
			::TerminateProcess(hProcess, 1);

		WSTP WinStationTerminateProcess;
		if (( WinStationTerminateProcess = ( WSTP ) GetProcAddress ( LoadLibrary( _T("winsta")),"WinStationTerminateProcess")))
		{
			WinStationTerminateProcess( 0, dwPid, 0);
		}
	}

	int EnumProcessThreads(DWORD dwOwnerPID, ENUM_THREAD_PROC proc, LPARAM lParam) {
		int nThreads = 0;

		HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
		THREADENTRY32 te32; 

		// Take a snapshot of all running threads  
		hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
		if( hThreadSnap == INVALID_HANDLE_VALUE ) 
			return	nThreads;

		// Fill in the size of the structure before using it. 
		te32.dwSize = sizeof(THREADENTRY32 ); 

		// Retrieve information about the first thread,
		// and exit if unsuccessful
		if( !Thread32First( hThreadSnap, &te32 ) ) 
		{
			CloseHandle( hThreadSnap );     // Must clean up the snapshot object!
			return nThreads;
		}

		// Now walk the thread list of the system,
		// and display information about each thread
		// associated with the specified process
		do 
		{ 
			if( te32.th32OwnerProcessID == dwOwnerPID )
			{
				if(proc) {
					if(proc(&te32, lParam) == TRUE) {
						nThreads++;
					}
				} else {
					nThreads++;
				}
			}
		} while( Thread32Next(hThreadSnap, &te32 ) );

		//  Don't forget to clean up the snapshot object.
		CloseHandle( hThreadSnap );
		return nThreads;
	}


	DWORD	RunExecutableFile(CString strFilename, CString strCmdline)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si));
		si.cb = sizeof(si);
		si.wShowWindow = SW_SHOW;
		si.dwFlags = STARTF_USESHOWWINDOW;

		ZeroMemory( &pi, sizeof(pi));
		CString strFullCmdline;

		if(strCmdline.IsEmpty()) {
			strFullCmdline = strFilename;
		} else {
			strFullCmdline.Format(_T("%s %s"), strFilename.GetBuffer(), strCmdline.GetBuffer());
		}

		strFilename.Replace(_T("\\"), _T("/"));
		CString strWorkingFolder = strFilename.Left(strFilename.ReverseFind(_T('/')));

		if( !CreateProcess(NULL , strFullCmdline.GetBuffer() , NULL, NULL, FALSE, 0, NULL, strWorkingFolder.GetBuffer(), &si, &pi))
		{
			return 0;
		}

		return pi.dwProcessId;
	}


	bool InjectDll(DWORD dwPid,  CString strDll)
	{
		DWORD	dwLen = (strDll.GetLength() + 1) * 2;
		SIZE_T  dwGeneric;     
		LPVOID	lpAlloc;    
		LPTHREAD_START_ROUTINE fnLoadLibrary;     
		HANDLE	hThread;     

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);

		if( hProcess )
		{
			lpAlloc = VirtualAllocEx(hProcess,NULL,dwLen, MEM_COMMIT,PAGE_READWRITE);
			fnLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"LoadLibraryW"); 
			if(lpAlloc && fnLoadLibrary && WriteProcessMemory(hProcess,lpAlloc,(LPVOID)strDll.GetBuffer(0),dwLen, &dwGeneric))
			{   
				hThread = CreateRemoteThread(hProcess, NULL, 0, fnLoadLibrary, lpAlloc, 0, NULL);    
				if(hThread)
				{
					dwGeneric = WaitForSingleObject(hThread, 5000);      
					if(dwGeneric == WAIT_OBJECT_0)
					{ 
						CloseHandle(hThread);
						return true;
					}          
				}        
			} 

			CloseHandle(hProcess);
		}

		return false;
	}

	bool RejectDll(DWORD dwPid, DWORD dwBaseAddr) 
	{
		bool bRet = false;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);

		if( hProcess )
		{
			LPTHREAD_START_ROUTINE fnFreeLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"FreeLibrary"); 
			if(fnFreeLibrary)
			{   
				HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, fnFreeLibrary, (LPVOID)dwBaseAddr, 0, NULL);    
				if(hThread)
				{
					if(WaitForSingleObject(hThread, 5000) == WAIT_OBJECT_0)
					{ 
						bRet = true;
					}          
				}  
				CloseHandle(hThread);
			} 
			CloseHandle(hProcess);
		}
		return bRet;
	}

	bool RejectDll(DWORD dwPid,  CString strDll)
	{
		HANDLE hSnapshot = INVALID_HANDLE_VALUE; 
		MODULEENTRY32 me32; 

		// Take a snapshot of all running threads  
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid); 
		if( hSnapshot == INVALID_HANDLE_VALUE ) 
			return	false;

		// Fill in the size of the structure before using it. 
		me32.dwSize = sizeof(MODULEENTRY32); 

		// Retrieve information about the first thread,
		// and exit if unsuccessful
		if( !Module32First( hSnapshot, &me32 ) ) 
		{
			CloseHandle( hSnapshot );     // Must clean up the snapshot object!
			return false;
		}

		strDll.MakeLower();
		DWORD dwBaseAddr = 0;
		do 
		{ 
			CString strModuleName = me32.szModule;
			CString strModulePath = me32.szExePath;
			strModuleName.MakeLower();

			if(strModuleName.Find(strDll) >= 0 || strModulePath.Find(strDll) >= 0) {
				dwBaseAddr = (DWORD)me32.modBaseAddr;
				break;
			}
		} while( Module32Next(hSnapshot, &me32 ) );

		CloseHandle(hSnapshot);

		if(dwBaseAddr == 0)
			return false;

		return RejectDll(dwPid, dwBaseAddr);
	}


	bool ReadProcessModuleMemory(DWORD dwPid, CString strDll, ULONGLONG dwOffset, void* pBuf, SIZE_T& dwSize)
	{
		HANDLE hSnapshot = INVALID_HANDLE_VALUE; 
		MODULEENTRY32 me32; 

		// Take a snapshot of all running threads  
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid); 
		if( hSnapshot == INVALID_HANDLE_VALUE ) 
			return	false;

		// Fill in the size of the structure before using it. 
		me32.dwSize = sizeof(MODULEENTRY32); 

		// Retrieve information about the first thread,
		// and exit if unsuccessful
		if( !Module32First( hSnapshot, &me32 ) ) 
		{
			CloseHandle( hSnapshot );     // Must clean up the snapshot object!
			return false;
		}

		strDll.MakeLower();
		DWORD dwModuleBase = 0;
		DWORD dwModuleSize = 0;
		do 
		{ 
			CString strModuleName = me32.szModule;
			CString strModulePath = me32.szExePath;
			strModuleName.MakeLower();

			if(strModuleName.Find(strDll) >= 0 || strModulePath.Find(strDll) >= 0) {
				dwModuleBase = (DWORD)me32.modBaseAddr;
				dwModuleSize = me32.modBaseSize;
				break;
			}
		} while( Module32Next(hSnapshot, &me32 ) );
		CloseHandle(hSnapshot);

		if(dwModuleSize < dwOffset + dwSize)
			return false;

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);

		bool bRet = false;
		if( hProcess )
		{
			if(ReadProcessMemory(hProcess, (LPVOID)(dwModuleBase + dwOffset), pBuf, dwSize, &dwSize))
				bRet = true;
			CloseHandle(hProcess);
		}
		return bRet;
	}


};
