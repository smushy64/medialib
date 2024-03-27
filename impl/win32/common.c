/**
 * @file   common.c
 * @brief  media/lib.h implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "impl/win32/common.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "core/sync.h"
#include "core/memory.h"
#include "core/print.h"

#include "media/lib.h"
#include "media/internal/logging.h"

#include <windowsx.h>

volatile struct Win32State* global_win32_state = NULL;

#define def( fn )\
fn##FN* in_##fn = NULL

def( RegisterClassExA );
def( UnregisterClassA );
def( AdjustWindowRectEx );
def( GetClientRect );
def( CreateWindowExA );
def( DestroyWindow );
def( SetWindowTextA );
def( SetWindowPos );
def( GetWindowPlacement );
def( SetWindowPlacement );
def( RegisterRawInputDevices );
def( GetRawInputData );
def( SetWindowLongPtrA );
def( GetWindowLongPtrA );
def( PeekMessageA );
def( DispatchMessageA );
def( DefWindowProcA );
def( MonitorFromWindow );
def( MonitorFromPoint );
def( GetMonitorInfoA );
def( ClientToScreen );
def( ShowCursor );
def( SetCursorPos );
def( GetDC );
def( ReleaseDC );
def( ShowWindow );
def( MessageBoxA );
def( PostMessageA );
def( GetForegroundWindow );
def( GetKeyState );
def( PostQuitMessage );
def( WaitMessage );
def( LoadCursorA );
def( SetCursor );
def( ClipCursor );
def( MapWindowPoints );
def( MapVirtualKeyA );
def( ToUnicode );
def( GetCursorPos );
def( ScreenToClient );
def( GetWindowThreadProcessId );

def( GetStockObject );

def( DwmSetWindowAttribute );

def( GetOpenFileNameA );
def( CommDlgExtendedError );

def( XInputGetState );
def( XInputSetState );

#undef def

attr_media_api b32 media_initialize(
    enum MediaLoggingLevel log_level,
    MediaLoggingCallbackFN* log_callback, void* log_callback_params
) {
    media_set_logging_level( log_level );
    media_set_logging_callback( log_callback, log_callback_params );

    #define destroy()\
        for( u32 i = 0; i < static_array_len( global_win32_state->modules ); ++i ) {\
            if( global_win32_state->modules[i] ) {\
                FreeLibrary( global_win32_state->modules[i] );\
            }\
        }\
        HeapFree( GetProcessHeap(), 0, (void*)global_win32_state )

    global_win32_state = HeapAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*global_win32_state) );
    if( !global_win32_state ) {
        media_error( "[WIN32] failed to allocate memory for win32!" );
        return false;
    }

    global_win32_state->pid = GetCurrentProcessId();

    #define open( name ) do {\
        global_win32_state->name = LoadLibraryA( #name ".DLL" );\
        if( !global_win32_state->name ) {\
            win32_error( "failed to open library " #name "!" );\
            destroy();\
            return false;\
        }\
    } while(0)

    #define load( mod, name ) do {\
        name = (name##FN*)GetProcAddress( global_win32_state->mod, #name );\
        if( !name ) {\
            win32_error( "failed to load " #name " from library " #mod "!" );\
            destroy();\
            return false;\
        }\
    } while(0)

    open( USER32 );
    open( GDI32 );
    open( DWMAPI );
    open( COMDLG32 );

    load( USER32, RegisterClassExA );
    load( USER32, UnregisterClassA );
    load( USER32, AdjustWindowRectEx );
    load( USER32, GetClientRect );
    load( USER32, CreateWindowExA );
    load( USER32, DestroyWindow );
    load( USER32, SetWindowTextA );
    load( USER32, SetWindowPos );
    load( USER32, GetWindowPlacement );
    load( USER32, SetWindowPlacement );
    load( USER32, RegisterRawInputDevices );
    load( USER32, GetRawInputData );
    load( USER32, SetWindowLongPtrA );
    load( USER32, GetWindowLongPtrA );
    load( USER32, PeekMessageA );
    load( USER32, DispatchMessageA );
    load( USER32, DefWindowProcA );
    load( USER32, MonitorFromWindow );
    load( USER32, MonitorFromPoint );
    load( USER32, GetMonitorInfoA );
    load( USER32, ClientToScreen );
    load( USER32, ShowCursor );
    load( USER32, SetCursorPos );
    load( USER32, GetDC );
    load( USER32, ReleaseDC );
    load( USER32, ShowWindow );
    load( USER32, MessageBoxA );
    load( USER32, PostMessageA );
    load( USER32, GetForegroundWindow );
    load( USER32, GetKeyState );
    load( USER32, PostQuitMessage );
    load( USER32, WaitMessage );
    load( USER32, LoadCursorA );
    load( USER32, SetCursor );
    load( USER32, ClipCursor );
    load( USER32, MapWindowPoints );
    load( USER32, MapVirtualKeyA );
    load( USER32, ToUnicode );
    load( USER32, GetCursorPos );
    load( USER32, ScreenToClient );
    load( USER32, GetWindowThreadProcessId );

    load( GDI32, GetStockObject );

    load( DWMAPI, DwmSetWindowAttribute );

    load( COMDLG32, GetOpenFileNameA );
    load( COMDLG32, CommDlgExtendedError );

    global_win32_state->XINPUT = LoadLibraryA( "XINPUT1_4.DLL" );
    if( !global_win32_state->XINPUT ) {
        global_win32_state->XINPUT = LoadLibraryA( "XINPUT9_1_0.DLL" );
        if( !global_win32_state->XINPUT ) {
            global_win32_state->XINPUT = LoadLibraryA( "XINPUT1_3.DLL" );
            if( !global_win32_state->XINPUT ) {
                win32_error( "failed to open any version of Xinput library!" );
                destroy();
                return false;
            }
        }
    }

    load( XINPUT, XInputGetState );
    load( XINPUT, XInputSetState );

    global_win32_state->cursors[MEDIA_CURSOR_ARROW]      = LoadCursorA( NULL, IDC_ARROW );
    global_win32_state->cursors[MEDIA_CURSOR_HAND]       = LoadCursorA( NULL, IDC_HAND );
    global_win32_state->cursors[MEDIA_CURSOR_TEXT]       = LoadCursorA( NULL, IDC_IBEAM );
    global_win32_state->cursors[MEDIA_CURSOR_WAIT]       = LoadCursorA( NULL, IDC_WAIT );
    global_win32_state->cursors[MEDIA_CURSOR_ARROW_WAIT] = LoadCursorA( NULL, IDC_APPSTARTING );
    global_win32_state->cursors[MEDIA_CURSOR_SIZE_ALL]   = LoadCursorA( NULL, IDC_SIZEALL );
    global_win32_state->cursors[MEDIA_CURSOR_SIZE_V]     = LoadCursorA( NULL, IDC_SIZENS );
    global_win32_state->cursors[MEDIA_CURSOR_SIZE_H]     = LoadCursorA( NULL, IDC_SIZEWE );
    global_win32_state->cursors[MEDIA_CURSOR_SIZE_L]     = LoadCursorA( NULL, IDC_SIZENWSE );
    global_win32_state->cursors[MEDIA_CURSOR_SIZE_R]     = LoadCursorA( NULL, IDC_SIZENESW );

    HMODULE module = GetModuleHandleA( NULL );

    global_win32_state->def_wndclass.cbSize        = sizeof(global_win32_state->def_wndclass);
    global_win32_state->def_wndclass.lpfnWndProc   = win32_winproc;
    global_win32_state->def_wndclass.hInstance     = module;
    global_win32_state->def_wndclass.lpszClassName = "LibMediaWindowClass";
    global_win32_state->def_wndclass.hbrBackground = (HBRUSH)GetStockBrush( BLACK_BRUSH );

    if( !RegisterClassExA( (const WNDCLASSEXA*)&global_win32_state->def_wndclass ) ) {
        win32_error( "failed to register default window class!" );
        destroy();
        return false;
    }

    read_write_fence();

    global_win32_state->msg_wnd_thread = CreateThread(
        NULL, kibibytes(512), win32_message_window_thread,
        NULL, 0, NULL );
    if( global_win32_state->msg_wnd_thread ) {
        media_info( "[WIN32] message window thread created." );
    } else {
        win32_error( "failed to create message window thread!" );
        UnregisterClassA( global_win32_state->def_wndclass.lpszClassName, module );
        destroy();
        return false;
    }

    read_write_fence();
    while( !global_win32_state->msg_wnd_result ) {}
    read_write_fence();

    if( global_win32_state->msg_wnd_result == WIN32_MESSAGE_WINDOW_RESULT_ERROR ) {
        UnregisterClassA( global_win32_state->def_wndclass.lpszClassName, module );
        destroy();
        return false;
    }

    #undef open
    #undef load
    #undef destroy
    return true;
}
attr_media_api void media_shutdown(void) {
    interlocked_increment( &global_win32_state->msg_wnd_exit );
    read_write_fence();

    while( global_win32_state->msg_wnd_result != WIN32_MESSAGE_WINDOW_RESULT_FINISHED )
    {}

    read_write_fence();

    HMODULE module = GetModuleHandleA( NULL );
    UnregisterClassA( global_win32_state->def_wndclass.lpszClassName, module );
    for( u32 i = 0; i < static_array_len( global_win32_state->modules ); ++i ) {
        if( global_win32_state->modules[i] ) {
            FreeLibrary( global_win32_state->modules[i] );
        }
    }

    HeapFree( GetProcessHeap(), 0, (void*)global_win32_state );
}

HWND win32_get_active_window(void) {
    HWND active = GetForegroundWindow();
    DWORD pid;
    GetWindowThreadProcessId( active, &pid );
    if( pid != global_win32_state->pid ) {
        return NULL;
    }

    return active;
}

#define WIN32_MEDIA_ERROR_BUFFER_PREFIX CONSOLE_COLOR_RED "[WIN32] "
#define WIN32_MEDIA_ERROR_BUFFER_SUFFIX CONSOLE_COLOR_RESET "\n"
#define WIN32_MEDIA_ERROR_BUFFER_CAP\
    (255 + (\
        (sizeof(WIN32_MEDIA_ERROR_BUFFER_PREFIX) - 1) +\
        (sizeof(WIN32_MEDIA_ERROR_BUFFER_SUFFIX) - 1)\
    ) )

attr_unused void win32_error_text(
    DWORD error_code, usize format_len, const char* format, ...
) {
    va_list va;
    va_start( va, format );
    media_log_va( MEDIA_LOGGING_LEVEL_ERROR, format_len, format, va );
    va_end( va );

    char error_buf[WIN32_MEDIA_ERROR_BUFFER_CAP] = {};
    memory_copy(
        error_buf, WIN32_MEDIA_ERROR_BUFFER_PREFIX,
        sizeof( WIN32_MEDIA_ERROR_BUFFER_PREFIX ) );
    usize len = sizeof(WIN32_MEDIA_ERROR_BUFFER_PREFIX) - 1;

    DWORD dwFlags =
        FORMAT_MESSAGE_FROM_SYSTEM    |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK;

    DWORD win32_format_len = FormatMessageA(
        dwFlags, NULL, error_code, 0,
        error_buf + len,
        WIN32_MEDIA_ERROR_BUFFER_CAP - len, NULL );
    len += win32_format_len;
    if(
        len + (sizeof(WIN32_MEDIA_ERROR_BUFFER_SUFFIX) - 1) >
        WIN32_MEDIA_ERROR_BUFFER_CAP
    ) {
        const usize sub = sizeof("..") + (sizeof(WIN32_MEDIA_ERROR_BUFFER_SUFFIX) - 1);
        usize copy_pos = len - sub;
        memory_copy(
            error_buf + copy_pos,
            "..." WIN32_MEDIA_ERROR_BUFFER_SUFFIX, sub );
        len += sub;
    } else {
        memory_copy(
            error_buf + len,
            WIN32_MEDIA_ERROR_BUFFER_SUFFIX,
            sizeof(WIN32_MEDIA_ERROR_BUFFER_SUFFIX) - 1 );
        len += sizeof(WIN32_MEDIA_ERROR_BUFFER_SUFFIX) - 1;
    }

    media_log(
        MEDIA_LOGGING_LEVEL_ERROR, win32_format_len,
        error_buf );
}

#undef WIN32_MEDIA_ERROR_BUFFER_PREFIX
#undef WIN32_MEDIA_ERROR_BUFFER_SUFFIX
#undef WIN32_MEDIA_ERROR_BUFFER_CAP

#endif /* Platform Windows */

