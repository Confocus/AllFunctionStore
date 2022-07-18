#include <Windows.h>
#include <iostream>
#include <tchar.h>
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

BOOL LoadNTDriver(TCHAR* lpszDriverName, TCHAR* lpszDriverPath)
{
	TCHAR szDriverImagePath[256];
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);
	wprintf(L"Driver image path is:%ls\n", szDriverImagePath);
	BOOL bRet = FALSE;
	DWORD dwErrCode = 0;
	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;

	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		dwErrCode = GetLastError();
		printf("OpenSCManager failed. Error code is %d\n", dwErrCode);
		goto BeforeLeave;
	}
	else
	{
		printf("OpenSCManager succeed.\n");
	}

	hServiceDDK = CreateService(
		hServiceMgr,
		lpszDriverName,
		lpszDriverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		szDriverImagePath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (NULL == hServiceDDK)
	{
		dwErrCode = GetLastError();
		if (dwErrCode != ERROR_IO_PENDING && dwErrCode != ERROR_SERVICE_EXISTS)
		{
			printf("CreateService failed. Error code:%d\n", dwErrCode);
			goto BeforeLeave;
		}
		else
		{
			printf("CreateService failed. Error code is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS.\n");
		}

		//如果服务已经创建过了
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (NULL == hServiceDDK)
		{
			dwErrCode = GetLastError();
			printf("OpenService failed. Service had been created, but OpenService failed. Error code:%d\n", dwErrCode);
			goto BeforeLeave;
		}
		else
		{
			printf("OpenService succeed!\n");
		}
	}
	else
	{
		printf("CreateService succeed!\n");
	}

	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		dwErrCode = GetLastError();
		if (dwErrCode != ERROR_IO_PENDING && dwErrCode != ERROR_SERVICE_ALREADY_RUNNING)
		{
			printf("StartService is failed. Error code is:%d\n", dwErrCode);
			goto BeforeLeave;
		}
		else
		{
			if (ERROR_IO_PENDING == dwErrCode)
			{
				printf("StartService failed ERROR_IO_PENDING. File:%s. Line:%d\n", __FILE__, __LINE__);
				goto BeforeLeave;
			}
			else
			{
				printf("StartService failed ERROR_SERVICE_ALREAD_RUNNING.\n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}

	bRet = TRUE;
BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}

	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;
}

BOOL UnloadNTDriver(TCHAR* szSvrName)
{
	BOOL bRet = FALSE;
	DWORD dwErrCode = 0;
	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;
	SERVICE_STATUS status;

	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == hServiceMgr)
	{
		dwErrCode = GetLastError();
		printf("OpenSCManager failed. Error code is %d\n", dwErrCode);
		goto BeforeLeave;
	}
	else
	{
		printf("OpenSCManager succeed!\n");
	}

	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);
	if (NULL == hServiceDDK)
	{
		dwErrCode = GetLastError();
		printf("OpenService failed. Error code is %d.\n", dwErrCode);
		goto BeforeLeave;
	}
	else
	{
		printf("OpenService succeed!\n");
	}

	if (ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &status))
	{
		dwErrCode = GetLastError();
		printf("ControlService SERVICE_CONTROL_STOP failed. Error code is %d.\n", dwErrCode);
	}
	else
	{
		printf("ControlService succeed.\n");
	}

	if (!DeleteService(hServiceDDK))
	{
		dwErrCode = GetLastError();
		printf("DeleteService failed. Error code is %d.\n", dwErrCode);
	}
	else
	{
		printf("DeleteService succeed.\n");
	}

	bRet = TRUE;

BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}

	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;
}

