//#include <ntddk.h>
#include <ntifs.h>

BOOLEAN EventOperationDemo()
{
	BOOLEAN bSucc = FALSE;
	HANDLE hCreateEvent = NULL;
	HANDLE hOpenEvent = NULL;
	PVOID pCreateEventObject = NULL;
	PVOID pOpenEventObject = NULL;

	do 
	{
		OBJECT_ATTRIBUTES ObjAttr = { 0 };
		UNICODE_STRING	uNameString = { 0 };
		RtlInitUnicodeString(&uNameString, L"\\BaseNamedObject\\TestEvent");
		InitializeObjectAttributes(&ObjAttr, &uNameString, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
		ZwCreateEvent(&hCreateEvent, EVENT_ALL_ACCESS, &ObjAttr, SynchronizationEvent, FALSE);
		if (hCreateEvent == NULL)
		{
			break;
		}

		ObReferenceObjectByHandle(hCreateEvent, EVENT_ALL_ACCESS, *ExEventObjectType, KernelMode, &pCreateEventObject, NULL);
		if (pCreateEventObject == NULL)
		{
			break;
		}

		ZwOpenEvent(&hOpenEvent, EVENT_ALL_ACCESS, &ObjAttr);
		if (hOpenEvent == NULL)
		{
			break;
		}

		ObReferenceObjectByHandle(hOpenEvent, EVENT_ALL_ACCESS, *ExEventObjectType, KernelMode, &pOpenEventObject, NULL);
		if (pOpenEventObject == NULL)
		{
			break;
		}

		bSucc = TRUE;
	} while (FALSE);

	if (pCreateEventObject)
	{
		ObDereferenceObject(pCreateEventObject);
		pCreateEventObject = NULL;
	}

	if (hCreateEvent)
	{
		ZwClose(hCreateEvent);
		hCreateEvent = NULL;
	}

	if (pOpenEventObject)
	{
		ObDereferenceObject(pOpenEventObject);
		pOpenEventObject = NULL;
	}

	if (hOpenEvent)
	{
		ZwClose(hOpenEvent);
		hOpenEvent = NULL;
	}

	return bSucc;
}