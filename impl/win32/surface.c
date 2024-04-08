/**
 * @file   surface.c
 * @brief  media/surface,prompt,cursor win32 implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "impl/win32/common.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "core/prelude.h"
#include "core/memory.h"
#include "core/sync.h"
#include "core/math/macros.h"

#include "media/internal/logging.h"
#include "media/surface.h"
#include "media/prompt.h"
#include "media/mouse.h"

#include "impl/win32/surface.h"

#include <cderr.h>
#include <hidusage.h>

struct Win32MessageProcData {
    BYTE  kb[256];
    InputMouseButton mb;
    HWND active;
};
struct Win32KeyWParam {
#if defined(CORE_ARCH_64_BIT)
    u32 ___padding;
#endif
    u16 keycode;
    b16 is_down;
};
#define win32_make_key_wparam( _keycode, _is_down )\
    rcast( WPARAM, (&(struct Win32KeyWParam){ .keycode=_keycode, .is_down=_is_down }) )
#define win32_get_key_wparam( wparam ) rcast( struct Win32KeyWParam, &(wparam) )

struct Win32TextWParam {
#if defined(CORE_ARCH_64_BIT)
    u32 ___padding;
#endif
    u16 vk;
    u16 scan;
};
#define win32_make_text_wparam( _vk, _scan )\
    rcast( WPARAM, (&(struct Win32TextWParam){ .vk=_vk, .scan=_scan }) )
#define win32_get_text_wparam( wparam ) rcast( struct Win32TextWParam, &(wparam) )
#define win32_make_text_lparam( keyboard_state ) ((LPARAM)(keyboard_state))
#define win32_get_text_lparam( lparam ) ((const BYTE*)(lparam))

struct Win32MousePositionParam {
#if defined(CORE_ARCH_64_BIT)
    u32 ___padding;
#endif
    i32 v;
};
#define win32_make_mouse_pos_wparam( x )\
    rcast( WPARAM, (&(struct Win32MousePositionParam){ .v=x }) )
#define win32_make_mouse_pos_lparam( y )\
    rcast( LPARAM, (&(struct Win32MousePositionParam){ .v=y }) )
#define win32_get_mouse_pos( param )\
    rcast( struct Win32MousePositionParam, &(param) )

struct Win32MouseButtonWParam {
#if defined(CORE_ARCH_64_BIT)
    u32 ___padding;
#endif
    u8 state;
    u8 delta;
    i8 scroll;
    u8 scroll_hor;
};
#define win32_make_mouse_button_wparam( _state, _delta, _scroll, _scroll_hor )\
    rcast( WPARAM, (&(struct Win32MouseButtonWParam){\
        .state=_state, .delta=_delta, .scroll=_scroll, .scroll_hor=_scroll_hor }) )
#define win32_get_mouse_button_wparam( wparam )\
    rcast( struct Win32MouseButtonWParam, &(wparam) )

attr_internal InputKeycode win32_vk_to_keycode( DWORD vk );
attr_internal MONITORINFO win32_get_monitor_info( HWND hwnd );
attr_internal usize win32_filter_list_memory_requirement(
    const MediaPromptFileFilterList* list );
attr_internal void win32_filter_list_to_string(
    const MediaPromptFileFilterList* list, char* buf );
attr_internal void win32_format_commdlg_error( DWORD error );

attr_media_api usize media_surface_query_memory_requirement(void) {
    return sizeof( struct Win32Surface );
}
attr_media_api b32 media_surface_create(
    String name, i32 x, i32 y, i32 w, i32 h,
    MediaSurfaceCreateFlags flags, MediaSurfaceCallbackFN* opt_callback,
    void* opt_callback_params, MediaSurface* opt_parent, MediaSurface* out_surface
) {
    HWND parent = NULL;
    if( opt_parent ) {
        surface_to_win32( opt_parent );
        parent = surface->hwnd;
    }

    b32 opengl  = bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_OPENGL );
    b32 vulkan  = bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_VULKAN );
    b32 directx = bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_DIRECTX );

    if( (opengl && vulkan) || (vulkan && directx) || (opengl && directx) ) {
        media_error(
            "surface: cannot combine multiple graphics backend create flags!" );
        return false;
    }

    struct Win32Surface* surface = out_surface;
    if( !surface ) {
        win32_error( "out_surface must point to valid memory!" );
        return false;
    }

    surface->callback        = opt_callback;
    surface->callback_params = opt_callback_params;
    surface->flags           = flags;
    surface->cursor          = MEDIA_CURSOR_ARROW;

    HMODULE module = GetModuleHandleA( NULL );

    /* copy name */
    if( name.len ) {
        usize max_copy = name.len;
        if( max_copy >= WIN32_SURFACE_NAME_CAP ) {
            max_copy = WIN32_SURFACE_NAME_CAP - 1;
        }
        memory_copy( surface->name, name.cc, max_copy );
        surface->name_len = max_copy;
    }

    surface->dwExStyle = WS_EX_OVERLAPPEDWINDOW;
    if( bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_RESIZEABLE ) ) {
        surface->dwStyle = WS_OVERLAPPEDWINDOW;
    } else {
        surface->dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    }

    if( bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_NO_MINIMIZE ) ) {
        surface->dwStyle = bitfield_clear( surface->dwStyle, WS_MINIMIZEBOX );
    }
    if( bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_NO_MAXIMIZE ) ) {
        surface->dwStyle = bitfield_clear( surface->dwStyle, WS_MAXIMIZEBOX );
    }

    surface->x = x;
    surface->y = y;
    surface->w = w;
    surface->h = h;

    RECT rect   = {};
    rect.right  = w;
    rect.bottom = h;

    if( !AdjustWindowRectEx(
        &rect, surface->dwStyle, false, surface->dwExStyle
    ) ) {
        win32_error( "failed to get window rect!" );
        goto media_surface_create_failed;
    }

    surface->hwnd = CreateWindowExA(
        surface->dwExStyle, global_win32_state->def_wndclass.lpszClassName,
        surface->name, surface->dwStyle, surface->x, surface->y,
        rect.right - rect.left, rect.bottom - rect.top,
        parent, NULL, module, NULL );

    if( !surface->hwnd ) {
        win32_error( "failed to create window!" );
        goto media_surface_create_failed;
    }

    surface->hdc = GetDC( surface->hwnd );

    SetWindowLongPtrA( surface->hwnd, GWLP_USERDATA, (LONG_PTR)surface );

    #define DWMWA_USE_IMMERSIVE_DARK_MODE (20)
    if( bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_DARK_MODE ) ) {
        // NOTE(alicia): no error checking required.
        BOOL value = TRUE;
        DwmSetWindowAttribute(
            surface->hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value) );

    }
    #undef DWMWA_USE_IMMERSIVE_DARK_MODE

    surface->state |= bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_HIDDEN ) ?
        WIN32_SURFACE_STATE_IS_HIDDEN : 0;
    if( !bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_HIDDEN ) ) {
        ShowWindow( surface->hwnd, SW_SHOW );
        surface->state |= WIN32_SURFACE_STATE_IS_FOCUSED;
    }

    if( bitfield_check( flags, MEDIA_SURFACE_CREATE_FLAG_FULLSCREEN ) ) {
        media_surface_set_mode(
            (MediaSurface*)surface, MEDIA_SURFACE_MODE_FULLSCREEN );
    }
    return true;
