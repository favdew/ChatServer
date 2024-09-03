#include "TalkDisplayWndProc.h"

typedef unsigned char tc_flag;

LRESULT CALLBACK TalkDisplayWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static HWND hScroll;
	PAINTSTRUCT ps;
	HDC hdc;
	static RECT textrect;	//Width is max string length + 
							//(textVborder * 2)
	static HBRUSH MyBrush, OldBrush;

	static std::list<TalkContent> list_;

	const int textVborder = 10;
	const int HeightForLine = 25;
	const double WidthForChar = 8.5f;
	const int boxVborder = 10;
	static int scroll_value;
	static int max_scroll_value;
	static int move_per_scroll = 50;
	static int scrollWidth = 20;


	switch (iMessage) {
	case WM_CREATE:
	{
		RECT rt;
		GetClientRect(hWnd, &rt);
		hScroll = CreateWindow(TEXT("scrollbar"), NULL, WS_CHILD | WS_VISIBLE | SBS_VERT, rt.right - 20, rt.top, scrollWidth, rt.bottom - rt.top, hWnd, (HMENU)0, g_hInst, NULL);
		max_scroll_value = 0;
		SetScrollRange(hScroll, SB_CTL, 0, max_scroll_value, TRUE);
		scroll_value = 0;
		SetScrollPos(hScroll, SB_CTL, scroll_value, TRUE);

		return 0;
	}
	case WM_VSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_THUMBTRACK:
			scroll_value = HIWORD(wParam);
			break;
		}
		SetScrollPos((HWND)lParam, SB_CTL, scroll_value, TRUE);
		if (redraw_scroll) redraw_scroll = false;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	case WM_MOUSEWHEEL:
	{
		short input = (short)((short)HIWORD(wParam) / 120);

		//When wheel is up
		if (input > 0)
			scroll_value = std::max<short>(0, scroll_value - (input * move_per_scroll));
		//When wheel is down
		else
			scroll_value = std::min<short>(max_scroll_value, scroll_value - (input * move_per_scroll));

		SendMessage(hWnd, WM_VSCROLL, (WPARAM)NULL, (LPARAM)hScroll);
		return 0;
	}
	case TM_ADDCONTENT:
	{
		talk_data* td = (talk_data*)wParam;
		list_.push_back(TalkContent(td->id(), td->nickname(), td->flag(),
			td->content()));
		redraw_scroll = true;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	case WM_PAINT:
	{
		RECT rt = { 0, boxVborder, 0, 0 }, client_rt;

		hdc = BeginPaint(hWnd, &ps);
		rt.top -= scroll_value;
		GetClientRect(hWnd, &client_rt);
		int height = 0;
		int top = rt.top;
		char uid[MAX_ID_LENGTH];
		TcharToChar(uid, user_id);
		std::string uid_buf = uid;
		uid_buf.insert(0, "nick");
		for (auto it = list_.begin(); it != list_.end(); it++)
		{
			rt = (*it).PaintTalkContent(hWnd, hdc, rt,
				(*it).name() != uid_buf.c_str());
			height = rt.bottom - top;
			rt.top = rt.bottom + boxVborder;
		}

		EndPaint(hWnd, &ps);

		height += boxVborder + boxVborder;

		//Scroll bar Range Process
		if (height > client_rt.bottom - client_rt.top)
		{
			max_scroll_value = (height - (client_rt.bottom - client_rt.top));
			if (max_scroll_value > MAXLONG) max_scroll_value = MAXLONG;
			SetScrollRange(hScroll, SB_CTL, 0, max_scroll_value, true);
			
			if (redraw_scroll)
			{
				scroll_value = max_scroll_value;
				SendMessage(hWnd, WM_VSCROLL, (WPARAM)NULL, (LPARAM)hScroll);
			}
		}
		else
		{
			max_scroll_value = 0;
			SetScrollRange(hScroll, SB_CTL, 0, 0, true);
		}
		return 0;
	}
	case WM_DESTROY:
	{
		if (message_read_processor != nullptr)
		{
			TerminateThread(message_read_processor->native_handle(), 1);
			if (message_read_processor->joinable())
				message_read_processor->join();
			delete message_read_processor;
			message_read_processor = nullptr;
		}
		list_.clear();
		return 0;
	}
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

TalkContent::TalkContent(std::string id, std::string name, tc_flag flag, std::string content)
	: id_(id), name_(name), flag_(flag), content_(content)
{

}

const std::string TalkContent::id() { return id_; }
const std::string TalkContent::id() const { return id_; }
const std::string TalkContent::name() const { return this->name_; }
const std::string TalkContent::name() { return this->name_; }
void TalkContent::set_name(const std::string& _name) { name_ = _name; }
const tc_flag TalkContent::flag() { return flag_; }
const tc_flag TalkContent::flag() const { return flag_; }
const std::string TalkContent::content() const { return this->content_; }
const std::string TalkContent::content() { return this->content_; }

TCHAR* TalkContent::get_boxed_content()
{
	TCHAR* ret;
	int char_count = 0;

	int pt = -1, ret_pt = 0;
	TCHAR* content_buffer = new TCHAR[content_.size() + 1];
	CharToTchar(content_buffer, 0, content_.c_str(), 0);
	std::size_t ret_length =
		lstrlenW(content_buffer) + (lstrlenW(content_buffer) / max_content_width * 2) + 1;
	ret = new TCHAR[ret_length];
	std::memset(ret, 0, ret_length * sizeof(TCHAR));
	for (int i = 0; i < lstrlenW(content_buffer); i++)
	{
		char_count++;
		if (pt == -1) pt = i;
		if (content_buffer[i] == '\r')
		{
			std::memcpy(ret + ret_pt, content_buffer + pt, char_count * sizeof(TCHAR));
			ret_pt += char_count;
			char_count = 0;
			ret[ret_pt] = '\n';
			ret_pt += 1;
			i++;
			pt = -1;
		}
		else if (char_count >= max_content_width)
		{
			char_count = 0;
			std::memcpy(ret + ret_pt, content_buffer + pt , max_content_width * sizeof(TCHAR));
			
			ret_pt += max_content_width;
			ret[ret_pt] = '\r';
			ret[ret_pt + 1] = '\n';
			ret_pt += 2;
			pt = -1;
		}
	}

	if (char_count != 0)
	{
		std::memcpy(ret + ret_pt, content_buffer + pt, char_count * sizeof(TCHAR));
	}

	delete[] content_buffer;
	return ret;
}

RECT TalkContent::PaintTalkContent(const HWND& hWnd, const HDC& hdc, RECT& rt, bool is_left_order)
{
	RECT static_rt, textrect = { 0, 0, 0, 0 };
	HBRUSH MyBrush, OldBrush;
	GetClientRect(hWnd, &static_rt);

	MyBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	OldBrush = (HBRUSH)SelectObject(hdc, MyBrush);
	SetBkMode(hdc, TRANSPARENT);

	TCHAR* str;

	switch (flag_)
	{
	case talk_data::TDF_CREATE:
	{
		TCHAR output_str[1024];
		long long room_number = std::atoll(content_.c_str());
		wsprintf(output_str, TEXT("방 생성 완료. 방번호는 %d 입니다."),
			room_number);

		RECT output_rt = { 0, 0, 0, 0 };
		DrawText(hdc, output_str, -1, &output_rt, DT_LEFT | DT_CALCRECT);
		rt.top += textVborder;
		rt.left = ((static_rt.left + static_rt.right - scrollWidth) / 2) -
			(output_rt.right / 2);
		rt.right = rt.left + output_rt.right;
		rt.bottom = rt.top + output_rt.bottom;
		DrawText(hdc, output_str, -1, &rt, DT_LEFT | DT_WORDBREAK);

		rt.left -= textHborder;
		rt.right += textHborder;
		rt.top -= textVborder;
		rt.bottom += textVborder;
		SelectObject(hdc, OldBrush);
		DeleteObject(MyBrush);
		return rt;
	}
	case talk_data::TDF_JOIN:
	{
		TCHAR output_str[1024];
		char uid[1024];
		TcharToChar(uid, user_id);
		if (id_ != uid)
		{
			TCHAR output_id_buf[1024];
			CharToTchar(output_id_buf, 1024, id_.c_str(), id_.size());
			wsprintf(output_str, TEXT("%s 님이 입장하였습니다."), output_id_buf);
		}
		else
		{
			wsprintf(output_str, TEXT("방에 입장하였습니다"));
		}
		RECT output_rt = { 0, 0, 0, 0 };
		DrawText(hdc, output_str, -1, &output_rt, DT_LEFT | DT_CALCRECT);
		rt.top += textVborder;
		rt.left = ((static_rt.left + static_rt.right - scrollWidth) / 2) -
			(output_rt.right / 2);
		rt.right = rt.left + output_rt.right;
		rt.bottom = rt.top + output_rt.bottom;
		DrawText(hdc, output_str, -1, &rt, DT_LEFT | DT_WORDBREAK);

		rt.left -= textHborder;
		rt.right += textHborder;
		rt.top -= textVborder;
		rt.bottom += textVborder;
		SelectObject(hdc, OldBrush);
		DeleteObject(MyBrush);
		return rt;
	}
	case talk_data::TDF_EXIT:
	{
		TCHAR output_str[1024];
		TCHAR output_id_buf[1024];
		CharToTchar(output_id_buf, 1024, id_.c_str(), id_.size());
		wsprintf(output_str, TEXT("%s 님이 퇴장하였습니다."), output_id_buf);
		RECT output_rt = { 0, 0, 0, 0 };
		DrawText(hdc, output_str, -1, &output_rt, DT_LEFT | DT_CALCRECT);
		rt.top += textVborder;
		rt.left = ((static_rt.left + static_rt.right - scrollWidth) / 2) -
			(output_rt.right / 2);
		rt.right = rt.left + output_rt.right;
		rt.bottom = rt.top + output_rt.bottom;
		DrawText(hdc, output_str, -1, &rt, DT_LEFT | DT_WORDBREAK);

		rt.left -= textHborder;
		rt.right += textHborder;
		rt.top -= textVborder;
		rt.bottom += textVborder;
		SelectObject(hdc, OldBrush);
		DeleteObject(MyBrush);

		return rt;
	}
	case talk_data::TDF_MESSAGE:
	{
		//Left order
		if (is_left_order)
			rt.left = static_rt.left + boxHborder;

		if (is_left_order)
		{
			str = new TCHAR[std::strlen(name_.c_str()) + 1];
			CharToTchar(str, std::strlen(name_.c_str()) + 1, name_.c_str(), std::strlen(name_.c_str()) + 1);
			DrawText(hdc, str, -1, &textrect, DT_LEFT | DT_CALCRECT);
			rt.top = rt.top + textVborder;
			rt.left = rt.left + textHborder;
			rt.right = rt.left + textrect.right;
			rt.bottom = rt.top + textrect.bottom;
			DrawText(hdc, str, -1, &rt, DT_LEFT | DT_WORDBREAK);
			delete[] str;
			rt.top += (textVborder / 2) + textrect.bottom;
		}

		auto str = get_boxed_content();
		DrawText(hdc, str, -1, &textrect, DT_LEFT | DT_CALCRECT);
		//Right order
		if (!is_left_order)
		{
			rt.left = static_rt.right - (boxHborder + textHborder +
				textrect.right + scrollWidth);
		}
		else
		{
			rt.left = static_rt.left + (boxHborder + textHborder);
		}
		rt.top = rt.top + textVborder;
		rt.right = rt.left + textrect.right;
		rt.bottom = rt.top + textrect.bottom;
		DrawText(hdc, str, -1, &rt, DT_LEFT | DT_WORDBREAK);
		rt.left -= textHborder; rt.right += textHborder; rt.top -= textVborder; rt.bottom += textVborder;
		delete[] str;

		Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
		SelectObject(hdc, OldBrush);
		DeleteObject(MyBrush);

		return rt;
	}
	default:
		return rt;
	}
}

TalkContent& TalkContent::operator=(const TalkContent& tk)
{
	this->name_ = tk.name();
	this->content_ = tk.content();
	return *this;
}