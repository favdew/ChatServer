#include "WindowStatus.h"

HINSTANCE g_hInst = NULL;

RECT windowRect;
int currentWindowStatus = 0;

const int windowWidth = 400;
const int windowHeight = 650;

boost::asio::io_context io_context;
message_proc* mproc = nullptr;
crypto_proc* cproc = nullptr;

TCHAR user_id[1024];
TCHAR user_name[1024];
TCHAR room_name[1024];

TCHAR input_user_id_buffer[1024];

char connect_ip[16];
char connect_port[8];

LPCTSTR lpszLoginClass = TEXT("Chatting");
LPCTSTR lpszMainClass = TEXT("Main");
LPCTSTR lpszTalkClass = TEXT("Talk");
LPCTSTR lpszRegisterClass = TEXT("Register");
LPCTSTR lpszIdPassFindClass = TEXT("IdPassFind");
LPCTSTR lpszNewPassSetClass = TEXT("NewPassSet");
LPCTSTR lpszRoomClass = TEXT("Room");
LPCTSTR lpszCustomColoredEditClass = TEXT("CustomColoredEditClass");

LRESULT APIENTRY ChangeEditBorderProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_PAINT:
	{
		HDC hdc;
		RECT window_rt;
		HPEN hPen;
		HBRUSH hBrush;
		HGDIOBJ h_old_pen, h_old_brush;

		CallWindowProc(original_edit_proc, hWnd, iMessage, wParam, lParam);

		hdc = GetDC(hWnd);
		GetWindowRect(hWnd, &window_rt);
		hPen = CreatePen(PS_SOLID, 1, edit_border_color);
		hBrush = CreateSolidBrush(NULL_BRUSH);

		h_old_pen = SelectObject(hdc, hPen);
		h_old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, 0, 0, window_rt.right - window_rt.left, window_rt.bottom - window_rt.top);

		SelectObject(hdc, h_old_pen);
		SelectObject(hdc, h_old_brush);
		DeleteObject(hBrush);
		DeleteObject(hPen);

		return TRUE;
	}
	}


	return CallWindowProc(original_edit_proc, hWnd, iMessage, wParam, lParam);
}

void wsinit()
{
	std::ifstream fs;
	fs.open("connect_conf.conf");
	if (fs.is_open())
	{
		std::string ip, port;
		
		std::getline(fs, ip);
		std::getline(fs, port);

		std::memset(connect_ip, 0, sizeof(connect_ip));
		std::memset(connect_port, 0, sizeof(connect_port));

		std::strcpy(connect_ip, ip.c_str());
		std::strcpy(connect_port, port.c_str());
	}
	fs.close();

	HWND hWndEdit = CreateWindow(TEXT("edit"), NULL, NULL, 0, 0, 0, 0, (HWND)NULL,
		(HMENU)NULL, g_hInst, NULL);
	original_edit_proc = (WNDPROC)GetWindowLong(hWndEdit, GWL_WNDPROC);

	WNDCLASS wclass;
	
	GetClassInfo((HINSTANCE)GetWindowLong(hWndEdit, GWL_HINSTANCE), TEXT("edit"),
		&wclass);

	wclass.lpfnWndProc = ChangeEditBorderProc;
	wclass.lpszClassName = lpszCustomColoredEditClass;

	RegisterClass(&wclass);

	cproc = new crypto_proc();
}