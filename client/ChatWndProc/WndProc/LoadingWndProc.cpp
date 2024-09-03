#include "LoadingWndProc.h"

BOOL CALLBACK LoadingWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		is_terminate = false;
		SetDlgItemText(hWnd, IDC_LOADINGSTATIC, loadingMessage);
		SetTimer(hWnd, TM_TERMINATE, 50, NULL);
		if (order_processor == nullptr)
		{
			order_processor = new std::thread([&hWnd]()
				{
					ldproc();
					is_terminate = true;
				});
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_ROOMCREATE_CANCEL:
			EndDialog(hWnd, ID_ROOMCREATE_CANCEL);
			break;
		}
		return TRUE;
	case WM_TIMER:
		switch (wParam)
		{
		case TM_TERMINATE:
		{
			if (is_terminate)
				EndDialog(hWnd, WM_TIMER);
			break;
		}
		}
		return TRUE;
	case WM_DESTROY:
		TerminateThread(order_processor->native_handle(), 1);
		order_processor->join();
		delete order_processor;
		order_processor = nullptr;
		return TRUE;
	}

	return FALSE;
}

int CallLoadingWndProc(HWND hWnd, LoadingMessage message, LoadingProcessor proc)
{
	ldproc = proc;
	loadingMessage = message;
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, LoadingWndProc);
}