#pragma once
#include <ntddk.h>

VOID MyDriverUnload(IN PDRIVER_OBJECT	pDriverObject);
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject, IN PCWSTR pcwszDevName, IN PCWSTR pcwszSymLink);
NTSTATUS MyDispatchRoutine(
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




