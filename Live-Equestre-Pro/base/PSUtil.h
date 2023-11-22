/************************************************************************
// 
// 
// updates
//	- 2017-03-14
//		* add GetFolderName() : get folder name from filename.
//		* add CreateHierarchyFolder() : create hierarchy folders.
//	- 2015-08-19	1.0
//		* 
************************************************************************/

#pragma once
#include <Tlhelp32.h>

namespace ns_base {

	HMODULE	GetCurrentModule();
	CString GetProcessPath();
	CString GetModulePath();
	void	CreateHierarchyFolder(CString strDirPath);
	CString GetFolderName(CString strFilename);

	void DoEvents();

	BOOL	SetPrivilege();				// 
	BOOL	EnablePrivilege(LPCTSTR lpName, HANDLE hProcess);	

	BOOL	IsVistaOrLater();			// 

	BOOL	GetCodeSection( DWORD &dwSectAddr, DWORD &dwSectSize);		//  

	// 
	typedef BOOL	(CALLBACK *ENUM_PROCESS_PROC)(LPPROCESSENTRY32 pProcess, LPARAM lParam);
	int		EnumProcesses(ENUM_PROCESS_PROC proc, LPARAM lParam);

	DWORD	FindProcess(CString fileName);
	void	myTerminateProcess(DWORD dwPid);

	// 
	typedef BOOL	(CALLBACK *ENUM_THREAD_PROC)(THREADENTRY32* pThread, LPARAM lParam);
	int		EnumProcessThreads(DWORD dwOwnerPID, ENUM_THREAD_PROC proc, LPARAM lParam);

	DWORD	CreateProcessWithDll(CString strPath, CString strApp, CString strCmdLine, CString strDll, BOOL bTerminateOnFailure);
	DWORD	RunExecutableFile(CString strFilename, CString strCmdline = CString());

	int		HideDll(CString strDllName);					// 

	bool	InjectDll(DWORD dwPid,  CString strDll);		// 
	bool	RejectDll(DWORD dwPid,  CString strDll);		
	bool	RejectDll(DWORD dwPid, DWORD dwBaseAddr);

	bool	ReadProcessModuleMemory(DWORD dwPid, CString strDll, DWORD dwOffset, void* pBuf, DWORD& dwSize);

#pragma pack(push, 1)
	typedef struct _LSA_UNICODE_STRING {
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

	typedef struct _PEB_LDR_DATA {
		ULONG                   Length;
		BOOLEAN                 Initialized;
		PVOID                   SsHandle;
		LIST_ENTRY              InLoadOrderModuleList;
		LIST_ENTRY              InMemoryOrderModuleList;
		LIST_ENTRY              InInitializationOrderModuleList;
	} PEB_LDR_DATA, *PPEB_LDR_DATA;

	typedef struct _LDR_MODULE {
		LIST_ENTRY              InLoadOrderModuleList;
		LIST_ENTRY              InMemoryOrderModuleList;
		LIST_ENTRY              InInitializationOrderModuleList;
		PVOID                   BaseAddress;
		PVOID                   EntryPoint;
		ULONG                   SizeOfImage;
		UNICODE_STRING          FullDllName;
		UNICODE_STRING          BaseDllName;
		ULONG                   Flags;
		SHORT                   LoadCount;
		SHORT                   TlsIndex;
		LIST_ENTRY              HashTableEntry;
		ULONG                   TimeDateStamp;
	} LDR_MODULE, *PLDR_MODULE;
#pragma pack(pop)


};

