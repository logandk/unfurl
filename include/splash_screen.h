/**
 * Splash screen adopted from:
 * http://code.logos.com/blog/2008/09/displaying_a_splash_screen_with_c_introduction.html
 */

#ifndef UNFURL_SPLASH_SCREEN_H
#define UNFURL_SPLASH_SCREEN_H

#include <windows.h>
#include <wincodec.h>
#include <tchar.h>

// Creates a stream object initialized with the data from an executable resource.
IStream *CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
{
  // initialize return value
  IStream *ipStream = NULL;

  // find the resource
  HRSRC hrsrc = FindResource(NULL, lpName, lpType);

  if (hrsrc == NULL)
  {
    return ipStream;
  }

  // load the resource
  DWORD dwResourceSize = SizeofResource(NULL, hrsrc);
  HGLOBAL hglbImage = LoadResource(NULL, hrsrc);

  if (hglbImage == NULL)
  {
    return ipStream;
  }

  // lock the resource, getting a pointer to its data
  LPVOID pvSourceResourceData = LockResource(hglbImage);

  if (pvSourceResourceData == NULL)
  {
    return ipStream;
  }

  // allocate memory to hold the resource data
  HGLOBAL hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);

  if (hgblResourceData == NULL)
  {
    return ipStream;
  }

  // get a pointer to the allocated memory
  LPVOID pvResourceData = GlobalLock(hgblResourceData);

  if (pvResourceData == NULL)
  {
    GlobalFree(hgblResourceData);
    return ipStream;
  }

  // copy the data from the resource to the new memory block
  CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
  GlobalUnlock(hgblResourceData);

  // create a stream on the HGLOBAL containing the data
  if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
  {
    return ipStream;
  }

  // couldn't create stream; free the memory
  GlobalFree(hgblResourceData);

  // no need to unlock or free the resource
  return ipStream;
}

// Loads a PNG image from the specified stream (using Windows Imaging Component).
IWICBitmapSource *LoadBitmapFromStream(IStream *ipImageStream)
{
  // initialize return value
  IWICBitmapSource *ipBitmap = NULL;

  // load WIC's PNG decoder
  IWICBitmapDecoder *ipDecoder = NULL;

  if (FAILED(CoCreateInstance(CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER, __uuidof(ipDecoder), reinterpret_cast<void **>(&ipDecoder))))
  {
    return ipBitmap;
  }

  // load the PNG
  if (FAILED(ipDecoder->Initialize(ipImageStream, WICDecodeMetadataCacheOnLoad)))
  {
    ipDecoder->Release();
    return ipBitmap;
  }

  // check for the presence of the first frame in the bitmap
  UINT nFrameCount = 0;

  if (FAILED(ipDecoder->GetFrameCount(&nFrameCount)) || nFrameCount != 1)
  {
    ipDecoder->Release();
    return ipBitmap;
  }

  // load the first frame (i.e., the image)
  IWICBitmapFrameDecode *ipFrame = NULL;

  if (FAILED(ipDecoder->GetFrame(0, &ipFrame)))
  {
    ipDecoder->Release();
    return ipBitmap;
  }

  // convert the image to 32bpp BGRA format with pre-multiplied alpha
  //   (it may not be stored in that format natively in the PNG resource,
  //   but we need this format to create the DIB to use on-screen)
  WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, ipFrame, &ipBitmap);
  ipFrame->Release();

  ipDecoder->Release();
  return ipBitmap;
}

// Creates a 32-bit DIB from the specified WIC bitmap.
HBITMAP CreateHBITMAP(IWICBitmapSource *ipBitmap)
{
  // initialize return value
  HBITMAP hbmp = NULL;

  // get image attributes and check for valid image
  UINT width = 0;
  UINT height = 0;

  if (FAILED(ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0)
  {
    return hbmp;
  }

  // prepare structure giving bitmap information (negative height indicates a top-down DIB)
  BITMAPINFO bminfo;
  ZeroMemory(&bminfo, sizeof(bminfo));
  bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bminfo.bmiHeader.biWidth = width;
  bminfo.bmiHeader.biHeight = -((LONG) height);
  bminfo.bmiHeader.biPlanes = 1;
  bminfo.bmiHeader.biBitCount = 32;
  bminfo.bmiHeader.biCompression = BI_RGB;

  // create a DIB section that can hold the image
  void *pvImageBits = NULL;
  HDC hdcScreen = GetDC(NULL);
  hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
  ReleaseDC(NULL, hdcScreen);

  if (hbmp == NULL)
  {
    return hbmp;
  }

  // extract the image into the HBITMAP
  const UINT cbStride = width * 4;
  const UINT cbImage = cbStride * height;

  if (FAILED(ipBitmap->CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE *>(pvImageBits))))
  {
    // couldn't extract image; delete HBITMAP
    DeleteObject(hbmp);
    hbmp = NULL;
  }

  return hbmp;
}

// Loads the PNG containing the splash image into a HBITMAP.
HBITMAP LoadSplashImage()
{
  HBITMAP hbmpSplash = NULL;

  // load the PNG image data into a stream
  IStream *ipImageStream = CreateStreamOnResource(_T("SPLASH"), RT_RCDATA);

  if (ipImageStream == NULL)
  {
    return hbmpSplash;
  }

  // load the bitmap with WIC
  IWICBitmapSource *ipBitmap = LoadBitmapFromStream(ipImageStream);

  if (ipBitmap == NULL)
  {
    ipImageStream->Release();
    return hbmpSplash;
  }

  // create a HBITMAP containing the image
  hbmpSplash = CreateHBITMAP(ipBitmap);
  ipBitmap->Release();

  ipImageStream->Release();
  return hbmpSplash;
}

