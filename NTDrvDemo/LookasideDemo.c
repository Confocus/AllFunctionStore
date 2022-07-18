#include <ntddk.h>

BOOLEAN UseLookasideDemo()
{
	PNPAGED_LOOKASIDE_LIST pLookasideList = NULL;
	PVOID pFirstMemory = NULL;
	do 
	{
		pLookasideList = (PNPAGED_LOOKASIDE_LIST)ExAllocatePoolWithTag(NonPagedPool, sizeof(NPAGED_LOOKASIDE_LIST), 'test');
		if (NULL == pLookasideList)
		{
			break;
		}
		memset(pLookasideList, 0, sizeof(NPAGED_LOOKASIDE_LIST));
		ExInitializeNPagedLookasideList(pLookasideList, NULL, NULL, 0, 128, 'test', 0);
		pFirstMemory = ExAllocateFromPagedLookasideList(pLookasideList);
		if (NULL == pFirstMemory)
		{
			break;
		}

		ExFreeToNPagedLookasideList(pLookasideList, pFirstMemory);
		pFirstMemory = NULL;
	} while (FALSE);

	if (pFirstMemory)
	{
		ExFreeToNPagedLookasideList(pLookasideList, pFirstMemory);
		pFirstMemory = NULL;
	}

	if (pLookasideList)
	{
		ExFreePoolWithTag(pLookasideList, 'test');
		pLookasideList = NULL;
	}
}