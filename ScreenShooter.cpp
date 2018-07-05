#include "ScreenShooter.h"



ScreenShooter::ScreenShooter():
	desktopWindow (GetDesktopWindow()),
	imageCLSID ()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	GetEncoderClsid(L"image/bmp", &this->imageCLSID);
}

bool ScreenShooter::TakeScreenshotImage() const
{
	const auto screenDC = GetDC(nullptr);
	const auto windowDC = GetDC(this->desktopWindow);

	const auto memDC = CreateCompatibleDC(windowDC);

	if (!memDC)
	{
		MessageBox(this->desktopWindow, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
		return false;
	}

	RECT rcClient;
	GetClientRect(this->desktopWindow, &rcClient);

	SetStretchBltMode(windowDC, HALFTONE);

	if (!StretchBlt(windowDC, 0, 0, rcClient.right, rcClient.bottom, screenDC, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SRCCOPY))
	{
		return false;
	}

	const auto screenBitmap = CreateCompatibleBitmap(windowDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

	if (!screenBitmap)
	{
		return false;
	}

	SelectObject(memDC, screenBitmap);

	if (!BitBlt(memDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, windowDC, 0, 0, SRCCOPY))
	{
		return false;
	}

	std::wstring fileName = L"screen";
	fileName += L"_";
	fileName += this->CurrentDateTime();
	fileName += L".bmp";
	this->SaveScreenShot(screenBitmap, fileName.c_str(), 100);

	DeleteObject(screenBitmap);
	DeleteObject(memDC);
	ReleaseDC(nullptr, screenDC);
	ReleaseDC(this->desktopWindow, windowDC);

	return true;
}

void ScreenShooter::Run()
{
	SetTimer(nullptr, 0, 1000, Timer);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void ScreenShooter::SaveScreenShot(HBITMAP bmp, LPCWSTR filename, ULONG quality) const
{
	Gdiplus::GpBitmap* bitmap;
	Gdiplus::DllExports::GdipCreateBitmapFromHBITMAP(bmp, nullptr, &bitmap);
	//CLSID imageCLSID;

	//GetEncoderClsid(L"image/bmp", &imageCLSID);
	Gdiplus::EncoderParameters encoderParams;
	encoderParams.Count = 1;
	encoderParams.Parameter[0].NumberOfValues = 1;
	encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
	encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	encoderParams.Parameter[0].Value = &quality;

	Gdiplus::DllExports::GdipSaveImageToFile(bitmap, filename, &imageCLSID, &encoderParams);
}

std::wstring ScreenShooter::CurrentDateTime() const
{
	time_t now = time(nullptr);
	tm tstruct;
	wchar_t buf[80];
	localtime_s(&tstruct, &now);
	wcsftime(buf, sizeof(buf), L"%m-%d-%Y_%H-%M-%S", &tstruct);

	return buf;
}

int ScreenShooter::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) const
{
	UINT num = 0;
	UINT size = 0;

	Gdiplus::GetImageEncodersSize(&num,
		&size);
	if (size == 0)
	{
		return -1;
	}
	const auto pImageCodecInfo = new Gdiplus::ImageCodecInfo[size];
	if (pImageCodecInfo == nullptr)
	{
		return -1;
	}
	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType,
			format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			delete[] pImageCodecInfo;
			return j;
		}
	}

	delete[] pImageCodecInfo;
	return -1;
}

void CALLBACK ScreenShooter::Timer(HWND hwnd, UINT msg, UINT_PTR event, DWORD time)
{
	static ScreenShooter sc;
	sc.TakeScreenshotImage();
}