#include <ChatWndProc.h>
#include <Windows.h>
#include "resource.h"
#include "resource1.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass, TalkDisplayClass;
	g_hInst = hInstance;

	wsinit();

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = CreateSolidBrush(background_color);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = LoginWndProc;
	WndClass.lpszClassName = lpszLoginClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszLoginClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
		NULL, (HMENU)NULL, hInstance, NULL);

	currentWindowStatus = STS_LOGINWINDOW;
	ShowWindow(hWnd, nCmdShow);

	WndClass.lpfnWndProc = MainWndProc;
	WndClass.lpszClassName = lpszMainClass;
	RegisterClass(&WndClass);

	WndClass.lpfnWndProc = TalkWndProc;
	WndClass.lpszClassName = lpszTalkClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	RegisterClass(&WndClass);

	WndClass.lpfnWndProc = RegisterWndProc;
	WndClass.lpszClassName = lpszRegisterClass;
	WndClass.lpszMenuName = NULL;
	RegisterClass(&WndClass);

	WndClass.lpfnWndProc = IdPassFindWndProc;
	WndClass.lpszClassName = lpszIdPassFindClass;
	RegisterClass(&WndClass);

	WndClass.lpfnWndProc = NewPassSetWndProc;
	WndClass.lpszClassName = lpszNewPassSetClass;
	RegisterClass(&WndClass);

	HBRUSH hBrush;
	hBrush = CreateSolidBrush(background_color);

	TalkDisplayClass.cbClsExtra = 0;
	TalkDisplayClass.cbWndExtra = 0;
	TalkDisplayClass.hbrBackground = hBrush;
	TalkDisplayClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	TalkDisplayClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	TalkDisplayClass.hInstance = hInstance;
	TalkDisplayClass.lpfnWndProc = TalkDisplayWndProc;
	TalkDisplayClass.lpszClassName = TEXT("talkdisplay");
	TalkDisplayClass.lpszMenuName = NULL;
	TalkDisplayClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&TalkDisplayClass);

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	DeleteObject(hBrush);

	return (int)Message.wParam;
}