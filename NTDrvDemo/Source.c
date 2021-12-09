#include <ntddk.h>
#include "Header.h"
//#pragma warning(disable:2220)

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pDeviceObject;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
}DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _MY_DATA_STRUCT
{
	LIST_ENTRY ListEntry;
	ULONG number;
}MY_DATA_STRUCT, *PMY_DATA_STRUCT;


NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT	pDriverObject,	//从IO管理器中传进来的驱动对象
	IN PUNICODE_STRING	pRegistryPath	//驱动程序在注册表中的路径
)
{
	UNREFERENCED_PARAMETER(pRegistryPath);
	UNREFERENCED_PARAMETER(pDriverObject);

	DbgPrint("Enter DriverEntry.\n");
	pDriverObject->DriverUnload = MyDriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = MyDispatchRoutine;

	//CreateDevice(pDriverObject);//创建设备对象
	CreateDevice(pDriverObject, L"\\Device\\MyNTDriver", L"\\??\\MyNTDriver");
	CreateDevice(pDriverObject, L"\\Device\\MyNTDriver2", L"\\??\\MyNTDriver2");
	DrvEnumRegValueKeyDemo();
	return STATUS_SUCCESS;
}

VOID MyDriverUnload(IN PDRIVER_OBJECT	pDriverObject)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	DbgPrint("Enter DriverUnload.\n");
	PDEVICE_OBJECT pNextObj;
	pNextObj = pDriverObject->DeviceObject;
	while (pNextObj != NULL)
	{
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextObj->DeviceExtension;
		UNICODE_STRING ustrLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&ustrLinkName);
		pNextObj = pNextObj->NextDevice;
		IoDeleteDevice(pDevExt->pDeviceObject);
	}
}

NTSTATUS MyDispatchRoutine(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject, IN PCWSTR pcwszDevName, IN PCWSTR pcwszSymLink)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj = NULL;
	PDEVICE_EXTENSION pDevExt = NULL;
	UNREFERENCED_PARAMETER(pDriverObject);

	UNICODE_STRING ustrDevName;
	//RtlInitUnicodeString(&ustrDevName, L"\\Device\\MyNTDriver");
	RtlInitUnicodeString(&ustrDevName, pcwszDevName);


	//创建一个设备对象
	status = IoCreateDevice(
		pDriverObject,
		sizeof(DRIVER_EXTENSION),
		&ustrDevName,
		FILE_DEVICE_UNKNOWN,
		0,
		TRUE,
		&pDevObj);

	if (!NT_SUCCESS(status))
		return status;

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDeviceObject = pDevObj;
	pDevExt->ustrDeviceName = ustrDevName;

	UNICODE_STRING ustrSymLinkName;
	//RtlInitUnicodeString(&ustrSymLinkName, L"\\??\\MyNTDriver");
	RtlInitUnicodeString(&ustrSymLinkName, pcwszSymLink);

	pDevExt->ustrSymLinkName = ustrSymLinkName;
	status = IoCreateSymbolicLink(&ustrSymLinkName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}

	return STATUS_SUCCESS;
}


//驱动中链表的使用
VOID LinkListDemo()
{
	LIST_ENTRY linkListHead;
	InitializeListHead(&linkListHead);

	PMY_DATA_STRUCT pMyData = NULL;

	DbgPrint("Start LinkListDemo.\n");
	for (ULONG i = 0; i < 10; i++)
	{
		//warning C4996: 'ExAllocatePool': was declared deprecated
		pMyData = (PMY_DATA_STRUCT)ExAllocatePoolWithTag(PagedPool, sizeof(MY_DATA_STRUCT), 'Tag1');
		if (pMyData)
		{
			pMyData->number = i;
			InsertHeadList(&linkListHead, &pMyData->ListEntry);
		}
	}

	while (!IsListEmpty(&linkListHead))
	{
		//从尾部删除
		PLIST_ENTRY pEntry = RemoveTailList(&linkListHead);
		//pEntry只能拿到FLink和BLink指针
		//DbgPrint("pEntry:%x\n", (ULONG)pEntry);
		pMyData = CONTAINING_RECORD(pEntry, MY_DATA_STRUCT, ListEntry);
		//DbgPrint("pMyData:%x\n", (ULONG)pMyData);
		DbgPrint("number:%d\n", pMyData->number);
		ExFreePool(pMyData);
	}
}

