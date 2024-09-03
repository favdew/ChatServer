#ifndef LOGINWNDPROC
#define LOGINWNDPROC

#ifndef WINDOWSTATUS
#include <WindowStatus.h>
#endif

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#include <WndProc/LoadingWndProc.h>
using std::min;
using std::max;
#include <gdiplus.h>
#include <CommCtrl.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

static HWND hWndLogin, hWndId, hWndPassword;

LRESULT APIENTRY LoginEditProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LoginWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#endif