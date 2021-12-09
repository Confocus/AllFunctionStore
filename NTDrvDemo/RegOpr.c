#include <ntddk.h>

#define MY_REG_SOFTWARE_KEY_NAME	L"\\Registry\\Machine\\Software\\MyKeyName"

VOID DrvCreateRegDemo()
{
	UNICODE_STRING ustrRegKeyName = { 0 };
	HANDLE hReg = NULL;
	RtlInitUnicodeString(&ustrRegKeyName, MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes,
		&ustrRegKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	ULONG ulResult = 0;
	NTSTATUS status = ZwCreateKey(&hReg,
		KEY_ALL_ACCESS,
		&objectAttributes,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		&ulResult);
	if (NT_SUCCESS(status))
	{
		if (REG_CREATED_NEW_KEY == ulResult)
		{
			DbgPrint("The register item is created.\n");
		}
		else if(REG_OPENED_EXISTING_KEY == ulResult)
		{
			DbgPrint("The register item has been created， and now is opened.\n");
		}
	}
	else
	{
		DbgPrint("ZwCreateKey failed.\n");
		return;
	}

	UNICODE_STRING ustrSubRegKeyName = { 0 };
	HANDLE hSubReg = NULL;
	RtlInitUnicodeString(&ustrSubRegKeyName, L"SubItem");

	OBJECT_ATTRIBUTES subObjectAttributes = { 0 };
	InitializeObjectAttributes(&subObjectAttributes,
		&ustrSubRegKeyName,
		OBJ_CASE_INSENSITIVE,
		hReg,
		NULL);

	status = ZwCreateKey(&hSubReg,
		KEY_ALL_ACCESS,
		&subObjectAttributes,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		&ulResult);
	if (NT_SUCCESS(status))
	{
		if (REG_CREATED_NEW_KEY == ulResult)
		{
			DbgPrint("The sub register item is created.\n");
		}
		else if (REG_OPENED_EXISTING_KEY == ulResult)
		{
			DbgPrint("The sub register item has been created， and now is opened.\n");
		}
	}
	else
	{
		DbgPrint("ZwCreateKey failed.\n");
		return;
	}

	ZwClose(hReg);
	ZwClose(hReg);
}
 
VOID DrvOpenRegDemo()
{
	UNICODE_STRING ustrRegKeyName = { 0 };
	HANDLE hReg = NULL;
	RtlInitUnicodeString(&ustrRegKeyName, MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes,
		&ustrRegKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenKey(&hReg,
		KEY_ALL_ACCESS,
		&objectAttributes);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwOpenKey success.\n");
	}

	ZwClose(hReg);
}

//添加、修改注册表键值
VOID DrvSetRegDemo()
{
	UNICODE_STRING ustrRegKeyName = { 0 };
	HANDLE hReg = NULL;
	RtlInitUnicodeString(&ustrRegKeyName, MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes,
		&ustrRegKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenKey(&hReg,
		KEY_ALL_ACCESS,
		&objectAttributes);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwOpenKey success.\n");
	}
	else
	{
		DbgPrint("ZwOpenKey failed.\n");
		return;
	}

	UNICODE_STRING ustrValueName = { 0 };
	RtlInitUnicodeString(&ustrValueName, L"REG_DWORD value");
	ULONG ulValue = 100;
	ZwSetValueKey(hReg,
		&ustrValueName,
		0,
		REG_DWORD,
		&ulValue,
		sizeof(ulValue));

	DbgPrint("ZwSetValueKey REG_DWORD finished.\n");

	RtlInitUnicodeString(&ustrValueName, L"REG_SZ value");
	WCHAR* strValue = L"hello world";

	ZwSetValueKey(hReg,
		&ustrValueName,
		0,
		REG_SZ,
		strValue,
		wcslen(strValue) * 2 + 2);

	DbgPrint("ZwSetValueKey REG_SZ finished.\n");

	RtlInitUnicodeString(&ustrValueName, L"REG_BINARY value");
	UCHAR buffer[10] = { 0 };
	RtlFillMemory(buffer, sizeof(buffer), 0x63);
	ZwSetValueKey(hReg,
		&ustrValueName,
		0,
		REG_BINARY,
		buffer,
		sizeof(buffer));

	DbgPrint("ZwSetValueKey REG_BINARY finished.\n");

	ZwClose(hReg);
}

//查询注册表
VOID DrvQueryRegDemo()
{
	UNICODE_STRING ustrRegKeyName = { 0 };
	HANDLE hReg = NULL;
	RtlInitUnicodeString(&ustrRegKeyName, MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes,
		&ustrRegKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenKey(&hReg,
		KEY_ALL_ACCESS,
		&objectAttributes);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwOpenKey success.\n");
	}
	else
	{
		DbgPrint("ZwOpenKey failed.\n");
		return;
	}

	UNICODE_STRING ustrValueName = { 0 };
	RtlInitUnicodeString(&ustrValueName, L"REG_DWORD value");
	ULONG ulSize = 0;
	status = ZwQueryValueKey(hReg,
		&ustrValueName,
		KeyValuePartialInformation,
		NULL,
		0,
		&ulSize);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND || 0 == ulSize)
	{
		ZwClose(hReg);
		DbgPrint("ZwQueryValueKey REG_DWORD failed. Status:%d, Size:%d.\n", status, ulSize);
		return;
	}
	else
	{
		DbgPrint("ZwQueryValueKey REG_DWORD success.\n");
	}

	PKEY_VALUE_PARTIAL_INFORMATION pvpi = NULL;
	pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)
		ExAllocatePoolWithTag(PagedPool, ulSize, 123);
	if (NULL == pvpi)
	{
		DbgPrint("ExAllocatePoolWithTag pvpi failed.\n");
		return;
	}
	status = ZwQueryValueKey(hReg,
		&ustrValueName,
		KeyValuePartialInformation,
		pvpi,
		ulSize,
		&ulSize);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hReg);
		DbgPrint("ZwQueryValueKey REG_DWORD failed. Status:%d, Size:%d.\n", status, ulSize);
		return;
	}
	else
	{
		DbgPrint("ZwQueryValueKey REG_DWORD success.\n");
	}

	if (pvpi->Type == REG_DWORD && pvpi->DataLength == sizeof(ULONG))
	{
		PULONG pulValue = (PULONG)pvpi->Data;
		DbgPrint("The REG_DWORD value is %d\n.", *pulValue);
	}

	ExFreePool(pvpi);
	pvpi = NULL;

	RtlInitUnicodeString(&ustrValueName, L"REG_SZ value");
	status = ZwQueryValueKey(hReg,
		&ustrValueName,
		KeyValuePartialInformation,
		NULL,
		0,
		&ulSize);
	if (status == STATUS_OBJECT_NAME_NOT_FOUND || 0 == ulSize)
	{
		ZwClose(hReg);
		DbgPrint("ZwQueryValueKey REG_SZ failed. Status:%d, Size:%d.\n", status, ulSize);
		return;
	}
	else
	{
		DbgPrint("ZwQueryValueKey REG_SZ success.\n");
	}

	pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 123);
	if (NULL == pvpi)
	{
		DbgPrint("ExAllocatePoolWithTag pvpi failed.\n");
		return;
	}

	status = ZwQueryValueKey(hReg,
		&ustrValueName,
		KeyValuePartialInformation,
		pvpi,
		ulSize,
		&ulSize);
	if (!NT_SUCCESS(status))
	{
		ZwClose(hReg);
		DbgPrint("ZwQueryValueKey REG_SZ failed. Status:%d, Size:%d.\n", status, ulSize);
		return;
	}
	else
	{
		DbgPrint("ZwQueryValueKey REG_SZ success.\n");
	}

	if (REG_SZ == pvpi->Type)
	{
		DbgPrint("The REG_DWORD value is %S.\n", pvpi->Data);
	}

	ZwClose(hReg);
}