VOID LookasideDemo()
{
	PAGED_LOOKASIDE_LIST pageList;
	ExInitializePagedLookasideList(&pageList, NULL, NULL, 0, sizeof(MY_DATA_STRUCT), 'Tag2', 0);
	PMY_DATA_STRUCT MyObjectArray[10] = { 0 };
	for (int i = 0; i < 10; i++)
	{
		MyObjectArray[i] = (PMY_DATA_STRUCT)ExAllocateFromPagedLookasideList(&pageList);
	}

	for (int i = 0; i < 10; i++)
	{
		ExFreeToPagedLookasideList(&pageList, MyObjectArray[i]);
		MyObjectArray[i] = NULL;
	}

	ExDeletePagedLookasideList(&pageList);
}

VOID TestExceptionInner()
{
	int* pAddr = 0;
	__try {
		DbgPrint("Enter TestExceptionInner.\n");
		ExRaiseAccessViolation();
	}
	__finally
	{
		while (1)
		{
			DbgPrint("Enter TestExceptionInner finally.\n");
		}
	}
}

VOID TestException()
{
	TestExceptionInner();

	//__try {
	//	TestExceptionInner();
	//}
	//__except (EXCEPTION_CONTINUE_SEARCH)
	//{
	//	DbgPrint("Enter TestException exception().\n");
	//}
}

