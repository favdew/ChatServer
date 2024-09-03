#include "NewPassSetWndProc.h"

LRESULT CALLBACK NewPassSetWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	const static int exitButtonWidth = 30, exitButtonHeight = 30;
	const static int exitButtonMarginWidth = 0, exitButtonMarginHeight = 0;

	const static int editWidth = 300, editHeight = 30;
	static int editMarginWidth;
	const static int editMarginHeight = 50;
	const static int firstEditMarginHeight = 150;

	const static int confirmButtonWidth = 300, confirmButtonHeight = 30;
	static int confirmButtonMarginWidth;
	const static int confirmButtonMarginHeight = 50;

	static HWND hWndExitBtn, hWndPassEdit, hWndPassConfirmEdit,
		hWndConfirmBtn;

	enum { HM_EXITBTN = 1, HM_CONFIRMBTN };

	switch (iMessage)
	{
	case WM_CREATE:
	{
		int tempHeight = 0;
		GetClientRect(hWnd, &windowRect);
		editMarginWidth = (windowRect.right - editWidth) / 2;
		confirmButtonMarginWidth = (windowRect.right - editWidth) / 2;

		hWndExitBtn = CreateWindow(TEXT("button"), TEXT("←"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			exitButtonMarginWidth, exitButtonMarginHeight, exitButtonWidth, exitButtonHeight,
			hWnd, (HMENU)HM_EXITBTN, g_hInst, NULL);

		tempHeight += editMarginHeight + firstEditMarginHeight;
		hWndPassEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_PASSWORD,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);

		tempHeight += editHeight + editMarginHeight;
		hWndPassConfirmEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_PASSWORD,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);

		tempHeight += editHeight + confirmButtonMarginHeight;
		hWndConfirmBtn = CreateWindow(TEXT("button"), TEXT("확인"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			confirmButtonMarginWidth, tempHeight, confirmButtonWidth, confirmButtonHeight,
			hWnd, (HMENU)HM_CONFIRMBTN, g_hInst, NULL);

		SendMessage(hWndPassEdit, EM_SETCUEBANNER, 0, (LPARAM)L"새 비밀번호");
		SendMessage(hWndPassConfirmEdit, EM_SETCUEBANNER, 0, (LPARAM)L"새 비밀번호 확인");
		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case HM_EXITBTN:
		{
			CreateWindow(lpszLoginClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
				CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
				NULL, (HMENU)NULL, g_hInst, NULL);
			currentWindowStatus = STS_LOGINWINDOW;
			DestroyWindow(hWnd);
			break;
		}
		case HM_CONFIRMBTN:
		{
			TCHAR input_passwd[256], input_passwd_conf[256];

			GetWindowText(hWndPassEdit, input_passwd, sizeof(input_passwd));
			GetWindowText(hWndPassConfirmEdit, input_passwd_conf, sizeof(input_passwd_conf));

			if (lstrlenW(input_passwd) <= 0 || lstrlenW(input_passwd_conf) <= 0)
			{
				MessageBox(hWnd, TEXT("모든 사항을 기입해야 합니다"), TEXT("안내"), MB_OK);
				return 0;
			}
			else if (lstrcmpW(input_passwd, input_passwd_conf))
			{
				MessageBox(hWnd, TEXT("입력한 비밀번호가 다릅니다"), TEXT("안내"), MB_OK);
				return 0;
			}
			else
			{
				tcp::resolver resolver(io_context);
				auto endpoints = resolver.resolve(connect_ip, connect_port);
				message_proc* proc = nullptr;
				int passchg_result = -1;

				CallLoadingWndProc(hWnd, TEXT("요청중"),
					[&endpoints, &hWnd, &proc, &input_passwd, &input_passwd_conf,
						&passchg_result]()
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
							char upwd[MAX_PASSWORD_LENGTH];

							TcharToChar(uid, input_user_id_buffer);
							TcharToChar(upwd, input_passwd);

							passchg_result = mproc->request_change_password(uid, upwd);
						}
						catch (std::exception e)
						{
							MessageBox(hWnd, TEXT("서버에 연결할 수 없습니다"),
								TEXT("connect error"), MB_OK);
							delete mproc;
							mproc = nullptr;
						}
					});

				if (!passchg_result)
				{
					MessageBox(hWnd, TEXT("비밀번호 변경에 성공하였습니다\n 초기화면으로 이동합니다"),
						TEXT("안내"), MB_OK);
					CreateWindow(lpszLoginClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
						NULL, (HMENU)NULL, g_hInst, NULL);
					currentWindowStatus = STS_LOGINWINDOW;
					DestroyWindow(hWnd);
				}
				else if(passchg_result && mproc != nullptr)
				{
					MessageBox(hWnd, TEXT("오류가 발생하였습니다\n 초기화면으로 이동합니다"),
						TEXT("안내"), MB_OK);
					CreateWindow(lpszLoginClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
						NULL, (HMENU)NULL, g_hInst, NULL);
					currentWindowStatus = STS_LOGINWINDOW;
					DestroyWindow(hWnd);
				}
			}

			break;
		}
		}
		return 0;
	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT draw_struct = (LPDRAWITEMSTRUCT)lParam;
		if (draw_struct->hwndItem == hWndExitBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		else if (draw_struct->hwndItem == hWndConfirmBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		return 0;
	}
	case WM_DESTROY:
		if (currentWindowStatus == STS_NEWPASSSETWINDOW)
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}