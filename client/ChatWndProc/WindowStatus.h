#ifndef WINDOWSTATUS
#define WINDOWSTATUS

#define BOOST_ASIO_NO_WIN32_LEAN_AND_MEAN
#include <fstream>
#include <memory>
#include <boost/asio.hpp>
#include <message_proc.h>
#include <crypto_proc.h>
#include "resource1.h"

#ifndef _WINDOWS_
#include <Windows.h>
#endif

extern HINSTANCE g_hInst;

extern RECT windowRect;
extern int currentWindowStatus;

extern const int windowWidth;
extern const int windowHeight;

extern boost::asio::io_context io_context;
extern message_proc* mproc;
extern crypto_proc* cproc;

extern TCHAR user_id[1024];
extern TCHAR user_name[1024];
extern TCHAR room_name[1024];

extern TCHAR input_user_id_buffer[1024];

extern char connect_ip[16];
extern char connect_port[8];

enum windowStatus {
	STS_LOGINWINDOW = 1, STS_MAINWINDOW,
	STS_TALKWINDOW, STS_REGISTERWINDOW, STS_IDPASSFINDWINDOW,
	STS_NEWPASSSETWINDOW
};

const COLORREF background_color = RGB(217, 255, 255);
const COLORREF button_border_color = RGB(5, 205, 196);
const COLORREF button_color = RGB(21, 249, 237);
const COLORREF on_click_button_color = RGB(118, 252, 245);
const COLORREF button_text_color = RGB(0, 0, 0);
const COLORREF edit_border_color = RGB(119, 255, 255);

extern LPCTSTR lpszLoginClass;
extern LPCTSTR lpszMainClass;
extern LPCTSTR lpszTalkClass;
extern LPCTSTR lpszRegisterClass;
extern LPCTSTR lpszIdPassFindClass;
extern LPCTSTR lpszNewPassSetClass;
extern LPCTSTR lpszRoomClass;
extern LPCTSTR lpszCustomColoredEditClass;

static WNDPROC original_edit_proc;

enum talkDisplayMessage { TM_ADDCONTENT = 0x0501 };

LRESULT APIENTRY ChangeEditBorderProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void wsinit();

#endif