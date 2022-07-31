#include <ntddk.h>
#include <ntstrsafe.h>

//假设最多有CCP_MAX_COM_ID个串口
#define MAX_COM_ID	32

static PDEVICE_OBJECT s_FltObj[MAX_COM_ID] = { 0 };
static PDEVICE_OBJECT s_TopObjectBeforeAttach[MAX_COM_ID] = { 0 };

NTSTATUS ccpDispatch(PDEVICE_OBJECT device, PIRP pIrp)
{
	ULONG i, j = 0;
	PIO_STACK_LOCATION pIrpLoc = IoGetCurrentIrpStackLocation(pIrp);
	

	for (i = 0; i < MAX_COM_ID; i++)
	{
		if (s_FltObj[i] == device)
		{
			if (pIrpLoc->MajorFunction == IRP_MJ_POWER)
			{
				PoStartNextPowerIrp(pIrp);
				IoSkipCurrentIrpStackLocation(pIrp);
				return PoCallDriver(s_TopObjectBeforeAttach[i], pIrp);
			}

			if (pIrpLoc->MajorFunction == IRP_MJ_WRITE)
			{
				ULONG uLen = pIrpLoc->Parameters.Write.Length;
				PUCHAR buf = NULL;
				if (pIrp->MdlAddress != NULL)
				{
					buf = (PUCHAR)MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
				}
				else
				{
					buf = (PUCHAR)pIrp->UserBuffer;
				}

				if (buf == NULL)
				{
					buf = (PUCHAR)pIrp->AssociatedIrp.SystemBuffer;
				}

				for (j = 0; j < uLen; ++j)
				{
					DbgPrint("[ComFlt]Send Data:%2x\r\n", buf[j]);
				}

				IoSkipCurrentIrpStackLocation(pIrp);
				return IoCallDriver(s_TopObjectBeforeAttach[i], pIrp);
			}
		}
	}

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

void ccpUnload(PDRIVER_OBJECT pDriverObject)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	ULONG i = 0;
	LARGE_INTEGER interval = { 0 };
	for (i = 0; i < MAX_COM_ID; i++)
	{
		if (s_TopObjectBeforeAttach[i] != NULL)
		{
			IoDeleteDevice(s_TopObjectBeforeAttach[i]);
		}
	}
	interval.QuadPart = (5 * 1000 * -10 * 1000);
	KeDelayExecutionThread(KernelMode, FALSE, &interval);

	for (i = 0; i < MAX_COM_ID; i++)
	{
		if (s_FltObj[i] != NULL)
		{
			IoDeleteDevice(s_FltObj[i]);
		}
	}
}

PDEVICE_OBJECT OpenCom(ULONG id, NTSTATUS* pStatus)
{
	UNICODE_STRING strComName;
	static WCHAR szComName[32] = { 0 };
	PFILE_OBJECT pFileObj = NULL;
	PDEVICE_OBJECT pDevObj = NULL;

	memset(szComName, 0, sizeof(WCHAR) * 32);
	RtlStringCchPrintfW(szComName, 32, L"\\Devcie\\Serial%d", id);
	RtlInitUnicodeString(&strComName, szComName);

	*pStatus = IoGetDeviceObjectPointer(&strComName, FILE_ALL_ACCESS, &pFileObj, &pDevObj);

	if (*pStatus == STATUS_SUCCESS)
	{
		ObDereferenceObject(pFileObj);
	}

	return pDevObj;
}

NTSTATUS AttachDevice(PDRIVER_OBJECT pDriverObject, PDEVICE_OBJECT pOldDeviceObject, PDEVICE_OBJECT* ppFltDeviceObject, PDEVICE_OBJECT* pNextDeviceObject)
{
	NTSTATUS status;
	PDEVICE_OBJECT pTopDevObj = NULL;
	status = IoCreateDevice(pDriverObject, 0, NULL, pOldDeviceObject->DeviceType, 0, FALSE, ppFltDeviceObject);
	if (status != STATUS_SUCCESS)
	{
		return status;
	}

	if (pOldDeviceObject->Flags & DO_BUFFERED_IO)
	{
		(*ppFltDeviceObject)->Flags |= DO_BUFFERED_IO;
	}

	if (pOldDeviceObject->Flags & DO_DIRECT_IO)
	{
		(*ppFltDeviceObject)->Flags |= DO_DIRECT_IO;
	}

	if (pOldDeviceObject->Characteristics & FILE_DEVICE_SECURE_OPEN)
	{
		(*ppFltDeviceObject)->Characteristics |= FILE_DEVICE_SECURE_OPEN;
	}
	(*ppFltDeviceObject)->Flags |= DO_POWER_PAGABLE;

	pTopDevObj = IoAttachDeviceToDeviceStack(*ppFltDeviceObject, pOldDeviceObject);
	if (pTopDevObj == NULL)
	{
		IoDeleteDevice(*ppFltDeviceObject);
		*ppFltDeviceObject = NULL;
		status = STATUS_UNSUCCESSFUL;
		return status;
	}

	*pNextDeviceObject = pTopDevObj;
	(*ppFltDeviceObject)->Flags = (*ppFltDeviceObject)->Flags & ~DO_DEVICE_INITIALIZING;
	return STATUS_SUCCESS;
}

VOID AttachAllComs(PDRIVER_OBJECT pDriverObject)
{
	for (ULONG i = 0; i < MAX_COM_ID; i++)
	{
		NTSTATUS status;
		PDEVICE_OBJECT pComObj = NULL;
		pComObj = OpenCom(i, &status);
		if (pComObj == NULL)
		{
			continue;
		}

		AttachDevice(pDriverObject, pComObj, &s_FltObj[i], &s_TopObjectBeforeAttach[i]);
	}
}


NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);
	size_t i;
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		pDriverObject->MajorFunction[i] = ccpDispatch;
	}

	pDriverObject->DriverUnload = ccpUnload;
	AttachAllComs(pDriverObject);

	return STATUS_SUCCESS;
}