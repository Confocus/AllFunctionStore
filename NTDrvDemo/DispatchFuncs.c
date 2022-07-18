#include <ntddk.h>
#include "Header.h"


NTSTATUS MyDispatchRoutine(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchRoutine.\n");
	UNREFERENCED_PARAMETER(pDevObj);

	PIO_STACK_LOCATION stack = NULL;
	stack = IoGetCurrentIrpStackLocation(pIrp);
	UCHAR type = stack->MajorFunction;
	DbgPrint("Current dispatch type is %d.\n", type);

	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	DbgPrint("Leave MyDispatchRoutine.\n");

	return status;
}

NTSTATUS MyDispatchCreate(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchCreate.\n");
	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Leave MyDispatchCreate.\n");
	return status;
}

NTSTATUS MyDispatchClose(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchClose.\n");

	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	DbgPrint("Leave MyDispatchClose.\n");

	return status;
}

NTSTATUS MyDispatchWrite(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchWrite.\n");

	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	DbgPrint("Leave MyDispatchWrite.\n");

	return status;
}

NTSTATUS MyDispatchWriteFromBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchWriteFromBuffer.\n");
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulWriteLength = stack->Parameters.Write.Length;
	ULONG ulWriteOffset = stack->Parameters.Write.ByteOffset.QuadPart;

	if (ulWriteOffset + ulWriteLength > pDevExt->ulMaxBufferSize)
	{
		status = STATUS_FILE_INVALID;
		ulWriteLength = 0;
	}
	else
	{
		memcpy(pDevExt->pBuffer + ulWriteOffset, pIrp->AssociatedIrp.SystemBuffer, ulWriteLength);
		DbgPrint("Write length is %d.\n", ulWriteLength);
		DbgPrint("Write content is: ");
		for (int i = 0; i < ulWriteLength; i++)
		{
			DbgPrint("%c ", *(pDevExt->pBuffer + ulWriteOffset + i));
		}
		DbgPrint("\n");
		status = STATUS_SUCCESS;
		if (ulWriteLength + ulWriteOffset > pDevExt->ulFileLength)
		{
			pDevExt->ulFileLength = ulWriteLength + ulWriteOffset;
		}
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulWriteLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	DbgPrint("Leave MyDispatchWriteFromBuffer.\n");

	return status;
}

NTSTATUS MyDispatchRead(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchRead.\n");
	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	//CreateThreadDemo();

	DbgPrint("Leave MyDispatchRead.\n");

	return status;
}

//从缓冲区读取数据：Buffered_IO
NTSTATUS MyDispatchReadFromBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchReadFromBuffer.\n");
	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//得到需要读设备的字节数
	ULONG ulReadLength = stack->Parameters.Read.Length;

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;
	memset(pIrp->AssociatedIrp.SystemBuffer, 0x66, ulReadLength);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Leave MyDispatchReadFromBuffer.\n");

	return status;
}


//从Mdl读取数据：Direct_IO
NTSTATUS MyDispatchReadFromMdl(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchReadFromMdl.\n");
	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//得到需要读设备的字节数
	ULONG ulReadLength = stack->Parameters.Read.Length;
	DbgPrint("Read length is %d.\n", ulReadLength);
	//得到锁定缓冲区的长度
	ULONG ulMdlLength = MmGetMdlByteCount(pIrp->MdlAddress);
	PVOID pMdlAddr = MmGetMdlVirtualAddress(pIrp->MdlAddress);
	ULONG ulMdlOffset = MmGetMdlByteOffset(pIrp->MdlAddress);
	DbgPrint("Mdl length is %d.\n", ulReadLength);
	DbgPrint("Mdl address is 0x%08X.\n", pMdlAddr);
	DbgPrint("Mdl offset is %d.\n", ulMdlOffset);

	if (ulMdlLength != ulReadLength)
	{
		pIrp->IoStatus.Information = 0;
		status = STATUS_UNSUCCESSFUL;
		DbgPrint("MdlLength != ReadLength.\n");
	}
	else
	{
		//得到在内核模式下的映射
		PVOID pKeAddr = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
		DbgPrint("Kernel address is 0x%08X.\n", pKeAddr);
		memset(pKeAddr, 0x71, ulReadLength);
		pIrp->IoStatus.Information = ulReadLength;
	}

	pIrp->IoStatus.Status = status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Leave MyDispatchReadFromMdl.\n");

	return status;
}