media_surface_create_failed:
    return false;
}
attr_media_api void media_surface_destroy( MediaSurface* in_surface ) {
    surface_to_win32( in_surface );
    read_write_fence();

    ReleaseDC( surface->hwnd, surface->hdc );

    DestroyWindow( surface->hwnd );
}
attr_media_api void media_surface_pump_events( MediaSurface* in_surface ) {
    surface_to_win32( in_surface );

    b32 is_active    = surface->hwnd == win32_get_active_window();
    b32 active_state = bitfield_check(
        surface->state, WIN32_SURFACE_STATE_IS_FOCUSED );

    HWND cursor_lock = global_win32_state->cursor_lock;

    if( is_active != active_state ) {
        if( is_active ) {
            surface->state =
                bitfield_set( surface->state, WIN32_SURFACE_STATE_IS_FOCUSED );
            if( cursor_lock == surface->hwnd ) {
                RECT client;
                if( GetClientRect( surface->hwnd, &client ) ) {
                    SetLastError( 0 );
                    int res = MapWindowPoints(
                        surface->hwnd, NULL, (POINT*)&client, 2 );

                    if( res || !GetLastError() ) {
                        ClipCursor( &client );
                    }
                }
            }
        } else {
            surface->state =
                bitfield_clear( surface->state, WIN32_SURFACE_STATE_IS_FOCUSED );
            if( cursor_lock == surface->hwnd ) {
                ClipCursor( NULL );
            }
        }

        MediaSurfaceCallbackData data;
        data.type         = MEDIA_SURFACE_CALLBACK_TYPE_FOCUS;
        data.focus.gained = is_active;

        if( surface->callback ) {
            surface->callback( surface, &data, surface->callback_params );
        }
    }

    MSG message = {};
    while( PeekMessageA( &message, surface->hwnd, 0, 0, PM_REMOVE ) ) {
        DispatchMessageA( &message );
    }
}
attr_media_api void media_surface_set_callback(
    MediaSurface* in_surface, MediaSurfaceCallbackFN* callback,
    void* opt_callback_params
) {
    surface_to_win32( in_surface );
    surface->callback        = callback;
    surface->callback_params = opt_callback_params;
}
attr_media_api void media_surface_clear_callback( MediaSurface* in_surface ) {
    surface_to_win32( in_surface );
    surface->callback        = NULL;
    surface->callback_params = NULL;
}
attr_media_api String media_surface_query_name( const MediaSurface* in_surface ) {
    surface_to_win32( in_surface );
    String result;
    result.cc  = surface->name;
    result.len = surface->name_len;
    return result;
}
attr_media_api void media_surface_set_name( MediaSurface* in_surface, String name ) {
    if( !name.len ) {
        return;
    }
    surface_to_win32( in_surface );

    memory_zero( surface->name, surface->name_len );
    usize max_copy = min( name.len, WIN32_SURFACE_NAME_CAP );
    memory_copy( surface->name, name.cc, max_copy );
    surface->name_len = max_copy;
    if( surface->name_len == WIN32_SURFACE_NAME_CAP ) {
        surface->name[WIN32_SURFACE_NAME_CAP - 1] = 0;
    }

    SetWindowTextA( surface->hwnd, surface->name );
}
attr_media_api void media_surface_query_position(
    const MediaSurface* in_surface, i32* out_x, i32* out_y
) {
    surface_to_win32( in_surface );
    *out_x = surface->x;
    *out_y = surface->y;
}
attr_media_api void media_surface_set_position(
    MediaSurface* in_surface, i32 x, i32 y
) {
    surface_to_win32( in_surface );
    // NOTE(alicia): surface's x,y are updated in win32_winproc
    SetWindowPos(
        surface->hwnd, NULL,
        x, y, 0, 0,
        SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
}
attr_media_api void media_surface_query_dimensions(
    const MediaSurface* in_surface, i32* out_w, i32* out_h
) {
    surface_to_win32( in_surface );
    *out_w = surface->w;
    *out_h = surface->h;
}
attr_media_api void media_surface_set_dimensions(
    MediaSurface* in_surface, i32 w, i32 h
) {
    surface_to_win32( in_surface );
    // NOTE(alicia): surface's w,h are updated in win32_winproc
    
    if(
        bitfield_check( surface->state, WIN32_SURFACE_STATE_IS_FULLSCREEN ) ||
        ( surface->w == w && surface->h == h )
    ) {
        return;
    }

    RECT rect = {};
    rect.right  = w;
    rect.bottom = h;

    DWORD dwStyle   = (DWORD)GetWindowLongPtrA( surface->hwnd, GWL_STYLE );
    DWORD dwExStyle = (DWORD)GetWindowLongPtrA( surface->hwnd, GWL_EXSTYLE );
    if( AdjustWindowRectEx(
        &rect, dwStyle, FALSE, dwExStyle
    ) ) {
        SetWindowPos(
            surface->hwnd, NULL,
            0, 0, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
    }

}
attr_media_api b32 media_surface_query_hidden( const MediaSurface* in_surface ) {
    surface_to_win32( in_surface );
    return bitfield_check( surface->state, WIN32_SURFACE_STATE_IS_HIDDEN );
}
attr_media_api void media_surface_set_hidden(
    MediaSurface* in_surface, b32 is_hidden
) {
    surface_to_win32( in_surface );
    // NOTE(alicia): if it's already showing/hidden, don't do anything.
    if( bitfield_check( surface->state, WIN32_SURFACE_STATE_IS_HIDDEN ) == is_hidden ) {
        return;
    }
    if( ShowWindow( surface->hwnd, is_hidden ? SW_HIDE : SW_SHOW ) ) {
        surface->state = is_hidden ?
            bitfield_set( surface->state, WIN32_SURFACE_STATE_IS_HIDDEN ) :
            bitfield_clear( surface->state, WIN32_SURFACE_STATE_IS_HIDDEN );
    }
}
attr_media_api MediaSurfaceMode media_surface_query_mode(
    const MediaSurface* in_surface
) {
    surface_to_win32( in_surface );
    if( bitfield_check( surface->state, WIN32_SURFACE_STATE_IS_FULLSCREEN ) ) {
        return MEDIA_SURFACE_MODE_FULLSCREEN;
    } else {
        return MEDIA_SURFACE_MODE_WINDOWED;
    }
}
attr_media_api void media_surface_set_mode(
    MediaSurface* in_surface, MediaSurfaceMode mode
) {
    b32 is_fullscreen = U32_MAX;
    switch( mode ) {
        case MEDIA_SURFACE_MODE_WINDOWED: {
            is_fullscreen = false;
        } break;
        case MEDIA_SURFACE_MODE_FULLSCREEN: {
            is_fullscreen = true;
        } break;
    }
    if( is_fullscreen == U32_MAX ) {
        String mode_string;
        mode_string.cc = media_surface_mode_to_string( mode, &mode_string.len );
        media_warn( "[WIN32] surface mode {s} is not supported!", mode_string );
        return;
    }

    surface_to_win32( in_surface );
    b32 current = bitfield_check( surface->state, WIN32_SURFACE_STATE_IS_FULLSCREEN );
    if( current == is_fullscreen ) {
        return;
    }

    if( is_fullscreen ) {
        memory_zero( &surface->placement, sizeof(surface->placement) );
        surface->placement.length = sizeof( surface->placement );
        GetWindowPlacement( surface->hwnd, &surface->placement );

        MONITORINFO minfo = win32_get_monitor_info( surface->hwnd );

        SetWindowLongPtrA( surface->hwnd, GWL_STYLE,   WS_POPUP );
        SetWindowLongPtrA( surface->hwnd, GWL_EXSTYLE, 0 );

        i32 x = minfo.rcMonitor.left;
        i32 y = minfo.rcMonitor.top;
        i32 w = minfo.rcMonitor.right - minfo.rcMonitor.left;
        i32 h = minfo.rcMonitor.bottom - minfo.rcMonitor.top;

        SetWindowPos(
            surface->hwnd, HWND_TOP, x, y, w, h,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
    } else {
        SetWindowLongPtrA( surface->hwnd, GWL_STYLE,   surface->dwStyle );
        SetWindowLongPtrA( surface->hwnd, GWL_EXSTYLE, surface->dwExStyle );

        SetWindowPlacement( surface->hwnd, &surface->placement );

        SetWindowPos(
            surface->hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
    }

    surface->state = is_fullscreen ?
        bitfield_set( surface->state, WIN32_SURFACE_STATE_IS_FULLSCREEN ) :
        bitfield_clear( surface->state, WIN32_SURFACE_STATE_IS_FULLSCREEN );
}

attr_media_api MediaMessageBoxResult media_message_box_blocking(
    MediaSurface* opt_parent, String title, String message,
    MediaMessageBoxType type, MediaMessageBoxOptions options
) {
    HWND parent = NULL;
    if( opt_parent ) {
        surface_to_win32( opt_parent );
        parent = surface->hwnd;
    }

    UINT uType = MB_SETFOREGROUND;

    switch( type ) {
        case MEDIA_MESSAGE_BOX_TYPE_INFO: {
            uType |= MB_ICONINFORMATION;
        } break;
        case MEDIA_MESSAGE_BOX_TYPE_WARN: {
            uType |= MB_ICONWARNING;
        } break;
        case MEDIA_MESSAGE_BOX_TYPE_ERROR: {
            uType |= MB_ICONERROR; 
        } break;
    }

    switch( options ) {
        case MEDIA_MESSAGE_BOX_OPTIONS_OK: {
            uType |= MB_OK;
        } break;
        case MEDIA_MESSAGE_BOX_OPTIONS_OK_CANCEL: {
            uType |= MB_OKCANCEL;
        } break;
        case MEDIA_MESSAGE_BOX_OPTIONS_YES_NO: {
            uType |= MB_YESNO;
        } break;
    }

    if( parent ) {
        uType |= MB_APPLMODAL;
    }

    int result = MessageBoxA( parent, message.cc, title.cc, uType );

    switch( result ) {
        case IDYES: if( options == MEDIA_MESSAGE_BOX_OPTIONS_YES_NO ) {
            return MEDIA_MESSAGE_BOX_RESULT_YES_PRESSED;
        } else {
            return MEDIA_MESSAGE_BOX_RESULT_ERROR;
        }
        case IDNO: if( options == MEDIA_MESSAGE_BOX_OPTIONS_YES_NO ) {
            return MEDIA_MESSAGE_BOX_RESULT_NO_PRESSED;
        } else {
            return MEDIA_MESSAGE_BOX_RESULT_ERROR;
        }
        case IDOK: if(
            options == MEDIA_MESSAGE_BOX_OPTIONS_OK ||
            options == MEDIA_MESSAGE_BOX_OPTIONS_OK_CANCEL
        ) {
            return MEDIA_MESSAGE_BOX_RESULT_OK_PRESSED;
        } else {
            return MEDIA_MESSAGE_BOX_RESULT_ERROR;
        }
        case IDCANCEL: if( options == MEDIA_MESSAGE_BOX_OPTIONS_OK_CANCEL ) {
            return MEDIA_MESSAGE_BOX_RESULT_CANCEL_PRESSED;
        } else {
            return MEDIA_MESSAGE_BOX_RESULT_ERROR;
        }
        default: return MEDIA_MESSAGE_BOX_RESULT_ERROR;
    }
}
attr_media_api MediaPromptFileOpenResult media_prompt_file_open_blocking(
    MediaSurface* opt_parent, String opt_prompt_title, Path initial_directory,
    MediaPromptFileFilterList* opt_filters, PathBuf* out_path
) {
    if( !out_path->cap || out_path->cap < MAX_PATH ) {
        media_error( "out_path must be capable of holding at least 260 bytes!" );
        return MEDIA_PROMPT_FILE_OPEN_RESULT_ERROR_OUT_PATH_TOO_SMALL;
    }

    HWND parent = NULL;
    if( opt_parent ) {
        surface_to_win32( opt_parent );
        parent = surface->hwnd;
    }

    String prompt_title = string_text( "Open a File." );
    if( opt_prompt_title.cc && opt_prompt_title.len ) {
        prompt_title = opt_prompt_title;
    }

    usize text_buffer_cap =
        prompt_title.len + 1 + initial_directory.len + 1;
    if( opt_filters ) {
        text_buffer_cap += win32_filter_list_memory_requirement( opt_filters );
    }
    char* text_buffer = memory_alloc( text_buffer_cap );
    if( !text_buffer ) {
        media_error(
            "failed to allocate {f,.2,m} "
            "for media_prompt_file_open_blocking text buffer!",
            (f64)text_buffer_cap );
        return MEDIA_PROMPT_FILE_OPEN_RESULT_ERROR_OUT_OF_MEMORY;
    }

    char* dialog_title = text_buffer;
    memory_copy( dialog_title, prompt_title.cc, prompt_title.len );

    char* dir = NULL;
    char* filter;
    if( initial_directory.cc && initial_directory.len ) {
        dir = dialog_title + (prompt_title.len + 1);
        memory_copy( dir, initial_directory.cc, initial_directory.len );

        filter = dir + initial_directory.len + 1;
    } else {
        filter = dialog_title + (prompt_title.len + 1);
    }

    if( opt_filters ) {
        win32_filter_list_to_string( opt_filters, filter );
    } else {
        filter = NULL;
    }

    OPENFILENAMEA open_file_name   = {};
    open_file_name.lStructSize     = sizeof(open_file_name);
    open_file_name.hwndOwner       = parent;
    open_file_name.lpstrTitle      = dialog_title;
    open_file_name.lpstrFile       = out_path->c;
    open_file_name.nMaxFile        = out_path->cap;

    open_file_name.lpstrFilter     = filter;
    open_file_name.nFilterIndex    = 0;
    open_file_name.lpstrFileTitle  = 0;
    open_file_name.lpstrInitialDir = dir;
    open_file_name.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    BOOL res = GetOpenFileNameA( &open_file_name );
    memory_free( text_buffer, text_buffer_cap );

    if( res == FALSE ) {
        DWORD error = CommDlgExtendedError();
        if( !error ) {
            return MEDIA_PROMPT_FILE_OPEN_RESULT_NO_FILE_SELECT;
        }
        win32_format_commdlg_error( error );
        return MEDIA_PROMPT_FILE_OPEN_RESULT_ERROR_PROMPT;
    }

    out_path->len = asciiz_len( out_path->cc );
    return MEDIA_PROMPT_FILE_OPEN_RESULT_SUCCESS;
}
attr_media_api void media_cursor_set( MediaSurface* in_surface, MediaCursor cursor ) {
    surface_to_win32( in_surface );
    surface->cursor = cursor % MEDIA_CURSOR_COUNT;
}
attr_media_api void media_cursor_center( MediaSurface* in_surface ) {
    surface_to_win32( in_surface );

    POINT pos = {};
    pos.x = surface->w / 2;
    pos.y = surface->h / 2;
    if( ClientToScreen( surface->hwnd, &pos ) ) {
        SetCursorPos( pos.x, pos.y );
    }
}
attr_media_api void media_cursor_lock( MediaSurface* in_surface, b32 is_locked ) {
    surface_to_win32( in_surface );
    global_win32_state->cursor_lock = is_locked ? surface->hwnd : NULL;
    if( is_locked ) {
        RECT client;
        if( GetClientRect( surface->hwnd, &client ) ) {
            if( MapWindowPoints( surface->hwnd, NULL, (POINT*)&client, 2 ) ) {
                ClipCursor( &client );
            }
        }
    } else {
        ClipCursor( NULL );
    }
}
attr_media_api void media_cursor_set_visible( b32 is_visible ) {
    ShowCursor( is_visible ? TRUE : FALSE );
}
attr_media_api InputKeymod media_keyboard_query_mod(void) {
    return global_win32_state->keymod;
}
attr_media_api b32 media_keyboard_query_key( InputKeycode key ) {
    return packed_bool_get( global_win32_state->keys, key );
}

// NOTE(alicia): internals

LRESULT win32_winproc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    struct Win32Surface* surface =
        (struct Win32Surface*)GetWindowLongPtrA( hwnd, GWLP_USERDATA );
    // no message can be handled before surface arrives.
    if( !surface ) {
        goto win32_winproc_skip;
    }
    MediaCursor cursor = surface->cursor;
    InputKeymod mod    = global_win32_state->keymod;
    b8* keys           = (b8*)global_win32_state->keys;
    if(
        bitfield_check( mod, INPUT_KEYMOD_ALT ) &&
        packed_bool_get( keys, INPUT_KEYCODE_F4 )
    ) {
        PostMessageA( hwnd, WM_CLOSE, 0, 0 );
    }

    // handle messages that don't need callback.
    switch( msg ) {
        case WM_SETCURSOR: {
            MediaCursor cursor_to_set = MEDIA_CURSOR_ARROW;
            switch( LOWORD( lparam ) ) {
                case HTRIGHT:
                case HTLEFT: {
                    cursor_to_set = MEDIA_CURSOR_SIZE_H;
                } break;
                case HTTOP:
                case HTBOTTOM: {
                    cursor_to_set = MEDIA_CURSOR_SIZE_V;
                } break;
                case HTBOTTOMRIGHT:
                case HTTOPLEFT: {
                    cursor_to_set = MEDIA_CURSOR_SIZE_L;
                } break;
                case HTBOTTOMLEFT:
                case HTTOPRIGHT: {
                    cursor_to_set = MEDIA_CURSOR_SIZE_R;
                } break;
                case HTCLIENT: {
                    cursor_to_set = cursor;
                } break;
                default: break;
            }
            SetCursor( global_win32_state->cursors[cursor_to_set] );

        } return TRUE;
        default: break;
    }

    if( !surface->callback ) {
        goto win32_winproc_skip;
    }
    // handle messages that require callback.
    #define cb() surface->callback( surface, &data, surface->callback_params )

    MediaSurfaceCallbackData data = {};
    switch( msg ) {
        case WM_CLOSE: {
            data.type = MEDIA_SURFACE_CALLBACK_TYPE_CLOSE;
            cb();
        } return 0;
        case WM_WINDOWPOSCHANGED: {
            WINDOWPOS* pos = (WINDOWPOS*)lparam;
            b32 no_size = bitfield_check( pos->flags, SWP_NOSIZE );
            b32 no_move = bitfield_check( pos->flags, SWP_NOMOVE );

            if( !no_move ) {
                i32 old_x = surface->x;
                i32 old_y = surface->y;
                i32 new_x = pos->x;
                i32 new_y = pos->y;

                if( !(old_x == new_x && old_y == new_y) ) {
                    data.type = MEDIA_SURFACE_CALLBACK_TYPE_POSITION;
                    data.pos.old_x = old_x;
                    data.pos.old_y = old_y;
                    data.pos.new_x = new_x;
                    data.pos.new_y = new_y;

                    surface->x = new_x;
                    surface->y = new_y;
                    cb();
                }
            }

            if( !no_size ) {
                i32 old_w = surface->w;
                i32 old_h = surface->h;

                RECT client;
                if( GetClientRect( hwnd, &client ) ) {
                    i32 new_w = client.right  < 1 ? 1 : client.right;
                    i32 new_h = client.bottom < 1 ? 1 : client.bottom;

                    if( old_w != new_w || old_h != new_h ) {
                        data.type =
                            MEDIA_SURFACE_CALLBACK_TYPE_RESIZE;
                        data.resize.old_w = old_w;
                        data.resize.old_h = old_h;
                        data.resize.new_w = new_w;
                        data.resize.new_h = new_h;

                        surface->w = new_w;
                        surface->h = new_h;

                        cb();

                        if( global_win32_state->cursor_lock == hwnd ) {
                            SetLastError( 0 );
                            int res = MapWindowPoints(
                                hwnd, NULL, (POINT*)&client, 2 );

                            if( res || !GetLastError() ) {
                                ClipCursor( &client );
                            }
                        }
                    }

                }
            }

        } return 0;
        case WM_INPUT_KEYBOARD: {
            struct Win32KeyWParam w = win32_get_key_wparam( wparam );

            data.type        = MEDIA_SURFACE_CALLBACK_TYPE_KEY;
            data.key.code    = w.keycode;
            data.key.is_down = w.is_down;
            data.key.mod     = mod;

            cb();
        } return 0;
        case WM_INPUT_KEYBOARD_TEXT: {
            struct Win32TextWParam w = win32_get_text_wparam( wparam );
            const BYTE* l = win32_get_text_lparam( lparam );

            data.type = MEDIA_SURFACE_CALLBACK_TYPE_TEXT;
            int res = ToUnicode( w.vk, w.scan, l, (LPWSTR)data.text.utf8, 8, 0 );
            if( res > 0 ) {
                cb();
            }
        } return 0;
        case WM_INPUT_MOUSE_BUTTON: {
            struct Win32MouseButtonWParam b = win32_get_mouse_button_wparam( wparam );

            if( b.delta ) {
                data.type = MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_BUTTON;
                data.mouse_button.buttons_state   = b.state;
                data.mouse_button.buttons_changed = b.delta;

                cb();
            }

            if( b.scroll ) {
                memory_zero( &data, sizeof(data) );
                data.type =
                    MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_WHEEL;
                data.mouse_wheel.is_horizontal = b.scroll_hor;
                data.mouse_wheel.delta         = b.scroll;
                cb();
            }
        } return 0;
        case WM_INPUT_MOUSE_POSITION: {
            struct Win32MousePositionParam x, y;
            x = win32_get_mouse_pos( wparam );
            y = win32_get_mouse_pos( lparam );

            POINT pos;
            pos.x = x.v;
            pos.y = y.v;

            ScreenToClient( hwnd, &pos );
            if(
                (pos.x >= 0 && pos.x <= surface->w) &&
                (pos.y >= 0 && pos.y <= surface->h)
            ) {
                data.type = MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE;
                data.mouse_move.x = pos.x;
                data.mouse_move.y = surface->h - pos.y;

                cb();
            }
        } return 0;
        case WM_INPUT_MOUSE_POSITION_RELATIVE: {
            struct Win32MousePositionParam x, y;
            x = win32_get_mouse_pos( wparam );
            y = win32_get_mouse_pos( lparam );

            data.type = MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA;
            data.mouse_delta.x = x.v;
            data.mouse_delta.y = y.v;

            cb();
        } return 0;
        default: break;
    }

    #undef cb

win32_winproc_skip:
    return DefWindowProcA( hwnd, msg, wparam, lparam );
}
DWORD win32_message_window_thread( LPVOID in_params ) {
    unused( in_params );
    struct Win32MessageProcData data;
    memory_zero( &data, sizeof(data) );

    WNDCLASSEXA message_class   = global_win32_state->def_wndclass;
    message_class.lpszClassName = "LibMediaMessageWindowClass";
    message_class.lpfnWndProc   = win32_message_proc;

    if( !RegisterClassExA( &message_class ) ) {
        win32_error( "failed to register message window class!" );
        global_win32_state->msg_wnd_result = WIN32_MESSAGE_WINDOW_RESULT_ERROR;
        return -1;
    }

    /* Initialize caps-lock state */{
        b32 caps = GetKeyState( VK_CAPITAL ) & 0x0001;
        global_win32_state->keymod = caps ? INPUT_KEYMOD_CAPSLK : 0;

        data.kb[VK_CAPITAL] = caps ? 0x0001 : 0;

        global_win32_state->keymod |=
            (GetKeyState( VK_SCROLL ) & 0x0001) ? INPUT_KEYMOD_SCRLK : 0;
        global_win32_state->keymod |=
            (GetKeyState( VK_NUMLOCK ) & 0x0001) ? INPUT_KEYMOD_NUMLK : 0;
    }

    HMODULE module = GetModuleHandleA( NULL );
    HWND message_window = CreateWindowExA(
        0, message_class.lpszClassName, NULL,
        0, 0, 0, 0, 0, HWND_MESSAGE, NULL, module, NULL  );
    if( !message_window ) {
        win32_error( "failed to create message window!" );
        UnregisterClassA( message_class.lpszClassName, module );

        global_win32_state->msg_wnd_result = WIN32_MESSAGE_WINDOW_RESULT_ERROR;
        return -1;
    }

    SetWindowLongPtrA( message_window, GWLP_USERDATA, (LONG_PTR)&data );

    global_win32_state->msg_wnd = message_window;

    RAWINPUTDEVICE rid[2];
    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage     = HID_USAGE_GENERIC_MOUSE;
    rid[0].dwFlags     = RIDEV_INPUTSINK;
    rid[0].hwndTarget  = message_window;

    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
    rid[1].dwFlags     = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
    rid[1].hwndTarget  = message_window;
    if( RegisterRawInputDevices(
        rid, static_array_len(rid), sizeof( rid[0] )
    ) == FALSE ) {
        win32_error( "failed to register raw input devices!" );
        DestroyWindow( message_window );
        UnregisterClassA( message_class.lpszClassName, module );

        global_win32_state->msg_wnd_result = WIN32_MESSAGE_WINDOW_RESULT_ERROR;
        return -1;
    }

    global_win32_state->msg_wnd_result = WIN32_MESSAGE_WINDOW_RESULT_SUCCESS;
    read_write_fence();

    POINT mouse_pos = {};
    while( !global_win32_state->msg_wnd_exit ) {
        data.active = win32_get_active_window();

        if( data.active != NULL ) {
            POINT new_pos;
            GetCursorPos( &new_pos );
            if( !memory_cmp( &mouse_pos, &new_pos, sizeof(mouse_pos) ) ) {
                PostMessageA(
                    data.active, WM_INPUT_MOUSE_POSITION,
                    win32_make_mouse_pos_wparam( mouse_pos.x ),
                    win32_make_mouse_pos_lparam( mouse_pos.y ) );
            }

            mouse_pos = new_pos;
        }

        MSG message = {};
        // win32_message_proc should only process WM_INPUT messages.
        while( PeekMessageA(
            &message, message_window, WM_INPUT, WM_INPUT, PM_REMOVE
        ) ) {
            DispatchMessageA( &message );
        }

        XINPUT_STATE not_used;
        for( DWORD i = 0; i < XUSER_MAX_COUNT; ++i ) {
            DWORD result = XInputGetState( i, &not_used );
            global_win32_state->active_gamepads[i] = result == ERROR_SUCCESS;
        }

        // Sleep thread until more messages are available.
        // WaitMessage();
    }

    UnregisterClassA( message_class.lpszClassName, module );
    read_write_fence();
    global_win32_state->msg_wnd_result = WIN32_MESSAGE_WINDOW_RESULT_FINISHED;
    return 0;
}
LRESULT win32_message_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    struct Win32MessageProcData* data =
        (struct Win32MessageProcData*)GetWindowLongPtrA( hwnd, GWLP_USERDATA );
    if( msg != WM_INPUT || !data || !data->active ) {
        return DefWindowProcA( hwnd, msg, wparam, lparam );
    }

    UINT dwSize = sizeof(RAWINPUT);
    BYTE lpb[sizeof(RAWINPUT)];

    GetRawInputData(
        (HRAWINPUT)lparam, RID_INPUT,
        lpb, &dwSize, sizeof(RAWINPUTHEADER) );

    RAWINPUT* raw = (RAWINPUT*)lpb;
    switch( raw->header.dwType ) {
        case RIM_TYPEKEYBOARD: {
            RAWKEYBOARD* kb = &raw->data.keyboard;
            if( kb->MakeCode == KEYBOARD_OVERRUN_MAKE_CODE ) {
                break;
            }
            u16 scan = kb->MakeCode;

            b32 up, down;
            up   =  bitfield_check( kb->Flags, (1 << 0) );
            down = !bitfield_check( kb->Flags, (1 << 0) );

            u16 scan_translate = scan | ((kb->Flags & RI_KEY_E0) ? 0xE000 : 0);
            scan_translate |= (kb->Flags & RI_KEY_E1) ? 0xE100 : 0;

            u16 vk = kb->VKey;
            switch( vk ) {
                case VK_SHIFT:
                case VK_CONTROL:
                case VK_MENU:
                    vk = LOWORD( MapVirtualKeyA(
                        scan_translate, MAPVK_VSC_TO_VK_EX ) );
                    if( kb->Flags == 0x0 || kb->Flags == 0x2 ) {
                        down = true;
                    } else if( kb->Flags == 0x1 || kb->Flags == 0x3 ) {
                        up = true;
                    }
                    break;
            }

            if( down ) {
                switch( kb->VKey ) {
                    case VK_SHIFT: {
                        global_win32_state->keymod |= INPUT_KEYMOD_SHIFT;
                    } break;
                    case VK_CONTROL: {
                        global_win32_state->keymod |= INPUT_KEYMOD_CTRL;
                    } break;
                    case VK_MENU: {
                        global_win32_state->keymod |= INPUT_KEYMOD_ALT;
                    } break;
                    case VK_CAPITAL: {
                        global_win32_state->keymod =
                            bitfield_check(
                                global_win32_state->keymod, INPUT_KEYMOD_CAPSLK ) ?
                            bitfield_clear(
                                global_win32_state->keymod, INPUT_KEYMOD_CAPSLK ) :
                            bitfield_set(
                                global_win32_state->keymod, INPUT_KEYMOD_CAPSLK );
                    } break;
                    case VK_SCROLL: {
                        global_win32_state->keymod =
                            bitfield_check(
                                global_win32_state->keymod, INPUT_KEYMOD_SCRLK ) ?
                            bitfield_clear(
                                global_win32_state->keymod, INPUT_KEYMOD_SCRLK ) :
                            bitfield_set(
                                global_win32_state->keymod, INPUT_KEYMOD_SCRLK );
                    } break;
                    case VK_NUMLOCK: {
                        global_win32_state->keymod =
                            bitfield_check(
                                global_win32_state->keymod, INPUT_KEYMOD_NUMLK ) ?
                            bitfield_clear(
                                global_win32_state->keymod, INPUT_KEYMOD_NUMLK ) :
                            bitfield_set(
                                global_win32_state->keymod, INPUT_KEYMOD_NUMLK );
                    } break;
                    default: break;
                }
            } else if( up ) {
                switch( kb->VKey ) {
                    case VK_SHIFT: {
                        global_win32_state->keymod =
                            bitfield_clear(
                                global_win32_state->keymod, INPUT_KEYMOD_SHIFT );
                    } break;
                    case VK_CONTROL: {
                        global_win32_state->keymod =
                            bitfield_clear(
                                global_win32_state->keymod, INPUT_KEYMOD_CTRL );
                    } break;
                    case VK_MENU: {
                        global_win32_state->keymod =
                            bitfield_clear(
                                global_win32_state->keymod, INPUT_KEYMOD_ALT );
                    } break;
                    default: break;
                }
                
            }

            InputKeycode key  = win32_vk_to_keycode( vk );
            b32 send_keyboard = true;

            if( down ) {
                if( packed_bool_get( global_win32_state->keys, key ) ) {
                    send_keyboard = false;
                }
                packed_bool_set( (b8*)global_win32_state->keys, key, true );
            } else if( up ) {
                packed_bool_set( (b8*)global_win32_state->keys, key, false );
            } else {
                send_keyboard = false;
            }
            if( key == INPUT_KEYCODE_UNKNOWN ) {
                send_keyboard = false;
            }

            if( data ) {
                if( down ) {
                    data->kb[vk] = 1 << 7;
                } else if( up ) {
                    data->kb[vk] = 0;
                }
                switch( vk ) {
                    case VK_SHIFT:
                    case VK_LSHIFT:
                    case VK_RSHIFT: {
                        data->kb[VK_SHIFT]  = data->kb[vk];
                        data->kb[VK_LSHIFT] = data->kb[vk];
                        data->kb[VK_RSHIFT] = data->kb[vk];
                    } break;
                    case VK_CONTROL:
                    case VK_LCONTROL:
                    case VK_RCONTROL: {
                        data->kb[VK_CONTROL]  = data->kb[vk];
                        data->kb[VK_LCONTROL] = data->kb[vk];
                        data->kb[VK_RCONTROL] = data->kb[vk];
                    } break;
                    case VK_MENU:
                    case VK_LMENU:
                    case VK_RMENU: {
                        data->kb[VK_MENU]  = data->kb[vk];
                        data->kb[VK_LMENU] = data->kb[vk];
                        data->kb[VK_RMENU] = data->kb[vk];
                    } break;
                    case VK_CAPITAL: {
                        if( bitfield_check(
                            global_win32_state->keymod, INPUT_KEYMOD_CAPSLK
                        ) ) {
                            data->kb[vk] |= 1 << 0;
                        } else {
                            data->kb[vk] &= ~(1 << 0);
                        }
                    } break;
                    default: break;
                }
            }

            if( send_keyboard ) {
                WPARAM w = win32_make_key_wparam( key, down );
                PostMessageA( data->active, WM_INPUT_KEYBOARD, w, 0 );
            }
            if( down && data ) {
                WPARAM w = win32_make_text_wparam( vk, scan_translate );
                LPARAM l = win32_make_text_lparam( data->kb );
                PostMessageA(
                    data->active, WM_INPUT_KEYBOARD_TEXT, w, l );
            }
        } break;
        case RIM_TYPEMOUSE: {
            RAWMOUSE* mb = &raw->data.mouse;
            WPARAM rel_x = win32_make_mouse_pos_wparam( mb->lLastX );
            LPARAM rel_y = win32_make_mouse_pos_lparam( mb->lLastY );

            u16 flags = mb->usButtonFlags;
            u8 buttons_state = data->mb;
            if( bitfield_check( flags, RI_MOUSE_LEFT_BUTTON_DOWN ) ) {
                buttons_state |= INPUT_MOUSE_BUTTON_LEFT;
            } else if( bitfield_check( flags, RI_MOUSE_LEFT_BUTTON_UP ) ) {
                buttons_state &= ~INPUT_MOUSE_BUTTON_LEFT;
            }
            if( bitfield_check( flags, RI_MOUSE_RIGHT_BUTTON_DOWN ) ) {
                buttons_state |= INPUT_MOUSE_BUTTON_RIGHT;
            } else if( bitfield_check( flags, RI_MOUSE_RIGHT_BUTTON_UP ) ) {
                buttons_state &= ~INPUT_MOUSE_BUTTON_RIGHT;
            }
            if( bitfield_check( flags, RI_MOUSE_MIDDLE_BUTTON_DOWN ) ) {
                buttons_state |= INPUT_MOUSE_BUTTON_MIDDLE;
            } else if( bitfield_check( flags, RI_MOUSE_MIDDLE_BUTTON_UP ) ) {
                buttons_state &= ~INPUT_MOUSE_BUTTON_MIDDLE;
            }
            if( bitfield_check( flags, RI_MOUSE_BUTTON_4_DOWN ) ) {
                buttons_state |= INPUT_MOUSE_BUTTON_EXTRA_1;
            } else if( bitfield_check( flags, RI_MOUSE_BUTTON_4_UP ) ) {
                buttons_state &= ~INPUT_MOUSE_BUTTON_EXTRA_1;
            }
            if( bitfield_check( flags, RI_MOUSE_BUTTON_5_DOWN ) ) {
                buttons_state |= INPUT_MOUSE_BUTTON_EXTRA_2;
            } else if( bitfield_check( flags, RI_MOUSE_BUTTON_5_UP ) ) {
                buttons_state &= ~INPUT_MOUSE_BUTTON_EXTRA_2;
            }

            u8 buttons_changed = data->mb ^ buttons_state;
            data->mb = buttons_state;

            i16 scroll = signum( rcast( i16, &mb->usButtonData ) );
            WPARAM buttons = win32_make_mouse_button_wparam(
                buttons_state, buttons_changed,
                scroll, bitfield_check( mb->usButtonFlags, RI_MOUSE_HWHEEL ) );

            PostMessageA(
                data->active, WM_INPUT_MOUSE_POSITION_RELATIVE, rel_x, rel_y );
            if( buttons_changed || scroll ) {
                PostMessageA(
                    data->active, WM_INPUT_MOUSE_BUTTON,
                    buttons, 0 );
            }
        } break;
    }

    if( GET_RAWINPUT_CODE_WPARAM( wparam ) == RIM_INPUT ) {
        return DefWindowProcA( hwnd, msg, wparam, lparam );
    }
    return 0;
}

attr_internal MONITORINFO win32_get_monitor_info( HWND hwnd ) {
    HMONITOR mon    = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONEAREST );
    MONITORINFO res = {};
    res.cbSize      = sizeof(res);

    GetMonitorInfoA( mon, &res );
    return res;
}
attr_internal usize win32_filter_list_memory_requirement(
    const MediaPromptFileFilterList* list
) {
    usize res = 0;

    for( usize i = 0; i < list->len; ++i ) {
        MediaPromptFileFilter* filter = list->filters + i;

        res += filter->name.len + 1;

        for( usize j = 0; j < filter->len; ++j ) {
            String* pattern = filter->patterns + j;

            res += pattern->len + 1;
        }
        res++;
    }

    return res + 2; // extra padding just in case.
}
attr_internal void win32_filter_list_to_string(
    const MediaPromptFileFilterList* list, char* buf
) {
    char* at = buf;
    for( usize i = 0; i < list->len; ++i ) {
        MediaPromptFileFilter* filter = list->filters + i;
        memory_copy( at, filter->name.cc, filter->name.len );
        at += filter->name.len + 1;

        for( usize j = 0; j < filter->len; ++j ) {
            String* pattern = filter->patterns + j;

            memory_copy( at, pattern->cc, pattern->len );
            at += pattern->len;
            *at = ';';
            at++;
        }
        at++; // skip null-terminator
    }
}
attr_internal void win32_format_commdlg_error( DWORD error ) {
    #define cerr( format, ... )\
        media_error( "[WIN32 {u,X}] " format, error, ##__VA_ARGS__ )

    switch( error ) {
        case CDERR_DIALOGFAILURE: {
            cerr( "dialog box could not be created!" );
        } break;
        case CDERR_FINDRESFAILURE: {
            cerr( "failed to find a specific resource!" );
        } break;
        case CDERR_INITIALIZATION: {
            cerr( "failed to inialize comm dlg!" );
        } break;
        case CDERR_LOADRESFAILURE: {
            cerr( "failed to load a specified resource!" );
        } break;
        case CDERR_LOADSTRFAILURE: {
            cerr( "failed to load a specified string!" );
        } break;
        case CDERR_LOCKRESFAILURE: {
            cerr( "failed to lock a specified resource!" );
        } break;
        case CDERR_MEMALLOCFAILURE: {
            cerr( "failed to allocate internal memory!" );
        } break;
        case CDERR_MEMLOCKFAILURE: {
            cerr( "failed to lock memory associated with a handle!" );
        } break;
        case FNERR_INVALIDFILENAME: {
            cerr( "file name was invalid!" );
        } break;
        default: {
            cerr( "unknown error!" );
        } break;
    }

    #undef cerr
}
attr_internal InputKeycode win32_vk_to_keycode( DWORD vk ) {
    switch( vk ) {
        case VK_BACK                   : return INPUT_KEYCODE_BACKSPACE;
        case VK_TAB                    : return INPUT_KEYCODE_TAB;
        case VK_RETURN                 : return INPUT_KEYCODE_ENTER;
        case VK_LSHIFT:
        case VK_SHIFT                  : return INPUT_KEYCODE_SHIFT_LEFT;
        case VK_RSHIFT                 : return INPUT_KEYCODE_SHIFT_RIGHT;
        case VK_LCONTROL:
        case VK_CONTROL                : return INPUT_KEYCODE_CONTROL_LEFT;
        case VK_RCONTROL               : return INPUT_KEYCODE_CONTROL_RIGHT;
        case VK_LMENU:
        case VK_MENU                   : return INPUT_KEYCODE_ALT_LEFT;
        case VK_RMENU                  : return INPUT_KEYCODE_ALT_RIGHT;
        case VK_PAUSE                  : return INPUT_KEYCODE_PAUSE;
        case VK_CAPITAL                : return INPUT_KEYCODE_CAPSLOCK;
        case VK_ESCAPE                 : return INPUT_KEYCODE_ESCAPE;
        case VK_SPACE                  : return INPUT_KEYCODE_SPACE;
        case VK_PRIOR                  : return INPUT_KEYCODE_PAGE_UP;
        case VK_NEXT                   : return INPUT_KEYCODE_PAGE_DOWN;
        case VK_END                    : return INPUT_KEYCODE_END;
        case VK_HOME                   : return INPUT_KEYCODE_HOME;
        case VK_LEFT                   : return INPUT_KEYCODE_ARROW_LEFT;
        case VK_UP                     : return INPUT_KEYCODE_ARROW_UP;
        case VK_RIGHT                  : return INPUT_KEYCODE_ARROW_RIGHT;
        case VK_DOWN                   : return INPUT_KEYCODE_ARROW_DOWN;
        case 0x30 ... 0x39             : return (vk - 0x30) + INPUT_KEYCODE_0;
        case 0x41 ... 0x5A             : return (vk - 0x41) + INPUT_KEYCODE_A;
        case VK_LWIN                   : return INPUT_KEYCODE_SUPER_LEFT;
        case VK_RWIN                   : return INPUT_KEYCODE_SUPER_RIGHT;
        case VK_NUMPAD0 ... VK_NUMPAD9 : return (vk - VK_NUMPAD0) + INPUT_KEYCODE_PAD_0;
        case VK_F1 ... VK_F24          : return (vk - VK_F1) + INPUT_KEYCODE_F1;
        case VK_NUMLOCK                : return INPUT_KEYCODE_NUM_LOCK;
        case VK_SCROLL                 : return INPUT_KEYCODE_SCROLL_LOCK;
        case VK_OEM_1                  : return INPUT_KEYCODE_SEMICOLON;
        case VK_OEM_PLUS               : return INPUT_KEYCODE_EQUALS;
        case VK_OEM_COMMA              : return INPUT_KEYCODE_COMMA;
        case VK_OEM_MINUS              : return INPUT_KEYCODE_MINUS;
        case VK_OEM_PERIOD             : return INPUT_KEYCODE_PERIOD;
        case VK_OEM_2                  : return INPUT_KEYCODE_SLASH;
        case VK_OEM_3                  : return INPUT_KEYCODE_BACKTICK;
        case VK_OEM_4                  : return INPUT_KEYCODE_BRACKET_LEFT;
        case VK_OEM_5                  : return INPUT_KEYCODE_BACKSLASH;
        case VK_OEM_6                  : return INPUT_KEYCODE_BRACKET_RIGHT;
        case VK_OEM_7                  : return INPUT_KEYCODE_QUOTE;
        default                        : return INPUT_KEYCODE_UNKNOWN;
    }
}

#endif /* Platform Windows */

