#include "LoginWndProc.h"

#pragma comment (lib,"Gdiplus.lib")

LRESULT APIENTRY LoginEditProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CHAR:
	{
		switch (wParam)
		{
		case VK_TAB:
			HWND actived;
			actived = GetFocus();

			if (actived == hWndId)
			{
				SetFocus(hWndPassword);
			}
			else if (actived == hWndPassword)
			{
				SetFocus(hWndId);
			}
			return 0;
		case VK_RETURN:
			SendMessage(hWndLogin, WM_COMMAND, 1, NULL);
			return 0;
		}
		break;
	}
	}
	return CallWindowProc(ChangeEditBorderProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK LoginWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	const static int staticWidth = 300, staticHeight = 200;
	static int staticMarginWidth;
	const static int staticMarginHeight = 50;

	const static int buttonWidth = 300, buttonHeight = 30;
	static int buttonMarginWidth;
	const static int buttonMarginHeight = 50;

	const static int subButtonWidth = 125, subButtonHeight = 30;
	static int subButtonMarginWidth;
	const static int subButtonMarginHeight = 50;

	static HWND hWndLogoStc;
	static HWND hWndIdEdit, hWndPasswordEdit, hWndLoginBtn;
	static HWND hWndRegisterBtn, hWndFindBtn;

	static Gdiplus::Bitmap logoBitmap(L"logo.png");

	enum { HM_LOGINBTN = 1, HM_REGISTERBTN, HM_FINDBTN };
	switch (iMessage)
	{
	case WM_CREATE:
	{
		int tempHeight = staticMarginHeight;

		hWndLogin = hWnd;

		GetClientRect(hWnd, &windowRect);
		staticMarginWidth = (windowRect.right - staticWidth) / 2;
		buttonMarginWidth = (windowRect.right - buttonWidth) / 2;
		subButtonMarginWidth = (windowRect.right - buttonWidth) / 2;

		hWndLogoStc = CreateWindow(TEXT("static"), TEXT("Only Text"), WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
			staticMarginWidth, tempHeight, staticWidth, staticHeight, hWnd, (HMENU)NULL, g_hInst, NULL);

		tempHeight += staticHeight + buttonMarginHeight;
		hWndIdEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			buttonMarginWidth, tempHeight, buttonWidth, buttonHeight,
			hWnd, (HMENU)NULL, g_hInst, NULL);

		hWndId = hWndIdEdit;
		SetWindowLong(hWndIdEdit, GWL_WNDPROC, (LONG)LoginEditProc);

		tempHeight += buttonHeight + buttonMarginHeight;
		hWndPasswordEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
			buttonMarginWidth, tempHeight, buttonWidth, buttonHeight,
			hWnd, (HMENU)NULL, g_hInst, NULL);

		hWndPassword = hWndPasswordEdit;
		SetWindowLong(hWndPasswordEdit, GWL_WNDPROC, (LONG)LoginEditProc);

		tempHeight += buttonHeight + buttonMarginHeight;
		hWndLoginBtn = CreateWindow(TEXT("button"), TEXT("로그인"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_PUSHBUTTON,
			buttonMarginWidth, tempHeight, buttonWidth, buttonHeight,
			hWnd, (HMENU)HM_LOGINBTN, g_hInst, NULL);

		tempHeight += buttonHeight + subButtonMarginHeight;
		hWndRegisterBtn = CreateWindow(TEXT("button"), TEXT("회원가입"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_PUSHBUTTON,
			subButtonMarginWidth, tempHeight, subButtonWidth, subButtonHeight,
			hWnd, (HMENU)HM_REGISTERBTN, g_hInst, NULL);

		hWndFindBtn = CreateWindow(TEXT("button"), TEXT("ID/PW 찾기"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_PUSHBUTTON,
			windowRect.right - subButtonWidth - subButtonMarginWidth, tempHeight, subButtonWidth, subButtonHeight,
			hWnd, (HMENU)HM_FINDBTN, g_hInst, NULL);

		SendMessage(hWndIdEdit, EM_SETCUEBANNER, 0, (LPARAM)L"아이디");
		SendMessage(hWndPasswordEdit, EM_SETCUEBANNER, 0, (LPARAM)L"비밀번호");

		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case HM_LOGINBTN:
		{
			GetWindowText(hWndIdEdit, user_id, sizeof(user_id));
			TCHAR user_passwd[1024];
			GetWindowText(hWndPasswordEdit, user_passwd, sizeof(user_passwd));
			if (lstrlenW(user_id) == 0) {
				MessageBox(hWnd, TEXT("아이디를 입력해주세요"), TEXT("알림"),
					MB_OK);
				break;
			}
			else if (lstrlenW(user_passwd) == 0)
			{
				MessageBox(hWnd, TEXT("비밀번호를 입력해주세요"), TEXT("알림"),
					MB_OK);
				break;
			}

			tcp::resolver resolver(io_context);
			auto endpoints = resolver.resolve(connect_ip, connect_port);
			message_proc* proc = nullptr;
			int login_result;
			CallLoadingWndProc(hWnd, TEXT("로그인 중"),
				[&endpoints, &hWnd, &proc, &user_passwd, &login_result]()
				{
					try {
						if (mproc == nullptr)
						{
							proc = new message_proc(io_context, endpoints);
							mproc = proc;
							mproc->set_cproc(cproc);
							std::string public_key;
							cproc->SavePublicKey(public_key);
							mproc->request_create_key(public_key.c_str(), public_key.size());
						}

						char uid[MAX_ID_LENGTH];
						char upwd[1024];
						TcharToChar(uid, user_id);
						TcharToChar(upwd, user_passwd);
						login_result = mproc->request_login(uid, upwd);
						std::memset(user_name, 0, sizeof(user_name));
						wsprintf(user_name, TEXT("nick%s"), user_id);
					}
					catch (std::exception e)
					{
						MessageBox(hWnd, TEXT("서버에 연결할 수 없습니다"),
							TEXT("connect error"), MB_OK);
						delete mproc;
						mproc = nullptr;
					}
				});

			if (!login_result && mproc != nullptr)
			{
				CreateWindow(lpszMainClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
					CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
					NULL, (HMENU)NULL, g_hInst, NULL);
				currentWindowStatus = STS_MAINWINDOW;
				DestroyWindow(hWnd);
			}
			else if (login_result == -1 && mproc != nullptr)
			{
				MessageBox(hWnd, TEXT("아이디와 비밀번호가 일치하지 않습니다"),
					TEXT("안내"), MB_OK);
				delete mproc;
				mproc = nullptr;
			}
			else if (login_result == -2 && mproc != nullptr)
			{
				MessageBox(hWnd, TEXT("이미 로그인 중인 아이디 입니다"),
					TEXT("안내"), MB_OK);
				delete mproc;
				mproc = nullptr;
			}
			break;
		}
		case HM_REGISTERBTN:
			CreateWindow(lpszRegisterClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
				CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
				NULL, (HMENU)NULL, g_hInst, NULL);
			currentWindowStatus = STS_REGISTERWINDOW;
			DestroyWindow(hWnd);
			break;
		case HM_FINDBTN:
			CreateWindow(lpszIdPassFindClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
				CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
				NULL, (HMENU)NULL, g_hInst, NULL);
			currentWindowStatus = STS_IDPASSFINDWINDOW;
			DestroyWindow(hWnd);
			break;
		}
		return 0;
	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT draw_struct = (LPDRAWITEMSTRUCT)(lParam);
		HWND item_id = draw_struct->hwndItem;
		if (item_id == hWndLoginBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		else if (item_id == hWndRegisterBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		else if (item_id == hWndFindBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		else if (item_id == hWndLogoStc)
		{
			HDC hdc = draw_struct->hDC;
			FillRect(hdc, &(draw_struct->rcItem), (HBRUSH)GetStockObject(NULL_BRUSH));

			ULONG_PTR gdiplusToken;
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
			Gdiplus::Image* image = new Gdiplus::Image(L"logo.png");
			{
				Gdiplus::Graphics g(draw_struct->hDC);
				Gdiplus::Rect rt;
				rt.X = draw_struct->rcItem.left;
				rt.Y = draw_struct->rcItem.top;
				rt.Width = draw_struct->rcItem.right - draw_struct->rcItem.left;
				rt.Height = draw_struct->rcItem.bottom - draw_struct->rcItem.top;
				g.DrawImage(image, rt);
			}

			delete image;
			Gdiplus::GdiplusShutdown(gdiplusToken);
		}
		return 0;
	}
	case WM_DESTROY:
		if (currentWindowStatus == STS_LOGINWINDOW)
			PostQuitMessage(0);
		return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}