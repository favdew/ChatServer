#ifndef IDPASSFINDWNDPROC
#define IDPASSFINDWNDPROC

#ifndef WINDOWSTATUS
#include <WindowStatus.h>
#endif

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#include <WndProc/LoadingWndProc.h>
#include <CommCtrl.h>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

LRESULT CALLBACK IdPassFindWndProc(HWND, UINT, WPARAM, LPARAM);

#endif