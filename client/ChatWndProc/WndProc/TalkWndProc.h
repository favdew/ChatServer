#ifndef TALKWNDPROC
#define TALkWNDPROC

#ifndef WINDOWSTATUS
#include <WindowStatus.h>
#endif

#ifndef _WINDOWS_
#include <Windows.h>
#endif

#include <thread>

const int exitButtonWidth = 30, exitButtonHeight = 30;
const int exitButtonMarginWidth = 0, exitButtonMarginHeight = 0;

const int talkDisplayWidth = 350, talkDisplayHeight = 430;
static int talkDisplayMarginWidth;
const int talkDisplayMarginHeight = 30;

const int textBoxWidth = 230, textBoxHeight = 100;
static int textBoxMarginWidth;
const int textBoxMarginHeight = 20;

const int sendButtonWidth = 100, sendButtonHeight = 100;
const int sendButtonMarginWidth = 20, sendButtonMarginHeight = 20;

static HWND hWndTalkProc;
static HWND hWndExitBtn, hWndTalkDisplay, hWndTextBox, hWndSendBtn;

static WNDPROC originalEditProc;

enum { HM_EXITBTN = 1, HM_CONTENTSTATIC, HM_TEXTBOX, HM_SENDBTN };

static std::thread* message_reader = nullptr;

void TerminateMessageReader();
char* ReadTalkMessage();
void SendTalkMessage(HWND, HWND, HWND);
void AddDisplayContent(const TCHAR* str);
LRESULT CALLBACK TalkMessageInputBoxProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TalkWndProc(HWND, UINT, WPARAM, LPARAM);

#endif