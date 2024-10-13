#if !defined(MEDIA_IMPL_WIN32_COMMON_H)
#define MEDIA_IMPL_WIN32_COMMON_H
/**
 * @file   common.h
 * @brief  Media Windows Common header.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 10, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/types.h"
#include "media/internal/logging.h"
#include "media/cursor.h"

// IWYU pragma: begin_exports
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <combaseapi.h>

// NOTE(alicia): defined in winuser.h, conflicts with MouseButton enum
#undef MB_RIGHT
// IWYU pragma: end_exports

#define WIN32_DEFAULT_WINDOW_CLASS L"MediaDefaultWindow"

#if !defined(DWMWA_USE_IMMERSIVE_DARK_MODE)
    #define DWMWA_USE_IMMERSIVE_DARK_MODE (20)
#endif

struct Win32State {
    union {
        struct {
            HMODULE USER32;
            HMODULE GDI32;
            HMODULE DWMAPI;
            HMODULE XINPUT;
            HMODULE OPENGL32;
            HMODULE OLE32;
        };
        HMODULE array[7];
    } modules;
    enum KeyboardMod mod;
    enum MouseButton mb;
};
extern struct Win32State* global_win32_state;
extern HCURSOR global_win32_cursors[CURSOR_TYPE_COUNT];
extern _Bool global_win32_cursor_hidden;

#define win32_error(...) media_error( "win32: " __VA_ARGS__)
#define win32_warn(...) media_warn( "win32: " __VA_ARGS__)

#define CoCheck( hres ) ((hres) == (S_OK))
#define CoRelease( punk ) do {\
    if( (punk) != NULL ) {\
        (punk)->lpVtbl->Release( (punk) );\
        (punk) = NULL;\
    }\
} while(0)

wchar_t* win32_utf8_to_ucs2_alloc(
    uint32_t utf8_len, const char* utf8, uint32_t* opt_out_len );

MONITORINFO win32_monitor_info( HWND opt_hwnd );

void win32_error_message_full(
    DWORD error_code, uint32_t message_len, const char* message );

#define win32_error_message( error_code, message )\
    win32_error_message_full( error_code, sizeof(message) - 1, message )

LRESULT win32_winproc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

HWND win32_get_focused_window(void);

// NOTE(alicia): windows library functions

#define decl( ret, fn, ... )\
    typedef ret fn##FN( __VA_ARGS__ );\
    extern fn##FN* in_##fn

// NOTE(alicia): USER32

decl( int, MessageBoxW, HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType );
#define MessageBoxW in_MessageBoxW

decl( ATOM, RegisterClassExW, const WNDCLASSEXW* unnamedParam1 );
#define RegisterClassExW in_RegisterClassExW

decl( BOOL, UnregisterClassW, LPCWSTR lpClassName, HINSTANCE hInstance );
#define UnregisterClassW in_UnregisterClassW

decl( HWND, CreateWindowExW,
     DWORD dwExStyle, LPCWSTR lpClassName,
     LPCWSTR lpWindowName, DWORD dwStyle,
     int X, int Y, int nWidth, int nHeight,
     HWND hWndParent, HMENU hMenu,
     HINSTANCE hInstance, LPVOID lpParam );
#define CreateWindowExW in_CreateWindowExW

decl( BOOL, DestroyWindow, HWND hWnd );
#define DestroyWindow in_DestroyWindow

decl( LRESULT, DefWindowProcW, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
#define DefWindowProcW in_DefWindowProcW

decl( BOOL, AdjustWindowRectEx,
     LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle );
#define AdjustWindowRectEx in_AdjustWindowRectEx

decl( BOOL, GetClientRect, HWND hWnd, LPRECT lpRect );
#define GetClientRect in_GetClientRect

decl( HDC, GetDC, HWND hWnd );
#define GetDC in_GetDC

decl( int, ReleaseDC, HWND hWnd, HDC hDC );
#define ReleaseDC in_ReleaseDC

decl( BOOL, ShowWindow, HWND hWnd, int nCmdShow );
#define ShowWindow in_ShowWindow

decl( BOOL, PeekMessageW,
     LPMSG lpMsg, HWND hWnd,
     UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg );
#define PeekMessageW in_PeekMessageW

decl( BOOL, TranslateMessage, const MSG* lpMsg );
#define TranslateMessage in_TranslateMessage

decl( LRESULT, DispatchMessageW, const MSG* lpMsg );
#define DispatchMessageW in_DispatchMessageW 

decl( BOOL, SetWindowTextW, HWND hWnd, LPCWSTR lpString );
#define SetWindowTextW in_SetWindowTextW

decl( BOOL, SetWindowPos,
     HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags );
#define SetWindowPos in_SetWindowPos

decl( HMONITOR, MonitorFromPoint, POINT pt, DWORD dwFlags );
#define MonitorFromPoint in_MonitorFromPoint

decl( HMONITOR, MonitorFromWindow, HWND hwnd, DWORD dwFlags );
#define MonitorFromWindow in_MonitorFromWindow

decl( BOOL, GetMonitorInfoW, HMONITOR hMonitor, LPMONITORINFO lpmi );
#define GetMonitorInfoW in_GetMonitorInfoW

decl( BOOL, GetWindowPlacement, HWND hWnd, WINDOWPLACEMENT* lpwndpl );
#define GetWindowPlacement in_GetWindowPlacement

decl( BOOL, SetWindowPlacement, HWND hWnd, const WINDOWPLACEMENT* lpwndpl );
#define SetWindowPlacement in_SetWindowPlacement

decl( HCURSOR, SetCursor, HCURSOR hCursor );
#define SetCursor in_SetCursor

decl( HCURSOR, LoadCursorA, HINSTANCE hInstance, LPCSTR lpCursorName );
#define LoadCursorA in_LoadCursorA

decl( int, ShowCursor, BOOL bShow );
#define ShowCursor in_ShowCursor

decl( BOOL, ClientToScreen, HWND hWnd, LPPOINT lpPoint );
#define ClientToScreen in_ClientToScreen

decl( BOOL, SetCursorPos, int X, int Y );
#define SetCursorPos in_SetCursorPos

decl( UINT, MapVirtualKeyW, UINT uCode, UINT uMapType );
#define MapVirtualKeyW in_MapVirtualKeyW

decl( HWND, GetForegroundWindow, void );
#define GetForegroundWindow in_GetForegroundWindow

decl( DWORD, GetWindowThreadProcessId, HWND hWnd, LPDWORD lpdwProcessId );
#define GetWindowThreadProcessId in_GetWindowThreadProcessId

decl( BOOL, PostMessageW, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
#define PostMessageW in_PostMessageW

decl( BOOL, GetCursorPos, LPPOINT lpPoint );
#define GetCursorPos in_GetCursorPos

decl( SHORT, GetKeyState, int nVirtKey );
#define GetKeyState in_GetKeyState

decl( int, ToUnicode,
     UINT wVirtKey, UINT wScanCode,
     const BYTE* lpKeyState, LPWSTR pwszBuff, int cchBuff, UINT wFlags );
#define ToUnicode in_ToUnicode

decl( BOOL, ScreenToClient, HWND hWnd, LPPOINT lpPoint );
#define ScreenToClient in_ScreenToClient

#if defined(MEDIA_ARCH_64_BIT)

    decl( LONG_PTR, SetWindowLongPtrW, HWND hWnd, int nIndex, LONG_PTR dwNewLong );
    #define SetWindowLongPtrW in_SetWindowLongPtrW

    decl( LONG_PTR, GetWindowLongPtrW, HWND hWnd, int nIndex );
    #define GetWindowLongPtrW in_GetWindowLongPtrW

    #define WIN32_PTR LONG_PTR

#else /* Arch 64-bit */

    typedef LONG SetWindowLongWFN( HWND hWnd, int nIndex, LONG dwNewLong );
    typedef LONG GetWindowLongWFN( HWND hWnd, int nIndex );

    extern SetWindowLongWFN* in_SetWindowLongW;
    extern GetWindowLongWFN* in_GetWindowLongW;

    #define SetWindowLongPtrW in_SetWindowLongW
    #define GetWindowLongPtrW in_GetWindowLongW

    #define WIN32_PTR LONG

#endif /* Arch 32-bit */

// NOTE(alicia): GDI32

decl( HGDIOBJ, GetStockObject, int i );
#define GetStockObject in_GetStockObject

// NOTE(alicia): OLE32

decl( HRESULT, CoInitialize, LPVOID pvReserved );
#define CoInitialize in_CoInitialize

decl( HRESULT, CoCreateInstance,
    REFCLSID rclsid, LPUNKNOWN pUnkOuter,
    DWORD dwClsContext, REFIID riid, LPVOID* ppv );
#define CoCreateInstance in_CoCreateInstance

decl( void, CoTaskMemFree, LPVOID pv );
#define CoTaskMemFree in_CoTaskMemFree

decl( void, CoUninitialize, void );
#define CoUninitialize in_CoUninitialize

decl( HRESULT, PropVariantClear, PROPVARIANT* pvar );
#define PropVariantClear in_PropVariantClear

// NOTE(alicia): DWMAPI

decl( HRESULT, DwmSetWindowAttribute,
     HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute );
#define DwmSetWindowAttribute in_DwmSetWindowAttribute

#endif /* Platform Windows */
#endif /* header guard */