//从应用层缓冲区读取数据：UserBuffer
NTSTATUS MyDispatchReadFromUserBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchReadFromUserBuffer.\n");
	UNREFERENCED_PARAMETER(pDevObj);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//得到需要读设备的字节数
	ULONG ulReadLength = stack->Parameters.Read.Length;
	ULONG ulReadOffset = (ULONG)stack->Parameters.Read.ByteOffset.QuadPart;
	PVOID pUserBuffer = pIrp->UserBuffer;
	DbgPrint("pIrp->UserBuffer is 0x%08X.\n", (ULONG)pUserBuffer);
	DbgPrint("Called by process pid: %d\n", PsGetCurrentProcessId());
	__try {
		DbgPrint("Enter try block..\n");
		ProbeForWrite(pUserBuffer, ulReadLength, 4);
		memset(pUserBuffer, 0x68, ulReadLength);
		DbgPrint("Leave try block..\n");
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Enter except block..\n");
		status = STATUS_UNSUCCESSFUL;
		DbgPrint("Leave except block..\n");
	}

	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulReadLength;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	DbgPrint("Leave MyDispatchReadFromUserBuffer.\n");

	return status;
}

//缓冲内存模式IOCTL
NTSTATUS MyDispatchDeviceIoControlFromBuffer(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter MyDispatchDeviceIoControlFromBuffer.\n");
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	ULONG info = 0;
	switch (code)
	{
	case IOCTL_TEST1:
	{
		DbgPrint("Enter IOCTL_TEST1.\n");
		UCHAR* pInputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
		for (ULONG i = 0; i < cbin; i++)
		{
			DbgPrint("%x ", pInputBuffer[i]);
		}
		UCHAR* pOutputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
		memset(pOutputBuffer, 0x68, cbout);
		info = cbout;
		DbgPrint("Leave IOCTL_TEST1.\n");
	}
	default:
		status = STATUS_INVALID_VARIANT;
	}
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = info;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	DbgPrint("Leave MyDispatchDeviceIoControlFromBuffer.\n");
	return status;
}


//应用层与内核层使用Event进行通信
NTSTATUS DrvDeviceIoControlEventDemo(
	IN PDEVICE_OBJECT pDevObj,
	IN PIRP pIrp)
{
	DbgPrint("Enter DrvDeviceIoControlEventDemo.\n");
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG ulInput = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG ulOutput = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG ulCode = stack->Parameters.DeviceIoControl.IoControlCode;
	ULONG ulInfo = 0;
	switch (ulCode)
	{
	case IOCTL_TRANSMIT_EVENT:
	{
		DbgPrint("Enter IOCTL_TRANSMIT_EVENT.\n");
		HANDLE hUserEvent = *(HANDLE*)pIrp->AssociatedIrp.SystemBuffer;
		PKEVENT pEvent = NULL;
		status = ObReferenceObjectByHandle(
			hUserEvent, 
			EVENT_MODIFY_STATE, 
			*ExEventObjectType, 
			KernelMode, 
			(PVOID*)&pEvent, 
			NULL);
		DbgPrint("Waiting ...\n");
		LARGE_INTEGER MyInterval = { 0 };
		MyInterval.QuadPart = -10 * 1000 * 1000;
		MyInterval.QuadPart *= 5;
		KeDelayExecutionThread(KernelMode, 0, &MyInterval);
		KeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);
		ObDereferenceObject(pEvent);
		DbgPrint("Leave IOCTL_TRANSMIT_EVENT.\n");
		break;
	}
	default:
		status = STATUS_INVALID_VARIANT;
	}
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = ulInfo;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	DbgPrint("Leave DrvDeviceIoControlEventDemo.\n");

	return status;
}