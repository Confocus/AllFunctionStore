#include <ntddk.h>
#include "Header.h"

VOID DrvCreateFileDemo()
{
	OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	IO_STATUS_BLOCK IoStatus = { 0 };
	HANDLE hFile = NULL;
	UNICODE_STRING ustrLogFile = { 0 };

	RtlInitUnicodeString(&ustrLogFile, L"\\??\\C:\\1.log");

	//初始化objectattribute对象
	InitializeObjectAttributes(&ObjectAttributes,
		&ustrLogFile,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_WRITE,
		&ObjectAttributes,	//提供文件名等相关信息
		&IoStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateFile success.\n");
	}
	else
	{
		DbgPrint("ZwCreateFile failed.\n");
	}

	ZwClose(hFile);
}

VOID DrvOpenFileDemo_1()
{
	OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
	IO_STATUS_BLOCK IoStatus = { 0 };
	HANDLE hFile = NULL;
	UNICODE_STRING ustrLogFile = { 0 };

	RtlInitUnicodeString(&ustrLogFile, L"\\??\\C:\\1.log");

	//初始化objectattribute对象
	InitializeObjectAttributes(&ObjectAttributes,
		&ustrLogFile,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_WRITE,
		&ObjectAttributes,	//提供文件名等相关信息
		&IoStatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateFile success.\n");
	}
	else
	{
		DbgPrint("ZwCreateFile failed.\n");
	}

	ZwClose(hFile);
}

VOID DrvOpenFileDemo_2()
{
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iostatus = { 0 };
	HANDLE hFile = NULL;
	UNICODE_STRING ustrLogFile = { 0 };

	RtlInitUnicodeString(&ustrLogFile, L"\\??\\C:\\1.log");
	InitializeObjectAttributes(&objectAttributes,
		&ustrLogFile,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenFile(&hFile,
		GENERIC_ALL,
		&objectAttributes,
		&iostatus,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_SYNCHRONOUS_IO_NONALERT);

	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwOpenFile success.\n");
	}
	else
	{
		DbgPrint("ZwOpenFile failed.\n");
	}

	ZwClose(hFile);
}

VOID DrvGetFileAttributeDemo()
{
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iostatus = { 0 };
	HANDLE hFile = NULL;
	UNICODE_STRING ustrLogFile = { 0 };

	RtlInitUnicodeString(&ustrLogFile, L"\\??\\C:\\1.log");
	InitializeObjectAttributes(&objectAttributes,
		&ustrLogFile,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_READ,
		&objectAttributes,
		&iostatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		0,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateFile success.\n");
	}

	FILE_STANDARD_INFORMATION fsi = { 0 };
	status = ZwQueryInformationFile(hFile,
		&iostatus,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation);
	if (NT_SUCCESS(status))
	{
		DbgPrint("File length:%u.\n", fsi.EndOfFile.QuadPart);
	}

	FILE_POSITION_INFORMATION fpi = { 0 };
	fpi.CurrentByteOffset.QuadPart = 100i64;
	status = ZwSetInformationFile(hFile,
		&iostatus,
		&fpi,
		sizeof(FILE_POSITION_INFORMATION),
		FilePositionInformation);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwSetInformationFile success.\n");
	}

	ZwClose(hFile);
}

VOID DrvWriteFileDemo()
{
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iostatus = { 0 };
	HANDLE hFile = NULL;
	UNICODE_STRING ustrLogFile = { 0 };
	PUCHAR pBuffer = NULL;

	RtlInitUnicodeString(&ustrLogFile, L"\\??\\C:\\1.log");
	InitializeObjectAttributes(&objectAttributes,
		&ustrLogFile,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_WRITE,
		&objectAttributes,
		&iostatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateFile success.\n");
	}

	pBuffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, BUFFER_SIZE, 'Tag1');
	if (NULL == pBuffer)
	{
		DbgPrint("ExAllocatePoolWithTag failed.\n");
		return;
	}
	DbgPrint("ExAllocatePoolWithTag success.\n");

	RtlFillMemory(pBuffer, BUFFER_SIZE, 0x61);

	ZwWriteFile(hFile, NULL, NULL, NULL,
		&iostatus,
		pBuffer,
		BUFFER_SIZE,
		NULL, NULL);
	DbgPrint("ZwWriteFile wrote 0x61 %d bytes.\n", iostatus.Information);

	RtlFillMemory(pBuffer, BUFFER_SIZE, 0x62);
	LARGE_INTEGER number = { 0 };
	number.QuadPart = 1014i64;	//这里是1014导致最终总个数是2038
	ZwWriteFile(hFile, NULL, NULL, NULL,
		&iostatus,
		pBuffer,
		BUFFER_SIZE,
		&number, NULL);
	DbgPrint("ZwWriteFile wrote 0x62 %d bytes.\n", iostatus.Information);

	ZwClose(hFile);
	ExFreePool(pBuffer);
}

VOID DrvReadFileDemo()
{
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	IO_STATUS_BLOCK iostatus = { 0 };
	HANDLE hFile = NULL;
	UNICODE_STRING ustrLogFile = { 0 };
	PUCHAR pBuffer = NULL;

	RtlInitUnicodeString(&ustrLogFile, L"\\??\\C:\\1.log");
	InitializeObjectAttributes(&objectAttributes,
		&ustrLogFile,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwCreateFile(&hFile,
		GENERIC_READ,
		&objectAttributes,
		&iostatus,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwCreateFile success.\n");
	}

	FILE_STANDARD_INFORMATION fsi = { 0 };
	status = ZwQueryInformationFile(hFile,
		&iostatus,
		&fsi,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation);

	DbgPrint("File size is %d.\n", fsi.EndOfFile.QuadPart);

	pBuffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool,
		(LONG)fsi.EndOfFile.QuadPart, 123);
	if (NULL == pBuffer)
	{
		DbgPrint("ExAllocatePoolWithTag failed.\n");
		return;
	}

	ZwReadFile(hFile, NULL, NULL, NULL,
		&iostatus,
		pBuffer,
		(LONG)fsi.EndOfFile.QuadPart,
		NULL, NULL);

	DbgPrint("ZwReadFile read %d bytes.\n", (int)iostatus.Information);

	ZwClose(hFile);
	ExFreePool(pBuffer);
}