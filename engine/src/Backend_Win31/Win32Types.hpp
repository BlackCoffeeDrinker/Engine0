#pragma once

// Minimal freestanding Win32 / Win32s API surface used by Backend_Win31.
// Intentionally does NOT include mingw <windows.h>.

#include <cstddef>
#include <cstdint>

#ifndef WINAPI
#define WINAPI __attribute__((stdcall))
#endif

#ifndef CALLBACK
#define CALLBACK __attribute__((stdcall))
#endif

#ifndef APIENTRY
#define APIENTRY WINAPI
#endif

#ifndef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT __declspec(dllimport)
#endif

#ifndef WINBASEAPI
#define WINBASEAPI DECLSPEC_IMPORT
#endif

#ifndef WINUSERAPI
#define WINUSERAPI DECLSPEC_IMPORT
#endif

#ifndef WINGDIAPI
#define WINGDIAPI DECLSPEC_IMPORT
#endif

#ifndef WINMMAPI
#define WINMMAPI DECLSPEC_IMPORT
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

using BOOL = int;
using BYTE = unsigned char;
using UCHAR = unsigned char;
using WORD = unsigned short;
using DWORD = unsigned long;
using UINT = unsigned int;
using LONG = long;
using ULONG = unsigned long;
using LONGLONG = long long;
using ULONGLONG = unsigned long long;
using INT = int;
using SHORT = short;
using USHORT = unsigned short;
using CHAR = char;
using WCHAR = wchar_t;
using FLOAT = float;
using ATOM = WORD;
using HANDLE = void *;
using PVOID = void *;
using LPVOID = void *;
using LPCVOID = const void *;
using LPSTR = char *;
using LPCSTR = const char *;
using LPWSTR = wchar_t *;
using LPCWSTR = const wchar_t *;
using LRESULT = long;
using WPARAM = unsigned int;
using LPARAM = long;
using HRESULT = long;
using COLORREF = DWORD;
using HGDIOBJ = void *;

struct HWND__;
struct HINSTANCE__;
struct HMENU__;
struct HDC__;
struct HBITMAP__;
struct HBRUSH__;
struct HFONT__;
struct HPEN__;
struct HICON__;
struct HCURSOR__;
struct HWAVEOUT__;

using HWND = HWND__ *;
using HINSTANCE = HINSTANCE__ *;
using HMODULE = HINSTANCE;
using HMENU = HMENU__ *;
using HDC = HDC__ *;
using HBITMAP = HBITMAP__ *;
using HBRUSH = HBRUSH__ *;
using HFONT = HFONT__ *;
using HPEN = HPEN__ *;
using HICON = HICON__ *;
using HCURSOR = HCURSOR__ *;
using HWAVEOUT = HWAVEOUT__ *;
using HPALETTE = void *;

using LPTSTR = LPSTR;
using LPCTSTR = LPCSTR;

constexpr DWORD MAX_PATH = 260;

#ifndef MAKEINTRESOURCEA
#define MAKEINTRESOURCEA(i) (reinterpret_cast<LPCSTR>(static_cast<ULONG_PTR>(static_cast<WORD>(i))))
#endif

using ULONG_PTR = unsigned long;
using LONG_PTR = long;
using DWORD_PTR = ULONG_PTR;
using UINT_PTR = unsigned int;
using INT_PTR = long;

// Fix FARPROC now that INT_PTR exists
using FARPROC = INT_PTR(WINAPI *)();

#define LOWORD(l) static_cast<WORD>(static_cast<DWORD_PTR>(l) & 0xffffu)
#define HIWORD(l) static_cast<WORD>((static_cast<DWORD_PTR>(l) >> 16) & 0xffffu)
#define LOBYTE(w) static_cast<BYTE>(static_cast<DWORD_PTR>(w) & 0xffu)
#define HIBYTE(w) static_cast<BYTE>((static_cast<DWORD_PTR>(w) >> 8) & 0xffu)
#define MAKELONG(a, b) static_cast<LONG>((static_cast<WORD>(a) & 0xffffu) | ((static_cast<DWORD>(static_cast<WORD>(b)) & 0xffffu) << 16))
#define MAKEWORD(a, b) static_cast<WORD>((static_cast<BYTE>(a) & 0xffu) | ((static_cast<WORD>(static_cast<BYTE>(b)) & 0xffu) << 8))
#define RGB(r, g, b) (static_cast<COLORREF>((static_cast<BYTE>(r) | (static_cast<WORD>(static_cast<BYTE>(g)) << 8)) | (static_cast<DWORD>(static_cast<BYTE>(b)) << 16)))