//枚举注册表子项
VOID DrvEnumRegItemDemo()
{
	UNICODE_STRING ustrRegKeyName = { 0 };
	HANDLE hReg = NULL;
	RtlInitUnicodeString(&ustrRegKeyName, MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes,
		&ustrRegKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenKey(&hReg,
		KEY_ALL_ACCESS,
		&objectAttributes);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwOpenKey success.\n");
	}
	else
	{
		DbgPrint("ZwOpenKey failed.\n");
		return;
	}

	ULONG ulSize = 0;
	ZwQueryKey(hReg,
		KeyFullInformation,
		NULL,
		0,
		&ulSize);

	PKEY_FULL_INFORMATION pfi = NULL;
	pfi = (PKEY_FULL_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 123);
	if (NULL == pfi)
	{
		DbgPrint("ExAllocatePoolWithTag failed.\n");
		return;
	}

	ZwQueryKey(hReg,
		KeyFullInformation,
		pfi,
		ulSize,
		&ulSize);

	for (ULONG i = 0; i < pfi->SubKeys; i++)
	{
		ulSize = 0;
		ZwEnumerateKey(hReg,
			i,
			KeyBasicInformation,
			NULL,
			0,
			&ulSize);

		PKEY_BASIC_INFORMATION pbi = NULL;
		pbi = (PKEY_BASIC_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 123);
		if (NULL == pbi)
		{
			DbgPrint("ExAllocatePoolWithTag failed in for loop.\n");
			continue;
		}

		ZwEnumerateKey(hReg,
			i,
			KeyBasicInformation,
			pbi,
			ulSize,
			&ulSize);

		UNICODE_STRING ustrKeyName = { 0 };
		ustrKeyName.Length = ustrKeyName.MaximumLength = (USHORT)pbi->NameLength;
		ustrKeyName.Buffer = pbi->Name;
		DbgPrint("The %d sub item name is %wZ.\n", i, &ustrKeyName);
		ExFreePool(pbi);
	}

	ExFreePool(pfi);
	pfi = NULL;
	ZwClose(hReg);
}

