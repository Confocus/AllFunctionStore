// WindowsUI.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsUI.h"

#define MAX_LOADSTRING 100
#define NUM		1000
#define TWOPI	(2 * 3.14159)

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSUI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	HDC hdc;
	static int cyChar;	//字符高度，行高，行距
	static int cxChar;	//字符宽度
	static int cxCaps;	//大写字符宽度
	static int cxClient;
	static int cyClient;	//窗口客户区高度
	TEXTMETRIC tm;	//字符信息依赖的结构体
	TCHAR szBuff[20] = { 0 };
	SCROLLINFO si;
	int iVertPos;
	int iPaintBegin;
	int iPaintEnd;

	int x, y = 0;//表示输出文字的位置
	static int cxScreen;//定义屏幕的大小
	static int cyScreen;

	static POINT apt[4] = { 0 };

	switch (message)
	{
	case WM_SIZE://调整窗口大小消息
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		apt[0].x = cxClient / 4;
		apt[0].y = cyClient / 2;

		apt[1].x = cxClient / 2;
		apt[1].y = cyClient / 4;

		apt[2].x = cxClient / 2;
		apt[2].y = cyClient * 3 / 4;

		apt[3].x = cxClient * 3 / 4;
		apt[3].y = cyClient / 2;
		break;
	case WM_CREATE:
		break;
	case WM_COMMAND:
		break;
	case WM_LBUTTONDOWN:
	{
		//鼠标左键按下去改变控点
		hdc = GetDC(hWnd);
		//从仓库里拿出白色画笔，重新用白色填充贝塞尔曲线，相当于把贝塞尔曲线消失掉
		SelectObject(hdc, GetStockObject(WHITE_PEN));

		PolyBezier(hdc, apt, 4);
		MoveToEx(hdc, apt[0].x, apt[0].y, NULL);
		LineTo(hdc, apt[1].x, apt[1].y);
		MoveToEx(hdc, apt[2].x, apt[2].y, NULL);
		LineTo(hdc, apt[3].x, apt[3].y);

		//更改起点控点的位置
		apt[1].x = LOWORD(lParam);
		apt[1].y = HIWORD(lParam);
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		PolyBezier(hdc, apt, 4);
		MoveToEx(hdc, apt[0].x, apt[0].y, NULL);
		LineTo(hdc, apt[1].x, apt[1].y);
		MoveToEx(hdc, apt[2].x, apt[2].y, NULL);
		LineTo(hdc, apt[3].x, apt[3].y);

		ReleaseDC(hWnd, hdc);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		PolyBezier(hdc, apt, 4);

		//连接起点和控点
		MoveToEx(hdc, apt[0].x, apt[0].y, NULL);
		LineTo(hdc, apt[1].x, apt[1].y);

		//连终点起点和控点
		MoveToEx(hdc, apt[2].x, apt[2].y, NULL);
		LineTo(hdc, apt[3].x, apt[3].y);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_VSCROLL:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
