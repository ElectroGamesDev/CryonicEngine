#include "WindowsHelper.h"
#include "RaylibWrapper.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void WindowsHelper::HideWindow()
{
	HWND hwnd = reinterpret_cast<HWND>(RaylibWrapper::GetWindowHandle());
	ShowWindow(hwnd, SW_HIDE);
}