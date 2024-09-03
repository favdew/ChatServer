#include "TalkWndProc.h"

void TerminateMessageReader()
{
	if (message_reader == nullptr) return;
	TerminateThread(message_reader->native_handle(), 1);
	if (message_reader->joinable())
		message_reader->join();
	delete message_reader;
	message_reader = nullptr;
}

char* ReadTalkMessage()
{	
	return mproc->read_chatting_message();
}

void SendTalkMessage(HWND hWndParent, HWND hWndTalkDisplay, HWND hWndTextBox)
{
	int size = 0, length;
	TCHAR* str = new TCHAR[1];
	do
	{
		size += 1024;
		delete[] str;
		str = new TCHAR[size];
		length = GetWindowText(hWndTextBox, str, size);
	} while (length == size);

	bool is_blank = true;
	std::size_t str_length = lstrlenW(str);
	for (int i = 0; i < str_length; i++)
	{
		if (str[i] != ' ' && str[i] != '\r' && str[i] != '\n')
		{
			is_blank = false;
			break;
		}
	}

	if (is_blank) return;
	if (length <= 0)
	{
		;
	}
	else if (mproc != nullptr)
	{
		char uid[MAX_ID_LENGTH];
		char uname[MAX_NICKNAME_LENGTH];
		char* content = new char[size];
		TcharToChar(uid, user_id);
		TcharToChar(uname, user_name);
		TcharToChar(content, str);
		talk_data td(uid, uname, talk_data::TDF_MESSAGE, content);
		delete[] content;
		mproc->send_talk_message(td);
	}
	
	delete[] str;
	SetWindowText(hWndTextBox, NULL);
}

void AddDisplayContent(const TCHAR* str)
{
	SendMessage(hWndTalkDisplay, TM_ADDCONTENT, (WPARAM)str, (LPARAM)hWndTalkProc);
}

LRESULT CALLBACK TalkMessageInputBoxProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
	case WM_CHAR:
		switch (wParam)
		{
		case 0x0D:
			if (GetKeyState(VK_SHIFT) & 0x8000)
				CallWindowProc(originalEditProc, hWnd, (UINT)WM_CHAR, (WPARAM)VK_RETURN, lParam);
			else
				SendTalkMessage(hWnd, hWndTalkDisplay, hWndTextBox);
			return 0;
		}
		break;
	}
	return CallWindowProc(originalEditProc, hWnd, iMessage, wParam, lParam);
}

LRESULT CALLBACK TalkWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CREATE:
	{
		int tempHeight = 0;
		GetClientRect(hWnd, &windowRect);
		talkDisplayMarginWidth = (windowRect.right - talkDisplayWidth) / 2;
		textBoxMarginWidth = talkDisplayMarginWidth;

		hWndTalkProc = hWnd;

		hWndExitBtn = CreateWindow(TEXT("button"), TEXT("←"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			exitButtonMarginWidth, tempHeight + exitButtonMarginHeight, exitButtonWidth, exitButtonHeight,
			hWnd, (HMENU)HM_EXITBTN, g_hInst, NULL);

		hWndTalkDisplay = CreateWindow(TEXT("talkdisplay"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
			talkDisplayMarginWidth, talkDisplayMarginHeight, talkDisplayWidth, talkDisplayHeight,
			hWnd, (HMENU)HM_CONTENTSTATIC, g_hInst, NULL);

		tempHeight += talkDisplayMarginHeight + talkDisplayHeight + textBoxMarginHeight;
		hWndTextBox = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE,
			textBoxMarginWidth, tempHeight, textBoxWidth, textBoxHeight,
			hWnd, (HMENU)HM_TEXTBOX, g_hInst, NULL);
		originalEditProc = (WNDPROC)SetWindowLong(hWndTextBox, GWL_WNDPROC, (LONG)TalkMessageInputBoxProc);

		hWndSendBtn = CreateWindow(TEXT("button"), TEXT("보내기"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
			textBoxMarginWidth + textBoxWidth + sendButtonMarginWidth, tempHeight, sendButtonWidth, sendButtonHeight,
			hWnd, (HMENU)HM_SENDBTN, g_hInst, NULL);

		if (message_reader == nullptr)
		{
			message_reader = new std::thread(
				[&hWnd]()
				{
					while (true)
					{
						auto msg = ReadTalkMessage();
						if (msg != nullptr)
						{
							cm_send_room_message message(msg);
							std::size_t buf_size = (message.td.content().size() * 2) + 1;
							TCHAR* str = new TCHAR[buf_size];
							CharToTchar(str, buf_size, message.td.content().c_str(),
								message.td.content().size());
							DestroyWindow(hWnd);
							SendMessage(hWndTalkDisplay, TM_ADDCONTENT, (WPARAM)(&message.td), (LPARAM)hWnd);
							delete[] str;
							delete[] msg;
						}
					}
				});
		}

		return 0;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case HM_EXITBTN:
			TerminateMessageReader();
			if (mproc != nullptr)
			{
				char uid[MAX_ID_LENGTH];
				char roomname[MAX_ROOM_NAME_LENGTH];

				TcharToChar(uid, user_id);
				TcharToChar(roomname, room_name);
				int result = mproc->request_exit_room(uid, roomname);
				//When it exited a room successfully
				if (result)
				{
					std::memset(room_name, 0, sizeof(room_name));
				}
				else
				{
					MessageBox(hWnd, TEXT("에러가 발생하였습니다."), TEXT("안내"), MB_OK);
					DestroyWindow(hWnd);
				}
			}
			CreateWindow(lpszMainClass, lpszLoginClass, WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
				CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
				NULL, (HMENU)NULL, g_hInst, NULL);
			currentWindowStatus = STS_MAINWINDOW;
			DestroyWindow(hWnd);
			break;
		case HM_SENDBTN:
			SendTalkMessage(hWnd, hWndTalkDisplay, hWndTextBox);
			break;
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
		else if (draw_struct->hwndItem == hWndSendBtn)
		{
			PaintReactedToClickButton(draw_struct, button_border_color, button_color, on_click_button_color);
			DrawTextCustomButton(draw_struct, button_text_color, NULL);
		}
		return 0;
	}
	case WM_DESTROY:
		TerminateMessageReader();
		if (lstrlenW(room_name) != 0)
		{
			char uid[MAX_ID_LENGTH];
			char roomname[MAX_ROOM_NAME_LENGTH];
			TcharToChar(uid, user_id);
			TcharToChar(roomname, room_name);
			mproc->request_exit_room(uid, roomname);
		}

		if (currentWindowStatus == STS_TALKWINDOW)
			PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}