// Heap
constexpr DWORD HEAP_ZERO_MEMORY = 0x00000008;

// File I/O
constexpr DWORD GENERIC_READ = 0x80000000u;
constexpr DWORD GENERIC_WRITE = 0x40000000u;
constexpr DWORD FILE_SHARE_READ = 0x00000001u;
constexpr DWORD FILE_SHARE_WRITE = 0x00000002u;
constexpr DWORD CREATE_NEW = 1;
constexpr DWORD CREATE_ALWAYS = 2;
constexpr DWORD OPEN_EXISTING = 3;
constexpr DWORD OPEN_ALWAYS = 4;
constexpr DWORD TRUNCATE_EXISTING = 5;
constexpr DWORD FILE_ATTRIBUTE_NORMAL = 0x00000080u;
constexpr DWORD INVALID_FILE_SIZE = 0xFFFFFFFFu;
// Not constexpr: reinterpret_cast pointer from integer is not a constant expression.
inline HANDLE InvalidHandleValue() {
  return reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(-1));
}
#define INVALID_HANDLE_VALUE (InvalidHandleValue())

constexpr DWORD FILE_BEGIN = 0;
constexpr DWORD FILE_CURRENT = 1;
constexpr DWORD FILE_END = 2;

// Window styles / messages
constexpr UINT WM_NULL = 0x0000;
constexpr UINT WM_CREATE = 0x0001;
constexpr UINT WM_DESTROY = 0x0002;
constexpr UINT WM_MOVE = 0x0003;
constexpr UINT WM_SIZE = 0x0005;
constexpr UINT WM_ACTIVATE = 0x0006;
constexpr UINT WM_SETFOCUS = 0x0007;
constexpr UINT WM_KILLFOCUS = 0x0008;
constexpr UINT WM_PAINT = 0x000F;
constexpr UINT WM_CLOSE = 0x0010;
constexpr UINT WM_QUIT = 0x0012;
constexpr UINT WM_ERASEBKGND = 0x0014;
constexpr UINT WM_SETTEXT = 0x000C;
constexpr UINT WM_GETTEXT = 0x000D;
constexpr UINT WM_COMMAND = 0x0111;
constexpr UINT WM_SYSCOMMAND = 0x0112;
constexpr UINT WM_KEYDOWN = 0x0100;
constexpr UINT WM_KEYUP = 0x0101;
constexpr UINT WM_CHAR = 0x0102;
constexpr UINT WM_SYSKEYDOWN = 0x0104;
constexpr UINT WM_SYSKEYUP = 0x0105;
constexpr UINT WM_MOUSEMOVE = 0x0200;
constexpr UINT WM_LBUTTONDOWN = 0x0201;
constexpr UINT WM_LBUTTONUP = 0x0202;
constexpr UINT WM_RBUTTONDOWN = 0x0204;
constexpr UINT WM_RBUTTONUP = 0x0205;
constexpr UINT WM_MBUTTONDOWN = 0x0207;
constexpr UINT WM_MBUTTONUP = 0x0208;
constexpr UINT WM_MOUSEWHEEL = 0x020A;

constexpr DWORD WS_OVERLAPPED = 0x00000000L;
constexpr DWORD WS_POPUP = 0x80000000L;
constexpr DWORD WS_CHILD = 0x40000000L;
constexpr DWORD WS_MINIMIZE = 0x20000000L;
constexpr DWORD WS_VISIBLE = 0x10000000L;
constexpr DWORD WS_DISABLED = 0x08000000L;
constexpr DWORD WS_CLIPSIBLINGS = 0x04000000L;
constexpr DWORD WS_CLIPCHILDREN = 0x02000000L;
constexpr DWORD WS_MAXIMIZE = 0x01000000L;
constexpr DWORD WS_CAPTION = 0x00C00000L;
constexpr DWORD WS_BORDER = 0x00800000L;
constexpr DWORD WS_DLGFRAME = 0x00400000L;
constexpr DWORD WS_VSCROLL = 0x00200000L;
constexpr DWORD WS_HSCROLL = 0x00100000L;
constexpr DWORD WS_SYSMENU = 0x00080000L;
constexpr DWORD WS_THICKFRAME = 0x00040000L;
constexpr DWORD WS_GROUP = 0x00020000L;
constexpr DWORD WS_TABSTOP = 0x00010000L;
constexpr DWORD WS_MINIMIZEBOX = 0x00020000L;
constexpr DWORD WS_MAXIMIZEBOX = 0x00010000L;
constexpr DWORD WS_OVERLAPPEDWINDOW = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

