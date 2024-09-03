#ifndef LOADINGWNDPROC
#define LOADINGWNDPROC

#include <functional>
#include <thread>
#include <mutex>

#ifndef WINDOWSTATUS
#include <WindowStatus.h>
#endif

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#include <Uxtheme.h>

#define TM_TERMINATE 1

typedef std::function<void()> LoadingProcessor;
typedef LPCWSTR LoadingMessage;

static LoadingProcessor ldproc;
static LoadingMessage loadingMessage;
static std::thread* order_processor = nullptr;
static std::mutex proc_mutex;
static bool is_terminate = false;

static BOOL CALLBACK LoadingWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
int CallLoadingWndProc(HWND hWnd, LoadingMessage message, LoadingProcessor proc);

#endif