//如何初始化内核字符串
VOID DrvStringOperateDemo()
{
	ANSI_STRING AnsiString;
	RtlInitAnsiString(&AnsiString, "Hello Ansi\n");

#define BUFFER_SIZE	1024
	UNICODE_STRING UnicodeString0 = { 0 };
	WCHAR* wideString = L"Hello Unicode\n";
	UnicodeString0.MaximumLength = BUFFER_SIZE;
	UnicodeString0.Buffer = (PCWSTR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	UnicodeString0.Length = (USHORT)(2 * wcslen(wideString));
	if (NULL == UnicodeString0.Buffer)
	{
		DbgPrint("ExAllocatePool failed.\n");
		return;
	}

	ASSERT(UnicodeString0.MaximumLength >= UnicodeString0.Length);

	RtlCopyMemory(UnicodeString0.Buffer, wideString, UnicodeString0.Length);

	UnicodeString0.Length = (USHORT)(2 * wcslen(wideString));

	DbgPrint("UnicodeString0: %wZ", &UnicodeString0);

	ExFreePool(UnicodeString0.Buffer);
	UnicodeString0.Buffer = NULL;
	UnicodeString0.Length = 0;
	UnicodeString0.MaximumLength = 0;



	//拷贝字符串操作:RtlCopyUnicodeString
	DbgPrint("Copy unicodestring:\n");
	UNICODE_STRING UnicodeString1;
	RtlInitUnicodeString(&UnicodeString1, L"Hello World\n");

	UNICODE_STRING UnicodeString2 = {0};
	UnicodeString2.Buffer = (PWSTR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	UnicodeString2.MaximumLength = BUFFER_SIZE;

	RtlCopyUnicodeString(&UnicodeString2, &UnicodeString1);

	DbgPrint("UnicodeString1:%wZ", &UnicodeString1);
	DbgPrint("UnicodeString2:%wZ", &UnicodeString2);

	RtlFreeUnicodeString(&UnicodeString2);

	//字符串比较操作：RtlEqualUnicodeString
	DbgPrint("Compare unicodestring:\n");
	UNICODE_STRING UnicodeString3 = {0};
	RtlInitUnicodeString(&UnicodeString3, L"Hello Compare");
	UNICODE_STRING UnicodeString4 = {0};
	RtlInitUnicodeString(&UnicodeString4, L"Hello Compare");

	if (RtlEqualUnicodeString(&UnicodeString3, &UnicodeString4, TRUE))
	{
		//KdPrint("RtlEqualUnicodeString\n");
		DbgPrint("UnicodeString3 and UnicodeString4 are equal\n");
	}
	else
	{
		DbgPrint("UnicodeString3 and UnicodeString4 are NOT equal\n");
	}

	//字符串转换成大写操作：RtlUpcaseUnicodeString
	UNICODE_STRING UnicodeString5 = {0};
	RtlInitUnicodeString(&UnicodeString5, L"Hello Upper");
	UNICODE_STRING UnicodeString6 = { 0 };

	DbgPrint("Lower string:%wZ\n", &UnicodeString5);
	RtlUpcaseUnicodeString(&UnicodeString6, &UnicodeString5, TRUE);
	DbgPrint("Upper string:%wZ\n", &UnicodeString6);
	RtlFreeUnicodeString(&UnicodeString6);

	//字符串与整型数字相互转换:RtlUnicodeStringToInteger 和 RtlIntegerToUnicodeString
	UNICODE_STRING UnicodeString7 = { 0 };
	RtlInitUnicodeString(&UnicodeString7, L"123");

	ULONG lNum = 0;
	NTSTATUS status = RtlUnicodeStringToInteger(&UnicodeString7, 10, &lNum);

	if (NT_SUCCESS(status))
	{
		DbgPrint("RtlUnicodeStringToInteger result:%d\n", lNum);
	}
	else
	{
		DbgPrint("RtlUnicodeStringToInteger failed.\n");
	}

	UNICODE_STRING UnicodeString8 = { 0 };
	UnicodeString8.Buffer = (PWSTR)ExAllocatePool(PagedPool, BUFFER_SIZE);
	UnicodeString8.MaximumLength = BUFFER_SIZE;
	status = RtlIntegerToUnicodeString(200, 10, &UnicodeString8);
	if (NT_SUCCESS(status))
	{
		DbgPrint("RtlIntegerToUnicodeString result:%wZ\n", &UnicodeString8);
	}
	else
	{
		DbgPrint("RtlIntegerToUnicodeString failed.\n");
	}
	RtlFreeUnicodeString(&UnicodeString8);

	//ANSI_STRING和UNICODE_STRING相互转换：RtlAnsiStringToUnicodeString 和 RtlAnsiStringToUnicodeString
	UNICODE_STRING UnicodeString9 = { 0 };
	RtlInitUnicodeString(&UnicodeString9, L"Hello World");

	ANSI_STRING AnsiString0 = { 0 };
	status = RtlUnicodeStringToAnsiString(&AnsiString0, &UnicodeString9, TRUE);
	if (NT_SUCCESS(status))
	{
		DbgPrint("RtlUnicodeStringToAnsiString result:%Z\n", &AnsiString0);
	}
	else
	{
		DbgPrint("RtlUnicodeStringToAnsiString failed.\n");
	}
	RtlFreeAnsiString(&AnsiString0);

	ANSI_STRING AnsiString1 = { 0 };
	RtlInitAnsiString(&AnsiString1, "Hello World");

	UNICODE_STRING UnicodeString10 = { 0 };
	status = RtlAnsiStringToUnicodeString(&UnicodeString10, &AnsiString1, TRUE);
	if (NT_SUCCESS(status))
	{
		DbgPrint("RtlAnsiStringToUnicodeString result:%wZ\n", &UnicodeString10);
	}
	else
	{
		DbgPrint("RtlAnsiStringToUnicodeString failed.\n");
	}

	RtlFreeUnicodeString(&UnicodeString10);
}

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

