#include "Win32APIWrapper.h"

int CharToTchar(TCHAR* dest, std::size_t dest_size, const char* source, std::size_t source_size)
{
	int len = MultiByteToWideChar(CP_ACP, 0, source, -1, NULL, 0);
	return MultiByteToWideChar(CP_ACP, 0, source, -1, dest, len);
}

int TcharToChar(char* dest, const TCHAR* source)
{
	int len = WideCharToMultiByte(CP_ACP, 0, source, -1, NULL, 0, NULL, NULL);
	return WideCharToMultiByte(CP_ACP, 0, source, -1, dest, len, NULL, NULL);
}

void PaintReactedToClickButton(LPDRAWITEMSTRUCT lpdrstr, COLORREF border, COLORREF background, COLORREF sel_background)
{
	HDC hdc = lpdrstr->hDC;
	RECT rt = lpdrstr->rcItem;

	HPEN h_border_pen = CreatePen(PS_SOLID, 1, border);
	if (lpdrstr->itemState & ODS_SELECTED)
	{
		HBRUSH h_bk_brush = CreateSolidBrush(sel_background);
		FillRect(hdc, &rt, h_bk_brush);
		DeleteObject(h_bk_brush);

		HGDIOBJ h_old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
		HGDIOBJ h_old_pen = SelectObject(hdc, h_border_pen);

		Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
		SelectObject(hdc, h_old_brush);
		SelectObject(hdc, h_old_pen);
		DeleteObject(h_border_pen);
	}
	else
	{
		HBRUSH h_brush = CreateSolidBrush(background);
		FillRect(hdc, &rt, h_brush);
		DeleteObject(h_brush);

		HGDIOBJ h_old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
		HGDIOBJ h_old_pen = SelectObject(hdc, h_border_pen);

		Rectangle(hdc, rt.left, rt.top, rt.right, rt.bottom);
		SelectObject(hdc, h_old_brush);
		SelectObject(hdc, h_old_pen);
		DeleteObject(h_border_pen);
	}
}

void DrawTextCustomButton(LPDRAWITEMSTRUCT lpdrstr, COLORREF textcolor, HFONT* font)
{
	HDC hdc = lpdrstr->hDC;
	RECT rt = lpdrstr->rcItem;
	HGDIOBJ h_old_font;

	if (font != NULL)
	{
		h_old_font = SelectObject(hdc, font);
		TCHAR wtext_buffer[1024];
		int len = GetWindowText(lpdrstr->hwndItem, wtext_buffer, 1024);
		int old_bk_mode = SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, textcolor);
		DrawText(hdc, wtext_buffer, len, &rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(hdc, old_bk_mode);
		SelectObject(hdc, h_old_font);
	}
	else
	{
		TCHAR wtext_buffer[1024];
		int len = GetWindowText(lpdrstr->hwndItem, wtext_buffer, 1024);
		int old_bk_mode = SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, textcolor);
		DrawText(hdc, wtext_buffer, len, &rt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SetBkMode(hdc, old_bk_mode);
	}
}