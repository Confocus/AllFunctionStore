#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <tchar.h>

#define NT_SUCCESS(x) ((x) >= 0)

typedef struct _PROCESS_BASIC_INFORMATION64 {
	NTSTATUS ExitStatus;
	UINT32 Reserved0;
	UINT64 PebBaseAddress;
	UINT64 AffinityMask;
	UINT32 BasePriority;
	UINT32 Reserved1;
	UINT64 UniqueProcessId;
	UINT64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64;

typedef struct _PEB64
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG64 Mutant;
	ULONG64 ImageBaseAddress;
	ULONG64 Ldr;
	ULONG64 ProcessParameters;
	ULONG64 SubSystemData;
	ULONG64 ProcessHeap;
	ULONG64 FastPebLock;
	ULONG64 AtlThunkSListPtr;
	ULONG64 IFEOKey;
	ULONG64 CrossProcessFlags;
	ULONG64 UserSharedInfoPtr;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	ULONG64 ApiSetMap;
} PEB64, * PPEB64;
//
//typedef struct _PEB {
//	BYTE                          Reserved1[2];
//	BYTE                          BeingDebugged;
//	BYTE                          Reserved2[1];
//	PVOID                         Reserved3[2];
//	PPEB_LDR_DATA                 Ldr;
//	PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
//	BYTE                          Reserved4[104];
//	PVOID                         Reserved5[52];
//	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
//	BYTE                          Reserved6[128];
//	PVOID                         Reserved7[1];
//	ULONG                         SessionId;
//} PEB, * PPEB;

typedef NTSTATUS(NTAPI* NT_QUERY_INFORMATION_PROCESS)(

	IN  HANDLE ProcessHandle,

	IN  PROCESSINFOCLASS ProcessInformationClass,

	OUT PVOID ProcessInformation,

	IN  ULONG ProcessInformationLength,

	OUT PULONG ReturnLength    OPTIONAL

	);

typedef NTSTATUS(WINAPI* NT_WOW64_QUERY_INFORMATION_PROCESS64)(
	HANDLE ProcessHandle, 
	UINT32 ProcessInformationClass,
	PVOID ProcessInformation, 
	UINT32 ProcessInformationLength,
	UINT32* ReturnLength);

typedef
NTSTATUS(WINAPI* Nt_WOW64_READ_VIRTUAL_MEMORY64)(
	HANDLE ProcessHandle, 
	PVOID64 BaseAddress,
	PVOID BufferData, 
	UINT64 BufferLength,
	PUINT64 ReturnLength);

NT_QUERY_INFORMATION_PROCESS pfnNtQueryInformationProcess = NULL;
NT_WOW64_QUERY_INFORMATION_PROCESS64 pfnNtWow64QueryInformationProcess = NULL;
Nt_WOW64_READ_VIRTUAL_MEMORY64 pfnNtWow64ReadVirtualMemory64 = NULL;

void main()
{
	BOOL bWow64Process = FALSE;
	DWORD dwProcessId = 0;
	DWORD dwSizeNeeded = 0;
	DWORD dwBytesRead = 0;
	DWORD dwErrorCode = 0;
	DWORD64 dwBytesRead64 = 0;
	NTSTATUS dwStatus = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 12160);
	if (NULL == hProcess)
	{
		return;
	}

	//HANDLE hProcess = GetCurrentProcess();
	PEB peb = {0};
	PEB64 peb64 = { 0 };
	PROCESS_BASIC_INFORMATION pbi = {0};
	PROCESS_BASIC_INFORMATION64 pbi64 = { 0 };

	HMODULE hNtDll = LoadLibrary(_T("ntdll.dll"));
	if (NULL == hNtDll)
	{
		return;
	}

	pfnNtQueryInformationProcess = (NT_QUERY_INFORMATION_PROCESS)GetProcAddress(hNtDll, "NtQueryInformationProcess");
	if (NULL == pfnNtQueryInformationProcess)
	{
		return;
	}

	pfnNtWow64QueryInformationProcess = (NT_WOW64_QUERY_INFORMATION_PROCESS64)GetProcAddress(hNtDll, "NtWow64QueryInformationProcess64");
	if (NULL == pfnNtWow64QueryInformationProcess)
	{
		return;
	}

	IsWow64Process(hProcess, &bWow64Process);

	if (bWow64Process)
	{
		dwStatus = pfnNtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), &dwSizeNeeded);
		if (!NT_SUCCESS(dwStatus) || 0 == pbi.PebBaseAddress)
		{
			dwErrorCode = GetLastError();
			return;
		}
		ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(PEB), &dwBytesRead);

	}
	else
	{
		dwStatus = pfnNtWow64QueryInformationProcess(hProcess, ProcessBasicInformation, &pbi64, sizeof(PROCESS_BASIC_INFORMATION64), (UINT32*)&dwSizeNeeded);
		if (!NT_SUCCESS(dwStatus) || 0 == pbi64.PebBaseAddress)
		{
			dwErrorCode = GetLastError();
			return;
		}
		pfnNtWow64ReadVirtualMemory64 = (Nt_WOW64_READ_VIRTUAL_MEMORY64)GetProcAddress(hNtDll, "NtWow64ReadVirtualMemory64");
		//如果32位程序读取的是一个64位地址会出问题
		//ReadProcessMemory(hProcess, (LPCVOID)pbi64.PebBaseAddress, &peb64, sizeof(PEB64), &dwBytesRead);
		pfnNtWow64ReadVirtualMemory64(hProcess, (PVOID64)pbi64.PebBaseAddress, &peb64, sizeof(PEB64), &dwBytesRead64);

	}
	
	

	getchar();
}