constexpr DWORD CW_USEDEFAULT = 0x80000000u;

constexpr int SW_HIDE = 0;
constexpr int SW_SHOWNORMAL = 1;
constexpr int SW_SHOW = 5;
constexpr int SW_RESTORE = 9;

constexpr UINT PM_NOREMOVE = 0x0000;
constexpr UINT PM_REMOVE = 0x0001;
constexpr UINT PM_NOYIELD = 0x0002;

constexpr UINT MF_STRING = 0x00000000u;
constexpr UINT MF_ENABLED = 0x00000000u;
constexpr UINT MF_GRAYED = 0x00000001u;
constexpr UINT MF_DISABLED = 0x00000002u;
constexpr UINT MF_SEPARATOR = 0x00000800u;
constexpr UINT MF_POPUP = 0x00000010u;

constexpr LONG GWL_STYLE = -16;
constexpr LONG GWL_EXSTYLE = -20;
constexpr LONG GWL_USERDATA = -21;
constexpr LONG GWL_WNDPROC = -4;
constexpr LONG GWL_HINSTANCE = -6;
constexpr LONG GWL_ID = -12;

constexpr UINT CS_VREDRAW = 0x0001;
constexpr UINT CS_HREDRAW = 0x0002;
constexpr UINT CS_OWNDC = 0x0020;
constexpr UINT CS_CLASSDC = 0x0040;
constexpr UINT CS_PARENTDC = 0x0080;
constexpr UINT CS_DBLCLKS = 0x0008;

constexpr int IDC_ARROW = 32512;
constexpr int IDI_APPLICATION = 32512;

constexpr UINT SRCCOPY = 0x00CC0020u;
constexpr UINT DIB_RGB_COLORS = 0;
constexpr UINT DIB_PAL_COLORS = 1;
constexpr UINT BI_RGB = 0;

constexpr LONG WA_INACTIVE = 0;
constexpr LONG WA_ACTIVE = 1;
constexpr LONG WA_CLICKACTIVE = 2;

// Virtual-key codes (subset)
constexpr int VK_LBUTTON = 0x01;
constexpr int VK_RBUTTON = 0x02;
constexpr int VK_CANCEL = 0x03;
constexpr int VK_MBUTTON = 0x04;
constexpr int VK_BACK = 0x08;
constexpr int VK_TAB = 0x09;
constexpr int VK_RETURN = 0x0D;
constexpr int VK_SHIFT = 0x10;
constexpr int VK_CONTROL = 0x11;
constexpr int VK_MENU = 0x12;
constexpr int VK_PAUSE = 0x13;
constexpr int VK_CAPITAL = 0x14;
constexpr int VK_ESCAPE = 0x1B;
constexpr int VK_SPACE = 0x20;
constexpr int VK_PRIOR = 0x21;
constexpr int VK_NEXT = 0x22;
constexpr int VK_END = 0x23;
constexpr int VK_HOME = 0x24;
constexpr int VK_LEFT = 0x25;
constexpr int VK_UP = 0x26;
constexpr int VK_RIGHT = 0x27;
constexpr int VK_DOWN = 0x28;
constexpr int VK_INSERT = 0x2D;
constexpr int VK_DELETE = 0x2E;
constexpr int VK_F1 = 0x70;
constexpr int VK_F2 = 0x71;
constexpr int VK_F3 = 0x72;
constexpr int VK_F4 = 0x73;
constexpr int VK_F5 = 0x74;
constexpr int VK_F6 = 0x75;
constexpr int VK_F7 = 0x76;
constexpr int VK_F8 = 0x77;
constexpr int VK_F9 = 0x78;
constexpr int VK_F10 = 0x79;
constexpr int VK_F11 = 0x7A;
constexpr int VK_F12 = 0x7B;

// waveOut
constexpr UINT WAVE_MAPPER = static_cast<UINT>(-1);
constexpr DWORD WHDR_DONE = 0x00000001;
constexpr DWORD WHDR_PREPARED = 0x00000002;
constexpr DWORD WHDR_BEGINLOOP = 0x00000004;
constexpr DWORD WHDR_ENDLOOP = 0x00000008;
constexpr DWORD WHDR_INQUEUE = 0x00000010;
constexpr WORD WAVE_FORMAT_PCM = 1;
constexpr UINT CALLBACK_NULL = 0x00000000;