//枚举子键
VOID DrvEnumRegValueKeyDemo()
{
	UNICODE_STRING ustrRegKeyName = { 0 };
	HANDLE hReg = NULL;
	RtlInitUnicodeString(&ustrRegKeyName, MY_REG_SOFTWARE_KEY_NAME);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes,
		&ustrRegKeyName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	NTSTATUS status = ZwOpenKey(&hReg,
		KEY_ALL_ACCESS,
		&objectAttributes);
	if (NT_SUCCESS(status))
	{
		DbgPrint("ZwOpenKey success.\n");
	}
	else
	{
		DbgPrint("ZwOpenKey failed.\n");
		return;
	}

	ULONG ulSize = 0;
	ZwQueryKey(hReg,
		KeyFullInformation,
		NULL,
		0,
		&ulSize);

	PKEY_FULL_INFORMATION pfi = NULL;
	pfi = (PKEY_FULL_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 123);
	if (NULL == pfi)
	{
		DbgPrint("ExAllocatePoolWithTag failed.\n");
		return;
	}

	ZwQueryKey(hReg,
		KeyFullInformation,
		pfi,
		ulSize,
		&ulSize);

	for (ULONG i = 0; i < pfi->Values; i++)
	{
		ulSize = 0;
		ZwEnumerateValueKey(hReg,
			i,
			KeyValueBasicInformation,
			NULL,
			0,
			&ulSize);

		PKEY_VALUE_BASIC_INFORMATION pvbi = NULL;
		pvbi = (PKEY_VALUE_BASIC_INFORMATION)ExAllocatePoolWithTag(PagedPool, ulSize, 123);
		if (NULL == pvbi)
		{
			DbgPrint("ExAllocatePoolWithTag failed in for loop.\n");
			continue;
		}

		ZwEnumerateValueKey(hReg,
			i,
			KeyValueBasicInformation,
			pvbi,
			ulSize,
			&ulSize);

		UNICODE_STRING ustrKeyName = { 0 };
		ustrKeyName.Length = ustrKeyName.MaximumLength = (USHORT)pvbi->NameLength;
		ustrKeyName.Buffer = pvbi->Name;
		DbgPrint("The %d sub item name is %wZ.", i, &ustrKeyName);
		if (REG_SZ == pvbi->Type)
		{
			DbgPrint("Type is REG_SZ.\n");
		}
		else if (REG_MULTI_SZ == pvbi->Type)
		{
			DbgPrint("Type is REG_MULTI_SZ.\n");
		}
		else if (REG_DWORD == pvbi->Type)
		{
			DbgPrint("Type is REG_DWORD.\n");
		}
		else if (REG_BINARY == pvbi->Type)
		{
			DbgPrint("Type is REG_BINARY.\n");
		}
		ExFreePool(pvbi);
	}

	ExFreePool(pfi);
	pfi = NULL;
	ZwClose(hReg);
}

//删除子项操作
VOID DrvDelRegKeyDemo()
{

}

//使用Rtl函数操作注册表