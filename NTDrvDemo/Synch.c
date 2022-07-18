#include <ntddk.h>
#include "Header.h"

VOID SystemThread(IN PVOID pContext)
{
	DbgPrint("Enter SystemThread.\n");
	PEPROCESS pEProcess = PsGetCurrentProcess();
	if (NULL == pEProcess)
	{
		DbgPrint("IoGetCurrentProcess failed.\n");
	}
	PTSTR pstrProcessName = (PTSTR)((ULONG)pEProcess + 0x174);
	DbgPrint("This thread run in process %s.\n", pstrProcessName);
	DbgPrint("Leave SystemThread.\n");
	PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID MyProcessThread(IN PVOID pContext)
{
	DbgPrint("Enter MyProcessThread.\n");
	PEPROCESS pEProcess = PsGetCurrentProcess();
	if (NULL == pEProcess)
	{
		DbgPrint("IoGetCurrentProcess failed.\n");
	}
	PTSTR pstrProcessName = (PTSTR)((ULONG)pEProcess + 0x174);
	DbgPrint("This thread run in process %s.\n", pstrProcessName);
	DbgPrint("Leave MyProcessThread.\n");
	PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID CreateThreadDemo()
{
	HANDLE hSystemThreadHandle = NULL;
	HANDLE hMyThreadHandle = NULL;
	NTSTATUS status = PsCreateSystemThread(&hSystemThreadHandle, 
		0, 
		NULL, 
		NULL, 
		NULL, 
		SystemThread, 
		NULL);

	status = PsCreateSystemThread(&hMyThreadHandle,
		0,
		NULL,
		NtCurrentProcess(),
		NULL,
		MyProcessThread,
		NULL);
}
