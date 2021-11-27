#include <ntddk.h>
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

VOID MyDriverUnload(IN PDRIVER_OBJECT	pDriverObject);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject, IN PCWSTR pcwszDevName, IN PCWSTR pcwszSymLink);
NTSTATUS MyDispatchRoutine(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
VOID LinkListDemo();
VOID TestException();

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
	LinkListDemo();
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