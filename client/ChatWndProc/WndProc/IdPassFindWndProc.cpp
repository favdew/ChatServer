#include "IdPassFindWndProc.h"

LRESULT CALLBACK IdPassFindWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	const static int exitButtonWidth = 30, exitButtonHeight = 30;
	const static int exitButtonMarginWidth = 0, exitButtonMarginHeight = 0;

	const static int editWidth = 300, editHeight = 30;
	static int editMarginWidth;
	const static int editMarginHeight = 50;
	const static int firstEditMarginHeight = 100;

	const static int confirmButtonWidth = 300, confirmButtonHeight = 30;
	static int confirmButtonMarginWidth;
	const static int confirmButtonMarginHeight = 50;

	static HWND hWndExitBtn, hWndIdEdit, hWndNameEdit, hWndEmailEdit,
		hWndConfirmBtn;

	enum { HM_EXITBTN = 1, HM_IDEDIT, HM_CONFIRMBTN };

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
		hWndIdEdit = CreateWindow(lpszCustomColoredEditClass, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
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

		tempHeight += editHeight + confirmButtonMarginHeight;
		hWndConfirmBtn = CreateWindow(TEXT("button"), TEXT("확인"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			confirmButtonMarginWidth, tempHeight, confirmButtonWidth, confirmButtonHeight,
			hWnd, (HMENU)HM_CONFIRMBTN, g_hInst, NULL);

		SendMessage(hWndIdEdit, EM_SETCUEBANNER, 0, (LPARAM)L"아이디");
		SendMessage(hWndNameEdit, EM_SETCUEBANNER, 0, (LPARAM)L"이름");
		SendMessage(hWndEmailEdit, EM_SETCUEBANNER, 0, (LPARAM)L"email");
		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case HM_EXITBTN:
			CreateWindow(lpszLoginClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
				CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
				NULL, (HMENU)NULL, g_hInst, NULL);
			currentWindowStatus = STS_LOGINWINDOW;
			DestroyWindow(hWnd);
			break;
		case HM_CONFIRMBTN:
		{
			TCHAR input_user_id[MAX_ID_LENGTH], input_name[256], input_email[256];

			GetWindowText(hWndIdEdit, input_user_id, sizeof(input_user_id));
			GetWindowText(hWndNameEdit, input_name, sizeof(input_name));
			GetWindowText(hWndEmailEdit, input_email, sizeof(input_email));

			if (lstrlenW(input_user_id) <= 0 || lstrlenW(input_name) <= 0 ||
				lstrlenW(input_email) <= 0)
			{
				MessageBox(hWnd, TEXT("모든 사항을 기입해야 합니다"), TEXT("안내"), MB_OK);
				return 0;
			}
			else
			{
				tcp::resolver resolver(io_context);
				auto endpoints = resolver.resolve(connect_ip, connect_port);
				message_proc* proc = nullptr;
				int ipfind_result = -1;

				CallLoadingWndProc(hWnd, TEXT("요청중"),
					[&endpoints, &hWnd, &proc, &input_user_id, &input_name,
					&input_email, &ipfind_result]()
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
							char uname[MAX_NAME_LENGTH];
							char uemail[MAX_EMAIL_LENGTH];

							TcharToChar(uid, input_user_id);
							TcharToChar(uname, input_name);
							TcharToChar(uemail, input_email);

							ipfind_result = mproc->request_confirm_uinfo(uid, uname, uemail);
						}
						catch (std::exception e)
						{
							MessageBox(hWnd, TEXT("서버에 연결할 수 없습니다"),
								TEXT("connect error"), MB_OK);
							delete mproc;
							mproc = nullptr;
						}
					});

				if (!ipfind_result)
				{
					CreateWindow(lpszNewPassSetClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
						NULL, (HMENU)NULL, g_hInst, NULL);
					currentWindowStatus = STS_NEWPASSSETWINDOW;
					std::memcpy(input_user_id_buffer, input_user_id, sizeof(input_user_id_buffer));
					DestroyWindow(hWnd);
				}
				else if(ipfind_result && mproc != nullptr)
				{
					MessageBox(hWnd, TEXT("입력한 정보와 일치하는 유저를 찾을 수 없습니다"),
						TEXT("안내"), MB_OK);
					return 0;
				}
			}
			break;
		}
		}
		return 0;
	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT draw_struct = (LPDRAWITEMSTRUCT)(lParam);
		HWND item_id = draw_struct->hwndItem;

		if (item_id == hWndExitBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		else if (item_id == hWndConfirmBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		return 0;
	}
	case WM_DESTROY:
		if (currentWindowStatus == STS_IDPASSFINDWINDOW)
			PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}