// Registers a window class for the splash and splash owner windows.
void RegisterWindowClass(HINSTANCE g_hInstance)
{
  WNDCLASS wc = { 0 };
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = g_hInstance;
  //wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(_T("SPLASH")));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = _T("SplashWindow");
  RegisterClass(&wc);
}


// Creates the splash owner window and the splash window.
HWND CreateSplashWindow(HINSTANCE g_hInstance)
{
  return CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, _T("SplashWindow"), NULL, WS_POPUP | WS_VISIBLE,
                        0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);
}

// Calls UpdateLayeredWindow to set a bitmap (with alpha) as the content of the splash window.
void SetSplashImage(HWND hwndSplash, HBITMAP hbmpSplash)
{
  // get the size of the bitmap
  BITMAP bm;
  GetObject(hbmpSplash, sizeof(bm), &bm);
  SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

  // get the primary monitor's info
  POINT ptZero = { 0 };
  HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO monitorinfo = { 0 };
  monitorinfo.cbSize = sizeof(monitorinfo);
  GetMonitorInfo(hmonPrimary, &monitorinfo);

  // center the splash screen in the middle of the primary work area
  const RECT &rcWork = monitorinfo.rcWork;
  POINT ptOrigin;
  ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
  ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

  // create a memory DC holding the splash bitmap
  HDC hdcScreen = GetDC(NULL);
  HDC hdcMem = CreateCompatibleDC(hdcScreen);
  HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcMem, hbmpSplash);

  // use the source image's alpha channel for blending
  BLENDFUNCTION blend = { 0 };
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = 235;
  blend.AlphaFormat = AC_SRC_ALPHA;

  // paint the window (in the right location) with the alpha-blended bitmap
  UpdateLayeredWindow(hwndSplash, hdcScreen, &ptOrigin, &sizeSplash,
                      hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

  // delete temporary objects
  SelectObject(hdcMem, hbmpOld);
  DeleteDC(hdcMem);
  ReleaseDC(NULL, hdcScreen);
}

HBITMAP CreateAlphaTextBitmap(LPCSTR inText, HFONT inFont, COLORREF inColour)
{
  int TextLength = (int)strlen(inText);
  if (TextLength <= 0) return NULL;

  // Create DC and select font into it
  HDC hTextDC = CreateCompatibleDC(NULL);
  HFONT hOldFont = (HFONT)SelectObject(hTextDC, inFont);
  HBITMAP hMyDIB = NULL;

  // Get text area
  RECT TextArea = {0, 0, 0, 0};
  DrawText(hTextDC, inText, TextLength, &TextArea, DT_CALCRECT);
  if ((TextArea.right > TextArea.left) && (TextArea.bottom > TextArea.top))
  {
    BITMAPINFOHEADER BMIH;
    memset(&BMIH, 0x0, sizeof(BITMAPINFOHEADER));
    void *pvBits = NULL;

    // Specify DIB setup
    BMIH.biSize = sizeof(BMIH);
    BMIH.biWidth = TextArea.right - TextArea.left;
    BMIH.biHeight = TextArea.bottom - TextArea.top;
    BMIH.biPlanes = 1;
    BMIH.biBitCount = 32;
    BMIH.biCompression = BI_RGB;

    // Create and select DIB into DC
    hMyDIB = CreateDIBSection(hTextDC, (LPBITMAPINFO)&BMIH, 0, (LPVOID*)&pvBits, NULL, 0);
    HBITMAP hOldBMP = (HBITMAP)SelectObject(hTextDC, hMyDIB);
    if (hOldBMP != NULL)
    {
      // Set up DC properties
      SetTextColor(hTextDC, 0x00FFFFFF);
      SetBkColor(hTextDC, 0x00000000);
      SetBkMode(hTextDC, OPAQUE);

      // Draw text to buffer
      DrawText(hTextDC, inText, TextLength, &TextArea, DT_NOCLIP);
      BYTE* DataPtr = (BYTE*)pvBits;
      BYTE FillR = GetRValue(inColour);
      BYTE FillG = GetGValue(inColour);
      BYTE FillB = GetBValue(inColour);
      BYTE ThisA;
      for (int LoopY = 0; LoopY < BMIH.biHeight; LoopY++) {
        for (int LoopX = 0; LoopX < BMIH.biWidth; LoopX++) {
          ThisA = *DataPtr; // Move alpha and pre-multiply with RGB
          *DataPtr++ = (FillB * ThisA) >> 8;
          *DataPtr++ = (FillG * ThisA) >> 8;
          *DataPtr++ = (FillR * ThisA) >> 8;
          *DataPtr++ = ThisA; // Set Alpha
        }
      }

      // De-select bitmap
      SelectObject(hTextDC, hOldBMP);
    }
  }

  // De-select font and destroy temp DC
  SelectObject(hTextDC, hOldFont);
  DeleteDC(hTextDC);

  // Return DIBSection
  return hMyDIB;
}

#endif

