#include <Windows.h>
#include <iostream>
#include <tchar.h>

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

int main()
{
	TCHAR szDriverName[] = _T("MyDriver1");
	TCHAR szDriverPath[] = _T("MyDriver1.sys");

	BOOL bRet = LoadNTDriver(szDriverName, szDriverPath);
	if (!bRet)
	{
		printf("LoadNTDriver failed.\n");
		return 0;
	}

	printf("LoadNTDriver succeed. Press any key unload NT driver.\n");
	getchar();

	UnloadNTDriver(szDriverName);
	if (!bRet)
	{
		printf("UnloadNTDriver failed.\n");
		return 0;
	}

	return 0;
}