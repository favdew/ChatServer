#ifndef WIN32WRAPPER
#define	WIN32WRAPPER

#include <Windows.h>
#include <cstring>

int CharToTchar(TCHAR* dest, std::size_t dest_size, const char* source, std::size_t source_size);
int TcharToChar(char* dest, const TCHAR* source);

void PaintReactedToClickButton(LPDRAWITEMSTRUCT lpdrstr, COLORREF border, COLORREF background, COLORREF sel_background);
void DrawTextCustomButton(LPDRAWITEMSTRUCT lpdrstr, COLORREF textcolor, HFONT* font);

#endif