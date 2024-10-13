/**
 * @file   common.c
 * @brief  Media Windows Common.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 10, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "impl/win32/common.h"

#include "media/lib.h"
#include "media/input/keyboard.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <initguid.h>

struct Win32State* global_win32_state = NULL;
_Bool global_win32_cursor_hidden   = false;
HCURSOR global_win32_cursors[CURSOR_TYPE_COUNT];

// NOTE(alicia): always request discrete graphics.
__declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001;
__declspec(dllexport) int   AmdPowerXpressRequestHighPerformance = 1;

#define def( fn )\
fn##FN* in_##fn = NULL

def( MessageBoxW );
def( RegisterClassExW );
def( UnregisterClassW );
def( CreateWindowExW );
def( DestroyWindow );
def( DefWindowProcW );
def( AdjustWindowRectEx );
def( GetClientRect );
def( GetDC );
def( ReleaseDC );
def( ShowWindow );
def( PeekMessageW );
def( TranslateMessage );
def( DispatchMessageW );
def( SetWindowTextW );
def( SetWindowPos );
def( MonitorFromPoint );
def( MonitorFromWindow );
def( GetMonitorInfoW );
def( GetWindowPlacement );
def( SetWindowPlacement );
def( SetCursor );
def( LoadCursorA );
def( ShowCursor );
def( ClientToScreen );
def( SetCursorPos );
def( MapVirtualKeyW );
def( GetForegroundWindow );
def( GetWindowThreadProcessId );
def( PostMessageW );
def( GetCursorPos );
def( GetKeyState );
def( ToUnicode );
def( ScreenToClient );

#if defined(MEDIA_ARCH_64_BIT)
def( SetWindowLongPtrW );
def( GetWindowLongPtrW );
#else
SetWindowLongWFN* in_SetWindowLongW = NULL;
GetWindowLongWFN* in_GetWindowLongW = NULL;
#endif

def( GetStockObject );

def( CoInitialize );
def( CoCreateInstance );
def( CoTaskMemFree );
def( CoUninitialize );
def( PropVariantClear );

def( DwmSetWindowAttribute );

attr_internal void win32_unload_modules(void) {
    if( !global_win32_state ) {
        return;
    }
    int module_count =
        sizeof(global_win32_state->modules) /
        sizeof(HMODULE);
    for( int i = 0; i < module_count; ++i ) {
        if( global_win32_state->modules.array[i] ) {
            CloseHandle( global_win32_state->modules.array[i] );
        }
    }
}

attr_media_api uintptr_t media_lib_query_memory_requirement(void) {
    return sizeof(struct Win32State);
}
attr_media_api _Bool media_lib_initialize(
    MediaLoggingLevel       log_level,
    MediaLoggingCallbackFN* opt_log_callback,
    void*                   opt_log_callback_params,
    void*                   buffer
) {
    media_lib_set_logging_level( log_level );
    media_lib_set_logging_callback( opt_log_callback, opt_log_callback_params );

    if( !buffer ) {
        win32_error( "media_lib_initialize: buffer provided is null!" );
        return false;
    }

    global_win32_state = buffer;

    HMODULE module = GetModuleHandleW(0);

    #define open( lib ) do {\
        global_win32_state->modules.lib = LoadLibraryA( #lib ".DLL" );\
        if( !global_win32_state->modules.lib ) {\
            win32_error( "media_lib_initialize: failed to open library " #lib "!");\
            win32_unload_modules();\
            return false;\
        }\
    } while(0)
    #define load( lib, fn ) do {\
        fn = (fn##FN*)GetProcAddress( global_win32_state->modules.lib, #fn );\
        if( !fn ) {\
            win32_error( "media_lib_initialize: failed to load " #fn " from " #lib "!");\
            win32_unload_modules();\
            return false;\
        }\
    } while(0)

    open( USER32 );
    open( GDI32 );
    open( OLE32 );
    open( DWMAPI );

    load( USER32, MessageBoxW );
    load( USER32, RegisterClassExW );
    load( USER32, UnregisterClassW );
    load( USER32, CreateWindowExW );
    load( USER32, DestroyWindow );
    load( USER32, DefWindowProcW );
    load( USER32, AdjustWindowRectEx );
    load( USER32, GetClientRect );
    load( USER32, GetDC );
    load( USER32, ReleaseDC );
    load( USER32, ShowWindow );
    load( USER32, PeekMessageW );
    load( USER32, TranslateMessage );
    load( USER32, DispatchMessageW );
    load( USER32, SetWindowTextW );
    load( USER32, SetWindowPos );
    load( USER32, MonitorFromPoint );
    load( USER32, MonitorFromWindow );
    load( USER32, GetMonitorInfoW );
    load( USER32, GetWindowPlacement );
    load( USER32, SetWindowPlacement );
    load( USER32, SetCursor );
    load( USER32, LoadCursorA );
    load( USER32, ShowCursor );
    load( USER32, ClientToScreen );
    load( USER32, SetCursorPos );
    load( USER32, MapVirtualKeyW );
    load( USER32, GetForegroundWindow );
    load( USER32, GetWindowThreadProcessId );
    load( USER32, PostMessageW );
    load( USER32, GetCursorPos );
    load( USER32, GetKeyState );
    load( USER32, ToUnicode );
    load( USER32, ScreenToClient );

#if defined(MEDIA_ARCH_64_BIT)
    load( USER32, SetWindowLongPtrW );
    load( USER32, GetWindowLongPtrW );
#else
    in_SetWindowLongW =
        (SetWindowLongWFN*)GetProcAddress(
            global_win32_state->modules.USER32, "SetWindowLongW" );
    if( !in_SetWindowLongW ) {
        win32_error( "failed to load SetWindowLongW from USER32!");
        win32_unload_modules();
        return false;
    }

    in_GetWindowLongW =
        (GetWindowLongWFN*)GetProcAddress(
            global_win32_state->modules.USER32, "GetWindowLongW" );
    if( !in_GetWindowLongW ) {
        win32_error( "failed to load GetWindowLongW from USER32!");
        win32_unload_modules();
        return false;
    }
#endif

    load( GDI32, GetStockObject );

    load( OLE32, CoInitialize );
    load( OLE32, CoCreateInstance );
    load( OLE32, CoTaskMemFree );
    load( OLE32, CoUninitialize );
    load( OLE32, PropVariantClear );

    load( DWMAPI, DwmSetWindowAttribute );

    if( !CoCheck( CoInitialize( NULL ) ) ) {
        win32_error( "media_lib_initialize: failed to initialize COM!" );
        win32_unload_modules();
        return false;
    }

    WNDCLASSEXW default_window_class;
    memset( &default_window_class, 0, sizeof(default_window_class) );
    default_window_class.cbSize        = sizeof(default_window_class);
    default_window_class.lpszClassName = WIN32_DEFAULT_WINDOW_CLASS;
    default_window_class.hInstance     = module;
    default_window_class.hbrBackground = GetStockBrush( BLACK_BRUSH );
    default_window_class.lpfnWndProc   = win32_winproc;

    if( !RegisterClassExW( &default_window_class ) ) {
        win32_error( "media_lib_initialize: failed to register default window class!" );
        win32_unload_modules();
        CoUninitialize();
        return false;
    }

    global_win32_cursors[CURSOR_TYPE_ARROW]      = LoadCursorA( NULL, IDC_ARROW );
    global_win32_cursors[CURSOR_TYPE_HAND]       = LoadCursorA( NULL, IDC_HAND );
    global_win32_cursors[CURSOR_TYPE_TEXT]       = LoadCursorA( NULL, IDC_IBEAM );
    global_win32_cursors[CURSOR_TYPE_WAIT]       = LoadCursorA( NULL, IDC_WAIT );
    global_win32_cursors[CURSOR_TYPE_ARROW_WAIT] = LoadCursorA( NULL, IDC_APPSTARTING );
    global_win32_cursors[CURSOR_TYPE_SIZE_ALL]   = LoadCursorA( NULL, IDC_SIZEALL );
    global_win32_cursors[CURSOR_TYPE_SIZE_V]     = LoadCursorA( NULL, IDC_SIZENS );
    global_win32_cursors[CURSOR_TYPE_SIZE_H]     = LoadCursorA( NULL, IDC_SIZEWE );
    global_win32_cursors[CURSOR_TYPE_SIZE_L]     = LoadCursorA( NULL, IDC_SIZENWSE );
    global_win32_cursors[CURSOR_TYPE_SIZE_R]     = LoadCursorA( NULL, IDC_SIZENESW );

    _Bool caps   = GetKeyState( VK_CAPITAL ) & 0x0001;
    _Bool scroll = GetKeyState( VK_SCROLL )  & 0x0001;
    _Bool num    = GetKeyState( VK_NUMLOCK ) & 0x0001;

    global_win32_state->mod |= caps   ? KBMOD_CAPSLK : 0;
    global_win32_state->mod |= scroll ? KBMOD_SCRLK  : 0;
    global_win32_state->mod |= num    ? KBMOD_NUMLK  : 0;

    #undef open
    #undef load
    return true;
}
attr_media_api void media_lib_shutdown(void) {
    CoUninitialize();
    
    HMODULE module = GetModuleHandleA(0);

    UnregisterClassW( WIN32_DEFAULT_WINDOW_CLASS, module );

    win32_unload_modules();

    memset( global_win32_state, 0, sizeof(*global_win32_state) );
    global_win32_state = NULL;
}

attr_media_api void cursor_set_visible( _Bool is_visible ) {
    global_win32_cursor_hidden = !is_visible;
}

wchar_t* win32_utf8_to_ucs2_alloc(
    uint32_t utf8_len, const char* utf8, uint32_t* opt_out_len
) {
    int required_len = MultiByteToWideChar( CP_UTF8, 0, utf8, utf8_len, 0, 0 );
    wchar_t* res = HeapAlloc( 
        GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(wchar_t) * (required_len + 1));
    if( !res ) {
        return NULL;
    }

    MultiByteToWideChar( CP_UTF8, 0, utf8, utf8_len, res, required_len + 1 );
    if( opt_out_len ) {
        *opt_out_len = required_len;
    }
    return res;
}
MONITORINFO win32_monitor_info( HWND opt_hwnd ) {
    HMONITOR monitor = NULL;
    if( opt_hwnd ) {
        monitor = MonitorFromWindow( opt_hwnd, MONITOR_DEFAULTTONEAREST );
    } else {
        POINT pt;
        pt.x = 0;
        pt.y = 0;
        monitor = MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST );
    }

    MONITORINFO result;
    memset( &result, 0, sizeof(result) );
    result.cbSize = sizeof(result);

    GetMonitorInfoW( monitor, &result );
    return result;
}
HWND win32_get_focused_window(void) {
    HWND focused = GetForegroundWindow();
    DWORD pid;
    GetWindowThreadProcessId( focused, &pid );
    if( pid != GetCurrentProcessId() ) {
        return NULL;
    }
    return focused;
}

void win32_error_message_full(
    DWORD error_code, uint32_t message_len, const char* message
) {
    unused( error_code, message_len, message );
#if defined(MEDIA_ENABLE_LOGGING)
    if( error_code == ERROR_SUCCESS ) {
        return;
    }

    #define prefix "win32: "

    uint32_t buffer_size = message_len + sizeof(prefix) + 256 + 8;
    uint32_t buffer_len  = 0;

    char* buffer = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, buffer_size );
    memcpy( buffer, prefix, sizeof(prefix) );
    buffer_len += sizeof(prefix) - 1;

    memcpy( buffer + buffer_len, message, message_len );
    buffer_len += message_len;

    buffer[buffer_len++] = ' ';
    buffer[buffer_len++] = '"';

    int format_len = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, error_code, 0, buffer + buffer_len, buffer_size - buffer_len, 0 );

    buffer_len += format_len - 2;
    buffer[buffer_len++] = '"';

    media_log( MEDIA_LOGGING_LEVEL_ERROR, buffer_len, buffer );

    HeapFree( GetProcessHeap(), 0, buffer );
    #undef prefix
#endif /* MEDIA_ENABLE_LOGGING */
}

#undef def
#endif /* Platform Windows */
