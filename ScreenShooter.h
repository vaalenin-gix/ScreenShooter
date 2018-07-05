#pragma once

#include <windows.h>
#include <string>
#include <gdiplus.h>
#include <ctime>

class ScreenShooter
{
public:
	ScreenShooter();
	virtual ~ScreenShooter() = default;

	bool TakeScreenshotImage() const;

	static void Run();
private:
	HWND desktopWindow;
	CLSID imageCLSID;

	void SaveScreenShot(HBITMAP bmp, LPCWSTR filename, ULONG quality) const;
	std::wstring CurrentDateTime() const;
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) const;

	static void CALLBACK Timer(HWND hwnd, UINT msg, UINT_PTR event, DWORD time);
};

