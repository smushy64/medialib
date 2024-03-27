#if !defined(MEDIA_IMPL_WIN32_COMMON_H)
#define MEDIA_IMPL_WIN32_COMMON_H
/**
 * @file   common.h
 * @brief  Common structures and functions used throughout Media Win32 Implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "core/defines.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "core/types.h"
#include "core/collections.h"
#include "media/cursor.h"
#include "media/keyboard.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <xinput.h>

struct Win32State {
    WNDCLASSEXA def_wndclass;
    volatile HWND cursor_lock;
    b8          active_gamepads[4];
    u16         gamepad_rumble[4][2];
    b32         gl_initialized;

    volatile u16 msg_wnd_result;
    volatile u16 msg_wnd_exit;
    HANDLE       msg_wnd_thread;
    HWND         msg_wnd;

    DWORD pid;

    union {
        struct {
            HMODULE USER32;
            HMODULE GDI32;
            HMODULE DWMAPI;
            HMODULE OLE32;
            HMODULE COMDLG32;
            HMODULE OPENGL32;
            HMODULE XINPUT;
        };
        HMODULE modules[7];
    };

    HCURSOR cursors[MEDIA_CURSOR_COUNT];

    enum InputKeymod keymod;
    b8 keys[packed_bool_memory_requirement( INPUT_KEYCODE_COUNT )];
};

// NOTE(alicia): common constants and globals

volatile extern struct Win32State* global_win32_state;

#define WM_INPUT_KEYBOARD                (WM_USER + 1)
#define WM_INPUT_KEYBOARD_TEXT           (WM_USER + 2)
#define WM_INPUT_MOUSE_POSITION          (WM_USER + 3)
#define WM_INPUT_MOUSE_POSITION_RELATIVE (WM_USER + 4)
#define WM_INPUT_MOUSE_BUTTON            (WM_USER + 5)

#define WIN32_MESSAGE_WINDOW_RESULT_PENDING  (0)
#define WIN32_MESSAGE_WINDOW_RESULT_SUCCESS  (1)
#define WIN32_MESSAGE_WINDOW_RESULT_ERROR    (2)
#define WIN32_MESSAGE_WINDOW_RESULT_FINISHED (3)

// NOTE(alicia): common functions

DWORD win32_message_window_thread( LPVOID in_params );
LRESULT win32_message_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
LRESULT win32_winproc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
HWND win32_get_active_window(void);
attr_unused void win32_error_text(
    DWORD code, usize format_len, const char* format, ... );

#if defined(MEDIA_ENABLE_LOGGING)
    #define win32_error( format, ... )\
        win32_error_text( GetLastError(),\
            sizeof( str_red( "[MEDIA ERROR] [WIN32] " format ) ) - 1,\
            str_red( "[MEDIA ERROR] [WIN32] " format ), ##__VA_ARGS__ )
#else
    #define win32_error( ... )
#endif

// NOTE(alicia): USER32 declarations.

#define decl( ret, fn, ... )\
typedef ret fn##FN( __VA_ARGS__ );\
extern fn##FN* in_##fn

decl( ATOM, RegisterClassExA, const WNDCLASSEXA* );
#define RegisterClassExA in_RegisterClassExA

decl( BOOL, UnregisterClassA, LPCSTR, HINSTANCE );
#define UnregisterClassA in_UnregisterClassA

decl( BOOL, AdjustWindowRectEx, LPRECT, DWORD, BOOL, DWORD );
#define AdjustWindowRectEx in_AdjustWindowRectEx

decl( BOOL, GetClientRect, HWND, LPRECT );
#define GetClientRect in_GetClientRect

decl( HWND, CreateWindowExA,
     DWORD, LPCSTR, LPCSTR, DWORD,
     int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID );
#define CreateWindowExA in_CreateWindowExA

decl( BOOL, DestroyWindow, HWND );
#define DestroyWindow in_DestroyWindow

decl( BOOL, SetWindowTextA, HWND, LPCSTR );
#define SetWindowTextA in_SetWindowTextA

decl( BOOL, SetWindowPos, HWND, HWND, int, int, int, int, UINT );
#define SetWindowPos in_SetWindowPos

decl( BOOL, GetWindowPlacement, HWND, WINDOWPLACEMENT* );
#define GetWindowPlacement in_GetWindowPlacement

decl( BOOL, SetWindowPlacement, HWND, const WINDOWPLACEMENT* );
#define SetWindowPlacement in_SetWindowPlacement

decl( BOOL, RegisterRawInputDevices, PCRAWINPUTDEVICE, UINT, UINT );
#define RegisterRawInputDevices in_RegisterRawInputDevices

decl( UINT, GetRawInputData, HRAWINPUT, UINT, LPVOID, PUINT, UINT );
#define GetRawInputData in_GetRawInputData

decl( LONG_PTR, SetWindowLongPtrA, HWND, int, LONG_PTR );
#define SetWindowLongPtrA in_SetWindowLongPtrA

decl( LONG_PTR, GetWindowLongPtrA, HWND, int );
#define GetWindowLongPtrA in_GetWindowLongPtrA

decl( BOOL, PeekMessageA, LPMSG, HWND, UINT, UINT, UINT );
#define PeekMessageA in_PeekMessageA

decl( LRESULT, DispatchMessageA, const MSG* );
#define DispatchMessageA in_DispatchMessageA

decl( LRESULT, DefWindowProcA, HWND, UINT, WPARAM, LPARAM );
#define DefWindowProcA in_DefWindowProcA

decl( HMONITOR, MonitorFromWindow, HWND, DWORD );
#define MonitorFromWindow in_MonitorFromWindow

decl( HMONITOR, MonitorFromPoint, POINT, DWORD );
#define MonitorFromPoint in_MonitorFromPoint

decl( BOOL, GetMonitorInfoA, HMONITOR, LPMONITORINFO );
#define GetMonitorInfoA in_GetMonitorInfoA

decl( BOOL, ClientToScreen, HWND, LPPOINT );
#define ClientToScreen in_ClientToScreen

decl( int, ShowCursor, BOOL );
#define ShowCursor in_ShowCursor

decl( BOOL, SetCursorPos, int, int );
#define SetCursorPos in_SetCursorPos

decl( HDC, GetDC, HWND );
#define GetDC in_GetDC

decl( int, ReleaseDC, HWND, HDC );
#define ReleaseDC in_ReleaseDC

decl( BOOL, ShowWindow, HWND, int );
#define ShowWindow in_ShowWindow

decl( int, MessageBoxA, HWND, LPCSTR, LPCSTR, UINT );
#define MessageBoxA in_MessageBoxA

decl( BOOL, PostMessageA, HWND, UINT, WPARAM, LPARAM );
#define PostMessageA in_PostMessageA

decl( HWND, GetForegroundWindow, void );
#define GetForegroundWindow in_GetForegroundWindow

decl( SHORT, GetKeyState, int );
#define GetKeyState in_GetKeyState

decl( void, PostQuitMessage, int );
#define PostQuitMessage in_PostQuitMessage

decl( BOOL, WaitMessage, void );
#define WaitMessage in_WaitMessage

decl( HCURSOR, LoadCursorA, HINSTANCE, LPCSTR );
#define LoadCursorA in_LoadCursorA

decl( HCURSOR, SetCursor, HCURSOR );
#define SetCursor in_SetCursor

decl( BOOL, ClipCursor, const RECT* );
#define ClipCursor in_ClipCursor

decl( int, MapWindowPoints, HWND, HWND, LPPOINT, UINT );
#define MapWindowPoints in_MapWindowPoints

decl( UINT, MapVirtualKeyA, UINT, UINT );
#define MapVirtualKeyA in_MapVirtualKeyA

decl( int, ToUnicode, UINT, UINT, const BYTE*, LPWSTR, int, UINT );
#define ToUnicode in_ToUnicode

decl( BOOL, GetCursorPos, LPPOINT );
#define GetCursorPos in_GetCursorPos

decl( BOOL, ScreenToClient, HWND, LPPOINT );
#define ScreenToClient in_ScreenToClient

decl( DWORD, GetWindowThreadProcessId, HWND, LPDWORD );
#define GetWindowThreadProcessId in_GetWindowThreadProcessId

// NOTE(alicia): GDI32 declarations

decl( HGDIOBJ, GetStockObject, int );
#define GetStockObject in_GetStockObject

// NOTE(alicia): DWMAPI declarations

decl( HRESULT, DwmSetWindowAttribute, HWND, DWORD, LPCVOID, DWORD );
#define DwmSetWindowAttribute in_DwmSetWindowAttribute

// NOTE(alicia): COMDLG32 declarations

decl( BOOL, GetOpenFileNameA, LPOPENFILENAMEA );
#define GetOpenFileNameA in_GetOpenFileNameA

decl( DWORD, CommDlgExtendedError, void );
#define CommDlgExtendedError in_CommDlgExtendedError

// NOTE(alicia): XINPUT declarations

decl( DWORD, XInputGetState, DWORD, XINPUT_STATE* );
#define XInputGetState in_XInputGetState

decl( DWORD, XInputSetState, DWORD, XINPUT_VIBRATION* );
#define XInputSetState in_XInputSetState

#endif /* Platform Windows */

#endif /* header guard */