#ifndef TALKDISPLAYWNDPROC
#define TALKDISPLAYWNDPROC

#include <thread>

#ifndef WINDOWSTATUS
#include <WindowStatus.h>
#endif

#ifndef _WINDOWS_
#include <Windows.h>
#endif

LRESULT CALLBACK TalkDisplayWndProc(HWND, UINT, WPARAM, LPARAM);

static std::thread* message_read_processor = nullptr;
static bool redraw_scroll = false;

const int max_content_width = 15;

class TalkContent
{
typedef unsigned char tc_flag;

private:
	std::string id_;
	std::string name_;
	tc_flag flag_;
	std::string content_;

	const int textHborder = 10, textVborder = 10;
	const int boxHborder = 10;
	const int HeightForLine = 25;
	const double WidthForChar = 6.0f;
	const int scrollWidth = 20;
public:
	TalkContent(std::string id, std::string name, tc_flag flag, std::string content);

	const std::string id();
	const std::string id() const;
	const std::string name() const;
	const std::string name();
	void set_name(const std::string& _name);
	const tc_flag flag();
	const tc_flag flag() const;
	const std::string content() const;
	const std::string content();
	TCHAR* get_boxed_content();

	RECT PaintTalkContent(const HWND& hWnd, const HDC& hdc, RECT& rt, bool is_left_order);

	TalkContent& operator=(const TalkContent& tk);
};

#endif