#include "MainWndProc.h"

BOOL CALLBACK JoinRoomDlgProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hWnd, JOINROOM_ROOMNUMBER_EDIT), EM_SETCUEBANNER, 0, (LPARAM)L"방번호");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_JOINROOM_CONFIRM:
		{
			std::memset(input_buffer, 0, MAX_ROOM_NAME_LENGTH);
			GetDlgItemText(hWnd, JOINROOM_ROOMNUMBER_EDIT, input_buffer, MAX_ROOM_NAME_LENGTH);
			char buffer[400];
			TcharToChar(buffer, input_buffer);
			bool is_number = true;
			for (int i = 0; i < std::strlen(buffer); i++)
			{
				if (buffer[i] < -1 || buffer[i] > 255 || !std::isdigit(buffer[i]))
				{
					is_number = false;
					break;
				}
			}
			if (!is_number)
			{
				MessageBox(hWnd, TEXT("방 번호를 숫자로 입력해주세요."), TEXT("안내"),
					MB_OK);
				return TRUE;
			}
			EndDialog(hWnd, ID_JOINROOM_CONFIRM);
			return TRUE;
		}
		case ID_JOINROOM_CANCEL:
			EndDialog(hWnd, ID_JOINROOM_CANCEL);
			return TRUE;
		}
		return TRUE;
	case WM_DESTROY:
		EndDialog(hWnd, NULL);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK RoomCreateDlgProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
	{
		SendMessage(GetDlgItem(hWnd, ROOMCREATE_ROOMNAME_EDIT), EM_SETCUEBANNER, 0, (LPARAM)L"방이름");
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_ROOMCREATE_CONFIRM:
			std::memset(input_buffer, 0, MAX_ROOM_NAME_LENGTH);
			GetDlgItemText(hWnd, ROOMCREATE_ROOMNAME_EDIT, input_buffer, MAX_ROOM_NAME_LENGTH);
			EndDialog(hWnd, ID_ROOMCREATE_CONFIRM);
			return TRUE;
		case ID_ROOMCREATE_CANCEL:
			EndDialog(hWnd, ID_ROOMCREATE_CANCEL);
			return TRUE;
		}
		return TRUE;
	case WM_DESTROY:
		EndDialog(hWnd, NULL);
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	const static int buttonWidth = 300, buttonHeight = 50;
	const static int firstButtonMarginHeight = 150;
	static int buttonMarginWidth;
	const static int buttonMarginHeight = 50;

	static HWND hWndCreateRoom, hWndJoinRoom;

	enum { HM_CREATEROOM = 1, HM_JOINROOM };
	switch (iMessage)
	{
	case WM_CREATE:
	{
		int tempHeight = 0;
		GetClientRect(hWnd, &windowRect);
		buttonMarginWidth = (windowRect.right - buttonWidth) / 2;

		hWndCreateRoom = CreateWindow(TEXT("button"), TEXT("방만들기"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			buttonMarginWidth, firstButtonMarginHeight, buttonWidth, buttonHeight,
			hWnd, (HMENU)HM_CREATEROOM, g_hInst, NULL);

		tempHeight += firstButtonMarginHeight + buttonHeight + buttonMarginHeight;
		hWndJoinRoom = CreateWindow(TEXT("button"), TEXT("방입장"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			buttonMarginWidth, tempHeight, buttonWidth, buttonHeight,
			hWnd, (HMENU)HM_JOINROOM, g_hInst, NULL);

		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case HM_CREATEROOM:
		{
			auto dlgResult = DialogBox(g_hInst, MAKEINTRESOURCE(ROOMCREATE_DIALOG), hWnd, RoomCreateDlgProc);
			switch (dlgResult)
			{
			case ID_ROOMCREATE_CONFIRM:
				int result;
				CallLoadingWndProc(hWnd, TEXT("로딩중"),
					[&result]()
					{
						auto len = lstrlenW(user_id);
						char* user_id_buf = new char[(len * 2) + 1];
						TcharToChar(user_id_buf, user_id);
						char roomname_buf[MAX_ROOM_NAME_LENGTH];
						TcharToChar(roomname_buf, input_buffer);
						result = mproc->request_create_room(user_id_buf, roomname_buf);

						if (result)
						{
							std::memset(room_name, 0, sizeof(room_name));
							CharToTchar(room_name, sizeof(room_name), roomname_buf,
								std::strlen(roomname_buf));
						}

						delete[] user_id_buf;
					});

				if (result)
				{
					CreateWindow(lpszTalkClass, room_name, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
						NULL, (HMENU)NULL, g_hInst, NULL);
					currentWindowStatus = STS_TALKWINDOW;
					DestroyWindow(hWnd);
				}
				else
				{
					TCHAR str[256];
					wsprintf(str, TEXT("방을 생성할 수 없습니다"));
					MessageBox(hWnd, str,
						TEXT("안내"), MB_OK);
				}
				break;
			case ID_ROOMCREATE_CANCEL:

				break;
			}
			break;
		}
		case HM_JOINROOM:
			auto dlgResult = DialogBox(g_hInst, MAKEINTRESOURCE(JOINROOM_DIALOG), hWnd, JoinRoomDlgProc);
			switch (dlgResult)
			{
			case ID_JOINROOM_CONFIRM:
				int result;
				CallLoadingWndProc(hWnd, TEXT("로딩중"),
					[&result]()
					{
						auto len = lstrlenW(user_id);
						char* user_id_buf = new char[(len * 2) + 1];
						TcharToChar(user_id_buf, user_id);
						char input_buf[MAX_ROOM_NAME_LENGTH];
						TcharToChar(input_buf, input_buffer);
						long long room_number = std::atoll(input_buf);
						result = mproc->request_join_room(user_id_buf, room_number, room_name);

						delete[] user_id_buf;
					});

				//When it succeeded to join the room
				if (result)
				{
					CreateWindow(lpszTalkClass, room_name, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
						NULL, (HMENU)NULL, g_hInst, NULL);
					currentWindowStatus = STS_TALKWINDOW;
					DestroyWindow(hWnd);
				}
				else
				{
					TCHAR str[256];
					wsprintf(str, TEXT("방에 입장할 수 없습니다"));
					MessageBox(hWnd, str,
						TEXT("안내"), MB_OK);
				}
				break;

			case ID_JOINROOM_CANCEL:

				break;
			}
			break;
		}
		return 0;
	case WM_DRAWITEM:
	{
		if (lParam == NULL) return 0;
		LPDRAWITEMSTRUCT draw_struct = (LPDRAWITEMSTRUCT)lParam;
		if (draw_struct->hwndItem == hWndCreateRoom)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		else if (draw_struct->hwndItem == hWndJoinRoom)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		return 0;
	}
	case WM_DESTROY:
	{
		if (currentWindowStatus == STS_MAINWINDOW)
			PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}