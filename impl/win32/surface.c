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

#include "impl/win32/surface.h"
#include "impl/win32/input.h"

#include <cderr.h>

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

    if( bitfield_check( surface->state, WIN32_SURFACE_STATE_IS_FOCUSED ) ) {
        b32 alt = false, f4 = false;
        if( global_media_win32_input_state ) {
            alt = packed_bool_get(
                global_media_win32_input_state->keys, INPUT_KEYCODE_ALT_LEFT );
            alt = alt ||
                packed_bool_get(
                    global_media_win32_input_state->keys, INPUT_KEYCODE_ALT_RIGHT );
            f4 = packed_bool_get(
                global_media_win32_input_state->keys, INPUT_KEYCODE_F4 );
        } else {
            alt = GetKeyState( VK_MENU ) < 0;
            f4  = GetKeyState( VK_F4 )   < 0;
        }

        if( alt && f4 ) {
            PostMessageA( surface->hwnd, WM_CLOSE, 0, 0 );
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

// NOTE(alicia): internals

LRESULT win32_winproc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    struct Win32Surface* surface =
        (struct Win32Surface*)GetWindowLongPtrA( hwnd, GWLP_USERDATA );
    // no message can be handled before surface arrives.
    if( !surface ) {
        goto win32_winproc_skip;
    }
    MediaCursor cursor = surface->cursor;

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
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:  return 0;
        default: break;
    }

    if( !global_media_win32_input_state ) {
        goto win32_winproc_skip;
    }

    InputKeymod mod = global_media_win32_input_state->keymod;

    switch( msg ) {
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
            data.mouse_delta.x =  x.v;
            data.mouse_delta.y = -y.v;

            cb();
        } return 0;
        default: break;
    }

    #undef cb

win32_winproc_skip:
    return DefWindowProcA( hwnd, msg, wparam, lparam );
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

#endif /* Platform Windows */