//参数一：-start表示安装，-stop表示卸载
int main(int argc, char* argv[])
{
	TCHAR szDriverName[] = _T("NTDrvDemo");
	TCHAR szDriverPath[] = _T("NTDrvDemo.sys");
	BOOL bRet = FALSE;

	if (_stricmp(argv[1], "-start") == 0)
	{
		bRet = LoadNTDriver(szDriverName, szDriverPath);
		if (!bRet)
		{
			printf("LoadNTDriver failed.\n");
			return 0;
		}

		printf("LoadNTDriver succeed. Press any key unload NT driver.\n");
	}
	else if (_stricmp(argv[1], "-stop") == 0)
	{
		bRet = UnloadNTDriver(szDriverName);
		if (!bRet)
		{
			printf("UnloadNTDriver failed.\n");
			return 0;
		}

		printf("UnloadNTDriver succeed. Press any key unload NT driver.\n");
	}
	
	getchar();

	return 0;
}
//IMAGE_DIRECTORY_ENTRY_SECURITY
//int main()
//{
//	HANDLE hDevice = CreateFile(L"\\\\.\\MyNTDriver",
//		GENERIC_READ | GENERIC_WRITE,
//		0,
//		NULL,
//		OPEN_EXISTING,
//		FILE_ATTRIBUTE_NORMAL,
//		NULL);
//	if (INVALID_HANDLE_VALUE == hDevice)
//	{
//		printf("CreateFile MyNTDriver failed. LastError:%d\n", GetLastError());
//		return 1;
//	}
//
//	/*UCHAR buffer[10] = { 0 };
//	ULONG ulRead = 0;
//	BOOL bRet = FALSE;
//	bRet = ReadFile(hDevice, buffer, 10, &ulRead, NULL);
//	if (bRet)
//	{
//		printf("Read %d bytes from buffer.\n", ulRead);
//		printf("Buffer addr 0x%08X bytes from buffer.\n", (ULONG)buffer);
//
//		for (int i = 0; i < (int)ulRead; i++)
//		{
//			printf("%02X ", buffer[i]);
//		}
//		printf("\n");
//	}*/
//
//	/////////////////////////测试BufferedIO/////////////////////////////////////////////////
//	/*UCHAR buffer2[10] = { 0 };
//	memset(buffer2, 0x70, 10);
//	ULONG ulWrite = 0;
//	bRet = WriteFile(hDevice, buffer2, 10, &ulWrite, NULL);
//	if (bRet)
//	{
//		printf("Write %d bytes.\n", ulWrite);
//	}
//
//	bRet = ReadFile(hDevice, buffer, 10, &ulRead, NULL);
//	if (bRet)
//	{
//		printf("Read %d bytes from buffer.\n", ulRead);
//		for (int i = 0; i < (int)ulRead; i++)
//		{
//			printf("%02X ", buffer[i]);
//		}
//		printf("\n");
//	}*/
//
//	//测试DeviceIoControl
//	/*char bufferIn[10] = { 63,63,63,63,63,63,63,63,63,63 };
//	char bufferOut[10] = { 0 };
//	ULONG ulRetSize = 0;
//	DeviceIoControl(hDevice,
//		IOCTL_TEST1,
//		bufferIn,
//		10,
//		bufferOut,
//		10,
//		&ulRetSize,
//		NULL
//	);
//
//	CloseHandle(hDevice);*/
//
//	//应用层与内核层使用Event进行通信
//	BOOL bRet = FALSE;
//	DWORD dwOutput = 0;
//	HANDLE hEvent = NULL;
//
//	//自动的且未触发的事件
//	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
//	if (NULL == hEvent)
//	{
//		goto ErrorFlag;
//	}
//	bRet = DeviceIoControl(hDevice,
//		IOCTL_TRANSMIT_EVENT,
//		&hEvent,
//		sizeof(hEvent),
//		NULL,
//		0,
//		&dwOutput,
//		NULL);
//
//	WaitForSingleObject(hEvent, INFINITE);
//	printf("Get event..\n");
//
//ErrorFlag:
//	if (hEvent)
//	{
//		CloseHandle(hEvent);
//	}
//	if (hDevice)
//	{
//		CloseHandle(hDevice);
//	}
//	getchar();
//	return 0;
//}