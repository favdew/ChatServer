#ifndef MAINWNDPROC
#define MAINWNDPROC

#ifndef WINDOWSTATUS
#include <WindowStatus.h>
#endif

#ifndef _WINDOWS_
#include <Windows.h>
#endif
#include <CommCtrl.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <WndProc/LoadingWndProc.h>


static TCHAR input_buffer[MAX_ROOM_NAME_LENGTH];

BOOL CALLBACK JoinRoomDlgProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK RoomCreateDlgProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#endif