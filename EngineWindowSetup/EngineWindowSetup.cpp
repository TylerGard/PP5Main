// EngineWindowSetup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "EngineWindowSetup.h"
#include "WinMain.h"
#include "..\EngineWindowSetup\fbxDLL\fbxDLL.h"
#include <chrono>


#define MAX_LOADSTRING 100

// Global Variables:
HWND hWnd;                                   // window handle
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int windowWidth = 500;
int windowHeight = 500;
float mouseX;
float mouseY;
WindowClass windowClass;
fbxNS::fbxFunctions fbxDLLAPI;
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void FbxToDirectX() {
	windowClass.index = fbxDLLAPI.getIndices();
	std::vector<fbxNS::vertex> vertexS = fbxDLLAPI.getVertex();
	for (int i = 0; i < fbxDLLAPI.getVertex().size(); i++) {
		VertexPosColor temp;
		temp.Position.x = vertexS[i].position[0];
		temp.Position.y = vertexS[i].position[1];
		temp.Position.z = vertexS[i].position[2];
		//temp.Position.w = 1.0;
		temp.Color.x = 0.0f;
		temp.Color.y = 0.0f;
		temp.Color.z = 1.0f;
		temp.Color.w = 1.0f;

		windowClass.vertices.push_back(temp);
	}

	std::vector<xyzw> vertexST = fbxDLLAPI.getBoneVertex();
	for (int i = 0; i < fbxDLLAPI.getBoneVertex().size(); i++) {
		VertexPosColor temp;
		temp.Position.x = vertexST[i].x;
		temp.Position.y = vertexST[i].y;
		temp.Position.z = vertexST[i].z;
		//temp.Position.w = 1.0;
		temp.Color.x = 1.0f;
		temp.Color.y = 0.0f;
		temp.Color.z = 0.0f;
		temp.Color.w = 1.0f;

		windowClass.boneVertices.push_back(temp);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
#ifndef NDEBUG
	AllocConsole();
	FILE* new_std_in_out;
	freopen_s(&new_std_in_out, "CONOUT$", "w", stdout);
	freopen_s(&new_std_in_out, "CONIN$", "r", stdin);
#endif
	fbxDLLAPI.initializeFBX();
	
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ENGINEWINDOWSETUP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }
	FbxToDirectX();

	windowClass.start(hWnd, windowWidth, windowHeight);
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ENGINEWINDOWSETUP));

    MSG msg;
	std::chrono::time_point<std::chrono::system_clock> lastnow;
	lastnow = std::chrono::system_clock::now();
	float time = 0;
	float deltaTime = 0;

    // Main message loop:
	bool run = true;
    while (run)
    {
		deltaTime = (float)(std::chrono::system_clock::now() - lastnow).count() / 1e7f;
		time += deltaTime;
		while (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);

			if (msg.message == WM_QUIT)
			{
				run = false;
			}
		}
		
		if (GetKeyState(VK_RBUTTON) < 0)
		{
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(hWnd, &point);

			mouseX = (float)(point.x) / windowWidth - 0.5f;
			mouseY = (float)(point.y) / windowHeight - 0.5f;

			windowClass.UpdateMouse(deltaTime, mouseX, mouseY);

			point.x = windowWidth / 2;
			point.y = windowHeight / 2;

			ClientToScreen(hWnd, &point);
			SetCursorPos(point.x, point.y);
		}

		if (GetAsyncKeyState('Z') & 0x1)
		{
			windowClass.debugSwitch();
		}

		windowClass.Render();
    }

	


	//delete windowClass;
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENGINEWINDOWSETUP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ENGINEWINDOWSETUP);
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

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
      CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

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
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
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
