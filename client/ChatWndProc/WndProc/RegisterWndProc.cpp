#include "RegisterWndProc.h"

LRESULT CALLBACK RegisterWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	const static int exitButtonWidth = 30, exitButtonHeight = 30;
	const static int exitButtonMarginWidth = 0, exitButtonMarginHeight = 0;

	const static int editWidth = 300, editHeight = 30;
	static int editMarginWidth;
	const static int editMarginHeight = 50;
	const static int firstEditMarginHeight = 50;

	const static int registerButtonWidth = 300, registerButtonHeight = 30;
	static int registerButtonMarginWidth;
	const static int registerButtonMarginHeight = 50;

	static HWND hWndExitBtn, hWndIdEdit, hWndPassEdit, hWndPassConfirmEdit,
		hWndNameEdit, hWndEmailEdit, hWndRegisterBtn;

	enum { HM_EXITBTN = 1, HM_REGISTERBTN };
	switch (iMessage)
	{
	case WM_CREATE:
	{
		int tempHeight = 0;
		GetClientRect(hWnd, &windowRect);
		editMarginWidth = (windowRect.right - editWidth) / 2;
		registerButtonMarginWidth = (windowRect.right - registerButtonWidth) / 2;

		hWndExitBtn = CreateWindow(TEXT("button"), TEXT("��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			exitButtonMarginWidth, exitButtonMarginHeight, exitButtonWidth, exitButtonHeight,
			hWnd, (HMENU)HM_EXITBTN, g_hInst, NULL);

		tempHeight += editMarginHeight + firstEditMarginHeight;
		hWndIdEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);


		tempHeight += editHeight + editMarginHeight;
		hWndPassEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_PASSWORD,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);


		tempHeight += editHeight + editMarginHeight;
		hWndPassConfirmEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_PASSWORD,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);

		tempHeight += editHeight + editMarginHeight;
		hWndNameEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);

		tempHeight += editHeight + editMarginHeight;
		hWndEmailEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			editMarginWidth, tempHeight, editWidth, editHeight,
			hWnd, (HMENU)0, g_hInst, NULL);

		tempHeight += editHeight + editMarginHeight;
		hWndRegisterBtn = CreateWindow(TEXT("button"), TEXT("�����ϱ�"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			registerButtonMarginWidth, tempHeight, registerButtonWidth, registerButtonHeight,
			hWnd, (HMENU)HM_REGISTERBTN, g_hInst, NULL);

		SendMessage(hWndIdEdit, EM_SETCUEBANNER, 0, (LPARAM)L"���̵�");
		SendMessage(hWndPassEdit, EM_SETCUEBANNER, 0, (LPARAM)L"��й�ȣ");
		SendMessage(hWndPassConfirmEdit, EM_SETCUEBANNER, 0, (LPARAM)L"��й�ȣȮ��");
		SendMessage(hWndNameEdit, EM_SETCUEBANNER, 0, (LPARAM)L"�̸�");
		SendMessage(hWndEmailEdit, EM_SETCUEBANNER, 0, (LPARAM)L"email");
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
		case HM_REGISTERBTN:
		{
			TCHAR input_user_id[MAX_ID_LENGTH], input_passwd[256], input_passwd_conf[256],
				input_name[256], input_email[256];

			GetWindowText(hWndIdEdit, input_user_id, sizeof(input_user_id));
			GetWindowText(hWndPassEdit, input_passwd, sizeof(input_passwd));
			GetWindowText(hWndPassConfirmEdit, input_passwd_conf, sizeof(input_passwd_conf));
			GetWindowText(hWndNameEdit, input_name, sizeof(input_name));
			GetWindowText(hWndEmailEdit, input_email, sizeof(input_email));


			if (lstrlenW(input_user_id) <= 0 || lstrlenW(input_passwd) <= 0 ||
				lstrlenW(input_passwd_conf) <= 0 || lstrlenW(input_name) <= 0 ||
				lstrlenW(input_email) <= 0 )
			{
				MessageBox(hWnd, TEXT("��� ������ �����ؾ� �մϴ�"), TEXT("�ȳ�"), MB_OK);
				return 0;
			}
			else if (lstrcmpW(input_passwd, input_passwd_conf))
			{
				MessageBox(hWnd, TEXT("�Է��� ��й�ȣ�� �ٸ��ϴ�"), TEXT("�ȳ�"), MB_OK);
				return 0;
			}
			else
			{
				tcp::resolver resolver(io_context);
				auto endpoints = resolver.resolve(connect_ip, connect_port);
				message_proc* proc = nullptr;
				int register_result = -1;
				CallLoadingWndProc(hWnd, TEXT("��û��"),
					[&endpoints, &hWnd, &proc, &input_user_id, &input_passwd,
						&input_name, &input_email, &register_result]()
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
							char uname[MAX_NAME_LENGTH];
							char uemail[MAX_EMAIL_LENGTH];
							TcharToChar(uid, input_user_id);
							TcharToChar(upwd, input_passwd);
							TcharToChar(uname, input_name);
							TcharToChar(uemail, input_email);
							register_result = mproc->request_register(uid, upwd, uname, uemail);
						}
						catch (std::exception e)
						{
							MessageBox(hWnd, TEXT("������ ������ �� �����ϴ�"),
								TEXT("connect error"), MB_OK);
							delete mproc;
							mproc = nullptr;
						}
					});

				if (!register_result)
				{
					MessageBox(hWnd, TEXT("ȸ�����Կ� �����Ͽ����ϴ�\n �ʱ�ȭ������ �̵��մϴ�"),
						TEXT("�ȳ�"), MB_OK);
					CreateWindow(lpszLoginClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
						NULL, (HMENU)NULL, g_hInst, NULL);
					currentWindowStatus = STS_LOGINWINDOW;
					DestroyWindow(hWnd);
				}
				else if(register_result && mproc != nullptr)
				{
					MessageBox(hWnd, TEXT("�̹� �����ϴ� ���̵� �Դϴ�"), TEXT("�ȳ�"), MB_OK);
					return 0;
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
		else if (draw_struct->hwndItem == hWndRegisterBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		return 0;
	}
	case WM_DESTROY:
		if (currentWindowStatus == STS_REGISTERWINDOW)
			PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}