struct POINT {
  LONG x;
  LONG y;
};

struct RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
};

struct SIZE {
  LONG cx;
  LONG cy;
};

struct MSG {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
};

using WNDPROC = LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM);

struct WNDCLASSA {
  UINT style;
  WNDPROC lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
  HCURSOR hCursor;
  HBRUSH hbrBackground;
  LPCSTR lpszMenuName;
  LPCSTR lpszClassName;
};

struct PAINTSTRUCT {
  HDC hdc;
  BOOL fErase;
  RECT rcPaint;
  BOOL fRestore;
  BOOL fIncUpdate;
  BYTE rgbReserved[32];
};

struct BITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
};

struct RGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
};

struct BITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1];
};

struct WAVEFORMATEX {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
};

struct WAVEHDR {
  LPSTR lpData;
  DWORD dwBufferLength;
  DWORD dwBytesRecorded;
  DWORD_PTR dwUser;
  DWORD dwFlags;
  DWORD dwLoops;
  WAVEHDR *lpNext;
  DWORD_PTR reserved;
};

extern "C" {

// kernel32
WINBASEAPI HANDLE WINAPI GetProcessHeap();
WINBASEAPI LPVOID WINAPI HeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes);
WINBASEAPI LPVOID WINAPI HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, DWORD dwBytes);
WINBASEAPI BOOL WINAPI HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
WINBASEAPI DWORD WINAPI HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);

WINBASEAPI HMODULE WINAPI GetModuleHandleA(LPCSTR lpModuleName);
WINBASEAPI HMODULE WINAPI LoadLibraryA(LPCSTR lpLibFileName);
WINBASEAPI FARPROC WINAPI GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
WINBASEAPI BOOL WINAPI FreeLibrary(HMODULE hLibModule);
WINBASEAPI void WINAPI ExitProcess(UINT uExitCode);
WINBASEAPI void WINAPI Sleep(DWORD dwMilliseconds);
WINBASEAPI DWORD WINAPI GetTickCount();
WINBASEAPI void WINAPI OutputDebugStringA(LPCSTR lpOutputString);
WINBASEAPI DWORD WINAPI GetLastError();

WINBASEAPI HANDLE WINAPI CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                     void *lpSecurityAttributes, DWORD dwCreationDisposition,
                                     DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
WINBASEAPI BOOL WINAPI ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                                DWORD *lpNumberOfBytesRead, void *lpOverlapped);
WINBASEAPI BOOL WINAPI WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                                 DWORD *lpNumberOfBytesWritten, void *lpOverlapped);
WINBASEAPI BOOL WINAPI CloseHandle(HANDLE hObject);
WINBASEAPI DWORD WINAPI SetFilePointer(HANDLE hFile, LONG lDistanceToMove, LONG *lpDistanceToMoveHigh, DWORD dwMoveMethod);
WINBASEAPI DWORD WINAPI GetFileSize(HANDLE hFile, DWORD *lpFileSizeHigh);

// user32
WINUSERAPI ATOM WINAPI RegisterClassA(const WNDCLASSA *lpWndClass);
WINUSERAPI HWND WINAPI CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
                                       DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                                       HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
