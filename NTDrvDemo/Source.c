#include <ntddk.h>

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pDeviceObject;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
}DEVICE_EXTENSION, * PDEVICE_EXTENSION;

VOID MyDriverUnload(IN PDRIVER_OBJECT	pDriverObject);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS MyDispatchRoutine(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);

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

	CreateDevice(pDriverObject);//创建设备对象

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

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj = NULL;
	PDEVICE_EXTENSION pDevExt = NULL;
	UNREFERENCED_PARAMETER(pDriverObject);

	UNICODE_STRING ustrDevName;
	RtlInitUnicodeString(&ustrDevName, L"\\Device\\MyNTDriver");

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
	RtlInitUnicodeString(&ustrSymLinkName, L"\\??\\MyNTDriver");
	pDevExt->ustrSymLinkName = ustrSymLinkName;
	status = IoCreateSymbolicLink(&ustrSymLinkName, &ustrDevName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}

	return STATUS_SUCCESS;
}