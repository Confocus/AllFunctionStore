#pragma once
#include <ntddk.h>
#define BUFFER_SIZE	1024
#define MAX_FILE_LEGNTH	1024
#define IOCTL_TEST1	CTL_CODE(\
FILE_DEVICE_UNKNOWN,\
0x800,\
METHOD_BUFFERED,\
FILE_ANY_ACCESS)

#define IOCTL_TRANSMIT_EVENT	CTL_CODE(\
FILE_DEVICE_UNKNOWN,\
0x801,\
METHOD_BUFFERED,\
FILE_ANY_ACCESS)


typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT pDeviceObject;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
	PUCHAR pBuffer;
	ULONG ulFileLength;
	ULONG ulMaxBufferSize;
}DEVICE_EXTENSION, * PDEVICE_EXTENSION;

VOID MyDriverUnload(IN PDRIVER_OBJECT	pDriverObject);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject, 
	IN PCWSTR pcwszDevName, 
	IN PCWSTR pcwszSymLink);
NTSTATUS MyDispatchRoutine(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchCreate(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchClose(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchWrite(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchRead(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchReadFromBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);

NTSTATUS MyDispatchWriteFromBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchReadFromMdl(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchReadFromUserBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);
NTSTATUS MyDispatchDeviceIoControlFromBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);

NTSTATUS DrvDeviceIoControlEventDemo(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp);

VOID LinkListDemo();
VOID TestException();
VOID DrvStringOperateDemo();
VOID DrvCreateFileDemo();
VOID DrvOpenFileDemo_1();
VOID DrvOpenFileDemo_2();
VOID DrvGetFileAttributeDemo();
VOID DrvWriteFileDemo();
VOID DrvReadFileDemo();
VOID DrvCreateRegDemo();
VOID DrvOpenRegDemo();
VOID DrvSetRegDemo();
VOID DrvQueryRegDemo();
VOID DrvEnumRegItemDemo();
VOID DrvEnumRegValueKeyDemo();
//VOID CreateThreadDemo();