WINUSERAPI BOOL WINAPI ShowWindow(HWND hWnd, int nCmdShow);
WINUSERAPI BOOL WINAPI UpdateWindow(HWND hWnd);
WINUSERAPI BOOL WINAPI DestroyWindow(HWND hWnd);
WINUSERAPI BOOL WINAPI GetMessageA(MSG *lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
WINUSERAPI BOOL WINAPI PeekMessageA(MSG *lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
WINUSERAPI BOOL WINAPI TranslateMessage(const MSG *lpMsg);
WINUSERAPI LRESULT WINAPI DispatchMessageA(const MSG *lpMsg);
WINUSERAPI void WINAPI PostQuitMessage(int nExitCode);
WINUSERAPI BOOL WINAPI PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINUSERAPI LRESULT WINAPI DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
WINUSERAPI BOOL WINAPI SetWindowTextA(HWND hWnd, LPCSTR lpString);
WINUSERAPI int WINAPI GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount);
WINUSERAPI HDC WINAPI BeginPaint(HWND hWnd, PAINTSTRUCT *lpPaint);
WINUSERAPI BOOL WINAPI EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint);
WINUSERAPI HDC WINAPI GetDC(HWND hWnd);
WINUSERAPI int WINAPI ReleaseDC(HWND hWnd, HDC hDC);
WINUSERAPI BOOL WINAPI InvalidateRect(HWND hWnd, const RECT *lpRect, BOOL bErase);
WINUSERAPI BOOL WINAPI GetClientRect(HWND hWnd, RECT *lpRect);
WINUSERAPI BOOL WINAPI ClientToScreen(HWND hWnd, POINT *lpPoint);
WINUSERAPI BOOL WINAPI ScreenToClient(HWND hWnd, POINT *lpPoint);
WINUSERAPI BOOL WINAPI GetCursorPos(POINT *lpPoint);
WINUSERAPI SHORT WINAPI GetAsyncKeyState(int vKey);
WINUSERAPI HCURSOR WINAPI LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
WINUSERAPI HICON WINAPI LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName);
WINUSERAPI HMENU WINAPI CreateMenu();
WINUSERAPI HMENU WINAPI CreatePopupMenu();
WINUSERAPI BOOL WINAPI DestroyMenu(HMENU hMenu);
WINUSERAPI BOOL WINAPI AppendMenuA(HMENU hMenu, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem);
WINUSERAPI BOOL WINAPI SetMenu(HWND hWnd, HMENU hMenu);
WINUSERAPI BOOL WINAPI DrawMenuBar(HWND hWnd);
WINUSERAPI LONG WINAPI GetWindowLongA(HWND hWnd, int nIndex);
WINUSERAPI LONG WINAPI SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong);
WINUSERAPI int WINAPI MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

inline HWND CreateWindowA(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
                          int X, int Y, int nWidth, int nHeight,
                          HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
  return CreateWindowExA(0, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight,
                         hWndParent, hMenu, hInstance, lpParam);
}

// gdi32
WINGDIAPI HBITMAP WINAPI CreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT usage,
                                          void **ppvBits, HANDLE hSection, DWORD offset);
WINGDIAPI UINT WINAPI SetDIBColorTable(HDC hdc, UINT iStart, UINT cEntries, const RGBQUAD *prgbq);
WINGDIAPI BOOL WINAPI BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);
WINGDIAPI int WINAPI StretchDIBits(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight,
                                   int xSrc, int ySrc, int SrcWidth, int SrcHeight,
                                   const void *lpBits, const BITMAPINFO *lpbmi, UINT iUsage, DWORD rop);
WINGDIAPI HDC WINAPI CreateCompatibleDC(HDC hdc);
WINGDIAPI BOOL WINAPI DeleteDC(HDC hdc);
WINGDIAPI HGDIOBJ WINAPI SelectObject(HDC hdc, HGDIOBJ h);
WINGDIAPI BOOL WINAPI DeleteObject(HGDIOBJ ho);
WINGDIAPI int WINAPI SetDIBitsToDevice(HDC hdc, int xDest, int yDest, DWORD w, DWORD h,
                                       int xSrc, int ySrc, UINT StartScan, UINT cLines,
                                       const void *lpvBits, const BITMAPINFO *lpbmi, UINT ColorUse);
WINGDIAPI COLORREF WINAPI SetPixel(HDC hdc, int x, int y, COLORREF color);
WINGDIAPI HBRUSH WINAPI CreateSolidBrush(COLORREF color);

// winmm / waveOut
WINMMAPI UINT WINAPI waveOutOpen(HWAVEOUT *phwo, UINT uDeviceID, const WAVEFORMATEX *pwfx,
                                 DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
WINMMAPI UINT WINAPI waveOutClose(HWAVEOUT hwo);
WINMMAPI UINT WINAPI waveOutPrepareHeader(HWAVEOUT hwo, WAVEHDR *pwh, UINT cbwh);
WINMMAPI UINT WINAPI waveOutUnprepareHeader(HWAVEOUT hwo, WAVEHDR *pwh, UINT cbwh);
WINMMAPI UINT WINAPI waveOutWrite(HWAVEOUT hwo, WAVEHDR *pwh, UINT cbwh);
WINMMAPI UINT WINAPI waveOutReset(HWAVEOUT hwo);
WINMMAPI UINT WINAPI waveOutPause(HWAVEOUT hwo);
WINMMAPI UINT WINAPI waveOutRestart(HWAVEOUT hwo);

// CRT-ish helpers sometimes referenced by libgcc
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);

}// extern "C"
