/**
 * @file   surface.c
 * @brief  Media Windows Surface.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 11, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/surface.h"
#include "impl/win32/surface.h"
#include "impl/win32/input.h"
#include <windowsx.h>

struct Win32Input;
extern struct Win32Input* global_win32_input;

attr_media_api uintptr_t surface_query_memory_requirement(void) {
    return sizeof( struct Win32Surface );
}

attr_internal void win32_surface_flags_to_style(
    SurfaceCreateFlags flags, DWORD* out_dwstyle, DWORD* out_dwexstyle 
) {
    DWORD dwstyle   = 0;
    DWORD dwexstyle = WS_EX_OVERLAPPEDWINDOW;

    if( flags & SURFACE_CREATE_FLAG_RESIZEABLE ) {
        dwstyle = WS_OVERLAPPEDWINDOW;
    } else {
        dwstyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    }

    if( flags & SURFACE_CREATE_FLAG_NO_MINIMIZE ) {
        dwstyle &= ~WS_MINIMIZEBOX;
    }
    if( flags & SURFACE_CREATE_FLAG_NO_MAXIMIZE ) {
        dwstyle &= ~WS_MAXIMIZEBOX;
    }

    *out_dwstyle   = dwstyle;
    *out_dwexstyle = dwexstyle;
}

attr_media_api _Bool surface_create(
    uint32_t title_len, const char* title, int32_t x, int32_t y, int32_t w, int32_t h,
    SurfaceCreateFlags flags, SurfaceCallbackFN* opt_callback,
    void* opt_callback_params, SurfaceHandle* opt_parent, SurfaceHandle* out_surface
) {
    unused(x, y, w, h, flags );

    HANDLE parent = NULL;
    if( opt_parent ) {
        struct Win32Surface* parent_surface = opt_parent;
        parent = parent_surface->hwnd;
    }

    if( !out_surface ) {
        win32_error( "surface_create: did not provide a buffer for surface!" );
        return false;
    }

    struct Win32Surface* surface = out_surface;
    surface->callback        = opt_callback;
    surface->callback_params = opt_callback_params;

    surface->create_flags = flags;

    uint32_t max_title_len = title_len;
    if( title && title_len ) {
        if( max_title_len > SURFACE_MAX_TITLE_LEN ) {
            max_title_len = SURFACE_MAX_TITLE_LEN;
        }

        int wide_title_len = MultiByteToWideChar(
            CP_UTF8, 0, title,
            max_title_len, surface->title_ucs2,
            WIN32_SURFACE_TITLE_UCS2_CAP - 1 );

        surface->title_ucs2[wide_title_len] = 0;
    } else {
        memcpy( surface->title_ucs2, L"Surface", sizeof(L"Surface") );
        surface->title_len = sizeof(L"Surface") - 1;
    }

    DWORD dwExStyle = 0;
    DWORD dwStyle   = 0;

    win32_surface_flags_to_style( flags, &dwStyle, &dwExStyle );

    MONITORINFO monitor = win32_monitor_info( NULL );

    surface->w = w ? w : 800;
    surface->h = h ? h : 600;

    if( flags & SURFACE_CREATE_FLAG_X_CENTERED ) {
        int32_t monitor_width = monitor.rcMonitor.right - monitor.rcMonitor.left;
        surface->x = monitor.rcMonitor.left + ((monitor_width / 2) - (surface->w / 2));
    } else {
        surface->x = x;
    }
    if( flags & SURFACE_CREATE_FLAG_Y_CENTERED ) {
        int32_t monitor_height = monitor.rcMonitor.bottom - monitor.rcMonitor.top;
        surface->y = monitor.rcMonitor.top + ((monitor_height / 2) - (surface->h / 2));
    } else {
        surface->y = y;
    }

    RECT rect;
    memset( &rect, 0, sizeof(rect) );
    rect.right  = surface->w;
    rect.bottom = surface->h;
    
    if( !AdjustWindowRectEx( &rect, dwStyle, FALSE, dwExStyle ) ) {
        win32_error_message( GetLastError(),
            "surface_create: failed to set window size!" );
        return false;
    }

    HWND handle = CreateWindowExW(
        dwExStyle,
        WIN32_DEFAULT_WINDOW_CLASS, surface->title_ucs2,
        dwStyle,
        surface->x, surface->y,
        rect.right - rect.left, rect.bottom - rect.top,
        parent, NULL, GetModuleHandleW(0), 0 );

    if( !handle ) {
        win32_error_message(
            GetLastError(), "surface_create: failed to create window!" );
        return false;
    }

    memset( surface->title_utf8, 0, WIN32_SURFACE_TITLE_SIZE );
    if( title && title_len ) {
        memcpy( surface->title_utf8, title, max_title_len );
        surface->title_len = max_title_len;
    } else {
        memcpy( surface->title_utf8, "Surface", sizeof("Surface") );
        surface->title_len = sizeof("Surface") - 1;
    }

    surface->hwnd = handle;
    surface->hdc  = GetDC( surface->hwnd );

    SetWindowLongPtrW( surface->hwnd, GWLP_USERDATA, (WIN32_PTR)surface );

    if( flags & SURFACE_CREATE_FLAG_DARK_MODE ) {
        BOOL value = TRUE;
        DwmSetWindowAttribute(
            surface->hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
            &value, sizeof(value) );
    }

    if( flags & SURFACE_CREATE_FLAG_FULLSCREEN ) {
        surface_set_fullscreen( surface, true );
    }
    
    if( flags & SURFACE_CREATE_FLAG_HIDDEN ) {
        surface->state |= SURFACE_STATE_IS_HIDDEN;
    } else {
        ShowWindow( surface->hwnd, SW_SHOW );
    }

    return true;
}
attr_media_api void surface_destroy( SurfaceHandle* in_surface ) {
    struct Win32Surface* surface = in_surface;

    ReleaseDC( surface->hwnd, surface->hdc );
    DestroyWindow( surface->hwnd );

    memset( surface, 0, sizeof(*surface) );
}
attr_media_api void surface_pump_events(void) {
    MSG message;
    memset( &message, 0, sizeof(message) );
    while( PeekMessageW( &message, 0, 0, 0, PM_REMOVE ) ) {
        TranslateMessage( &message );
        DispatchMessageW( &message );
    }
}
attr_media_api void surface_set_callback(
    SurfaceHandle* in_surface, SurfaceCallbackFN* callback, void* opt_callback_params
) {
    struct Win32Surface* surface = in_surface;
    surface->callback        = callback;
    surface->callback_params = opt_callback_params;
}
attr_media_api void surface_clear_callback( SurfaceHandle* in_surface ) {
    struct Win32Surface* surface = in_surface;
    surface->callback        = 0;
    surface->callback_params = 0;
}
attr_media_api void* surface_get_platform_handle( SurfaceHandle* in_surface ) {
    struct Win32Surface* surface = in_surface;
    return (void*)surface->hwnd;
}
attr_media_api const char* surface_query_title(
    const SurfaceHandle* in_surface, uint32_t* opt_out_len
) {
    const struct Win32Surface* surface = in_surface;

    if( opt_out_len ) {
        *opt_out_len = surface->title_len;
    }
    return surface->title_utf8;
}
attr_media_api void surface_set_title(
    SurfaceHandle* in_surface, uint32_t len, const char* title
) {
    struct Win32Surface* surface = in_surface;
    memset( surface->title_ucs2, 0, WIN32_SURFACE_TITLE_SIZE );

    MultiByteToWideChar(
        CP_UTF8, 0, title, len,
        surface->title_ucs2, WIN32_SURFACE_TITLE_UCS2_CAP - 1 );

    SetWindowTextW( surface->hwnd, surface->title_ucs2 );
    memset( surface->title_ucs2, 0, WIN32_SURFACE_TITLE_UCS2_CAP );

    uint32_t max_len = len;
    if( max_len > SURFACE_MAX_TITLE_LEN - 1 ) {
        max_len = SURFACE_MAX_TITLE_LEN - 1;
    }

    memcpy( surface->title_utf8, title, max_len );
    surface->title_len = max_len - 1;
}
attr_media_api void surface_query_position(
    const SurfaceHandle* in_surface, int32_t* out_x, int32_t* out_y
) {
    const struct Win32Surface* surface = in_surface;

    *out_x = surface->x;
    *out_y = surface->y;
}
attr_media_api void surface_set_position(
    SurfaceHandle* in_surface, int32_t x, int32_t y
) {
    struct Win32Surface* surface = in_surface;
    // NOTE(alicia): surface x and y are updated in winproc

    SetWindowPos(
        surface->hwnd, NULL,
        x, y, 0, 0,
        SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
}
attr_media_api void surface_query_dimensions(
    const SurfaceHandle* in_surface, int32_t* out_w, int32_t* out_h
) {
    const struct Win32Surface* surface = in_surface;

    *out_w = surface->w;
    *out_h = surface->h;
}
attr_media_api void surface_set_dimensions(
    SurfaceHandle* in_surface, int32_t w, int32_t h
) {
    struct Win32Surface* surface = in_surface;

    if( surface->state & SURFACE_STATE_FULLSCREEN ) {
        return;
    }
    if( surface->w == w && surface->h == h ) {
        return;
    }

    RECT rect   = {};
    rect.right  = w;
    rect.bottom = h;

    DWORD dwStyle   = (DWORD)GetWindowLongPtrW( surface->hwnd, GWL_STYLE );
    DWORD dwExStyle = (DWORD)GetWindowLongPtrW( surface->hwnd, GWL_EXSTYLE );

    if( AdjustWindowRectEx( &rect, dwStyle, FALSE, dwExStyle ) ) {
        SetWindowPos(
            surface->hwnd, NULL,
            0, 0,
            rect.right - rect.left,
            rect.bottom - rect.top,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
    }
}
attr_media_api SurfaceStateFlags surface_query_state(
    const SurfaceHandle* in_surface
) {
    const struct Win32Surface* surface = in_surface;
    return surface->state;
}
attr_media_api void surface_set_fullscreen(
    SurfaceHandle* in_surface, _Bool is_fullscreen
) {
    struct Win32Surface* surface = in_surface;

    _Bool current_fullscreen = (surface->state & SURFACE_STATE_FULLSCREEN);

    if( current_fullscreen == is_fullscreen ) {
        return;
    }

    if( is_fullscreen ) {
        surface->state |= SURFACE_STATE_FULLSCREEN;
        memset( &surface->placement, 0, sizeof(surface->placement) );
        surface->placement.length = sizeof( surface->placement );
        GetWindowPlacement( surface->hwnd, &surface->placement );

        MONITORINFO monitor = win32_monitor_info( surface->hwnd );

        SetWindowLongPtrW( surface->hwnd, GWL_STYLE, WS_POPUP );
        SetWindowLongPtrW( surface->hwnd, GWL_EXSTYLE, 0 );

        int32_t x = monitor.rcMonitor.left;
        int32_t y = monitor.rcMonitor.top;
        int32_t w = monitor.rcMonitor.right - monitor.rcMonitor.left;
        int32_t h = monitor.rcMonitor.bottom - monitor.rcMonitor.top;

        SetWindowPos( 
            surface->hwnd, HWND_TOP, 
            x, y, w, h,
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
    } else {
        surface->state &= ~SURFACE_STATE_FULLSCREEN;

        DWORD dwStyle   = 0;
        DWORD dwExStyle = 0;

        win32_surface_flags_to_style( surface->create_flags, &dwStyle, &dwExStyle );

        SetWindowLongPtrW( surface->hwnd, GWL_STYLE, (WIN32_PTR)dwStyle );
        SetWindowLongPtrW( surface->hwnd, GWL_EXSTYLE, (WIN32_PTR)dwExStyle );

        SetWindowPlacement( surface->hwnd, &surface->placement );

        SetWindowPos(
            surface->hwnd, NULL,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
    }
}
attr_media_api void surface_set_hidden(
    SurfaceHandle* in_surface, _Bool is_hidden
) {
    struct Win32Surface* surface = in_surface;
    if( (surface->state & SURFACE_STATE_IS_HIDDEN) == is_hidden ) {
        return;
    }

    int cbShow = 0;
    if( is_hidden ) {
        cbShow = SW_HIDE;
        surface->state |= SURFACE_STATE_IS_HIDDEN;
    } else {
        cbShow = SW_SHOW;
        surface->state &= ~SURFACE_STATE_IS_HIDDEN;
    }

    ShowWindow( surface->hwnd, cbShow );
}

LRESULT win32_winproc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    struct Win32Surface* surface =
        (struct Win32Surface*)GetWindowLongPtrW( hwnd, GWLP_USERDATA );

    SurfaceCallbackData data;
    memset( &data, 0, sizeof(data) );
    #define cb() surface->callback( surface, &data, surface->callback_params )

    _Bool activated;
    switch( msg ) {
        case WM_SETCURSOR: {
            CursorType cursor = CURSOR_TYPE_ARROW;
            switch( LOWORD(lparam) ) {
                case HTRIGHT:
                case HTLEFT: {
                    cursor = CURSOR_TYPE_SIZE_H;
                } break;
                case HTTOP:
                case HTBOTTOM: {
                    cursor = CURSOR_TYPE_SIZE_V;
                } break;
                case HTBOTTOMRIGHT:
                case HTTOPLEFT: {
                    cursor = CURSOR_TYPE_SIZE_L;
                } break;
                case HTBOTTOMLEFT:
                case HTTOPRIGHT: {
                    cursor = CURSOR_TYPE_SIZE_R;
                } break;
                case HTCLIENT: {
                    if( surface ) {
                        cursor = surface->cursor;
                    }
                } break;
                default: break;
            }
            if( global_win32_cursor_hidden ) {
                SetCursor( NULL );
            } else {
                SetCursor( global_win32_cursors[cursor] );
            }
        } return TRUE;
        case WM_ACTIVATE: {
            if( LOWORD( wparam ) ) {
                activated = true;
            } else {
                activated = false;
            }
            if( activated == ((surface->state & SURFACE_STATE_IS_FOCUSED) != 0)) {
                return 0;
            }

            if( activated ) {
                surface->state |= SURFACE_STATE_IS_FOCUSED;
            } else {
                surface->state &= ~SURFACE_STATE_IS_FOCUSED;
            }
        } break;
        default: break;
    }

    if( !surface || !surface->callback ) {
        return DefWindowProcW( hwnd, msg, wparam, lparam );
    }

    switch( msg ) {
        case WM_CLOSE: {
            data.type = SURFACE_CALLBACK_TYPE_CLOSE;
            cb();
        } return 0;
        case WM_ACTIVATE: {
            data.type = SURFACE_CALLBACK_TYPE_FOCUS;
            data.focus.gained = activated;
            cb();
        } return 0;
        case WM_CHAR: {
            if( wparam == UNICODE_NOCHAR ) {
                return TRUE;
            }

            data.type = SURFACE_CALLBACK_TYPE_TEXT;
            WideCharToMultiByte(
                CP_UTF8, 0, (LPWSTR)&wparam, sizeof(wparam),
                data.text.utf8, sizeof(data.text.utf8), 0, 0 );

            if( data.text.utf8[0] == '\r' ) {
                memset( data.text.utf8, 0, sizeof(data.text.utf8) );
                data.text.utf8[0] = '\n';
            }

            cb();
        } return FALSE;
        case WM_WINDOWPOSCHANGED: {
            WINDOWPOS* pos = (WINDOWPOS*)lparam;
            _Bool no_size = pos->flags & SWP_NOSIZE;
            _Bool no_move = pos->flags & SWP_NOMOVE;
            
            if( !no_move ) {
                int32_t old_x = surface->x;
                int32_t old_y = surface->y;
                int32_t x     = pos->x;
                int32_t y     = pos->y;

                surface->x = x;
                surface->y = y;

                if( !(old_x == x && old_y == y) ) {
                    data.type = SURFACE_CALLBACK_TYPE_POSITION;
                    data.position.x = x;
                    data.position.y = y;
                    data.position.old_x = old_x;
                    data.position.old_y = old_y;

                    cb();

                    memset( &data, 0, sizeof(data) );
                }
            }

            if( !no_size ) {
                int32_t old_w = surface->w;
                int32_t old_h = surface->h;

                RECT client;
                memset( &client, 0, sizeof(client) );
                if( GetClientRect( surface->hwnd, &client ) ) {
                    int32_t w = client.right  < 1 ? 1 : client.right;
                    int32_t h = client.bottom < 1 ? 1 : client.bottom;

                    surface->w = w;
                    surface->h = h;

                    if( !(old_w == w && old_h == h ) ) {
                        data.type = SURFACE_CALLBACK_TYPE_RESIZE;
                        data.resize.old_w = old_w;
                        data.resize.old_h = old_h;
                        data.resize.w     = w;
                        data.resize.h     = h;

                        cb();

                        // TODO(alicia): lock mouse cursor
                    }
                }
            }
        } return 0;

        default: break;
    }

    if( global_win32_input ) {
        switch( msg ) {
            case WM_CUSTOM_KEYBOARD: {
                struct Win32KeyWParam w = win32_key_from_wparam( wparam );

                data.type        = SURFACE_CALLBACK_TYPE_KEY;
                data.key.code    = w.keycode;
                data.key.is_down = w.is_down;
                data.key.mod     = global_win32_state->mod;

                cb();
            } return 0;
            case WM_CUSTOM_MOUSE_POS: {
                struct Win32MousePositionParam x, y;
                x = win32_mouse_x_from_wparam( wparam );
                y = win32_mouse_y_from_lparam( lparam );

                POINT pos;
                pos.x = x.v;
                pos.y = y.v;

                ScreenToClient( hwnd, &pos );
                if(
                    ( pos.x >= 0 && pos.x <= surface->w ) &&
                    ( pos.y >= 0 && pos.y <= surface->h )
                ) {
                    data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE;
                    data.mouse_move.x = pos.x;
                    data.mouse_move.y = surface->h - pos.y;

                    cb();
                }
            } return 0;
            case WM_CUSTOM_MOUSE_DEL: {
                struct Win32MousePositionParam x, y;
                x = win32_mouse_x_from_wparam( wparam );
                y = win32_mouse_y_from_lparam( lparam );

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA;
                data.mouse_move_delta.x =  x.v;
                data.mouse_move_delta.y = -y.v;

                cb();
            } return 0;
            case WM_CUSTOM_MOUSE_BTN: {
                struct Win32MouseButtonWParam b =
                    win32_mouse_button_from_wparam( wparam );

                if( b.delta ) {
                    data.type = SURFACE_CALLBACK_TYPE_MOUSE_BUTTON;
                    data.mouse_button.state = b.state;
                    data.mouse_button.delta = b.delta;

                    cb();

                    memset( &data, 0, sizeof(data) );
                }

                if( b.scroll ) {
                    data.type                      = SURFACE_CALLBACK_TYPE_MOUSE_WHEEL;
                    data.mouse_wheel.delta         = b.scroll;
                    data.mouse_wheel.is_horizontal = b.is_scroll_horizontal;

                    cb();
                }
            } return 0;
            default: break;
        }
    } else {
        #define TRANSITION_STATE_MASK (1 << 31)
        #define EXTENDED_KEY_MASK     (1 << 24)
        #define SCANCODE_MASK         (0x00FF0000)

        if( !(surface->state & SURFACE_STATE_IS_FOCUSED) ) {
            return DefWindowProcW( hwnd, msg, wparam, lparam );
        }

        switch( msg ) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {

                _Bool prev = (lparam >> 30) == 1;
                if( prev ) {
                    return DefWindowProcW( hwnd, msg, wparam, lparam );
                }
                WORD vk      = LOWORD(wparam);
                WORD vk_orig = vk;
                WORD flags   = HIWORD(lparam);
                WORD scan    = LOBYTE(flags);

                if( (flags & KF_EXTENDED) == KF_EXTENDED ) {
                    scan = MAKEWORD(scan, 0xE0);
                }

                switch( vk ) {
                    case VK_SHIFT:
                    case VK_CONTROL:
                    case VK_MENU: {
                        vk = LOWORD( MapVirtualKeyW(
                            scan, MAPVK_VSC_TO_VK_EX ));
                    } break;
                    default: break;
                }

                _Bool is_down = !((lparam & TRANSITION_STATE_MASK) != 0);
                if( is_down ) {
                    switch( vk_orig ) {
                        case VK_SHIFT: {
                            global_win32_state->mod |= KBMOD_SHIFT;
                        } break;
                        case VK_CONTROL: {
                            global_win32_state->mod |= KBMOD_CTRL;
                        } break;
                        case VK_MENU: {
                            global_win32_state->mod |= KBMOD_ALT;
                        } break;
                        case VK_CAPITAL: {
                            if( global_win32_state->mod & KBMOD_CAPSLK ) {
                                global_win32_state->mod &= ~KBMOD_CAPSLK;
                            } else {
                                global_win32_state->mod |= KBMOD_CAPSLK;
                            }
                        } break;
                        case VK_SCROLL: {
                            if( global_win32_state->mod & KBMOD_SCRLK ) {
                                global_win32_state->mod &= ~KBMOD_SCRLK;
                            } else {
                                global_win32_state->mod |= KBMOD_SCRLK;
                            }
                        } break;
                        case VK_NUMLOCK: {
                            if( global_win32_state->mod & KBMOD_NUMLK ) {
                                global_win32_state->mod &= ~KBMOD_NUMLK;
                            } else {
                                global_win32_state->mod |= KBMOD_NUMLK;
                            }
                        } break;
                        default: break;
                    }
                } else {
                    switch( vk_orig ) {
                        case VK_SHIFT: {
                            global_win32_state->mod &= ~KBMOD_SHIFT;
                        } break;
                        case VK_CONTROL: {
                            global_win32_state->mod &= ~KBMOD_CTRL;
                        } break;
                        case VK_MENU: {
                            global_win32_state->mod &= ~KBMOD_ALT;
                        } break;
                        default: break;
                    }
                }

                data.type        = SURFACE_CALLBACK_TYPE_KEY;
                data.key.code    = vk_to_keyboard_code( vk );
                data.key.is_down = is_down;
                data.key.mod     = global_win32_state->mod;

                cb();
            } return TRUE;

            case WM_MOUSEMOVE: {
                RECT rect;
                memset( &rect, 0, sizeof(rect) );

                GetClientRect( hwnd, &rect );

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE;
                data.mouse_move.x = GET_X_LPARAM(lparam);
                data.mouse_move.y = rect.bottom - GET_Y_LPARAM(lparam);

                cb();
            } return TRUE;

            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP: {
                _Bool is_down =
                    msg == WM_LBUTTONDOWN ||
                    msg == WM_MBUTTONDOWN ||
                    msg == WM_RBUTTONDOWN;

                MouseButton btn = global_win32_state->mb;
                if( msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP ) {
                    btn = is_down ? btn | MB_LEFT : btn & ~MB_LEFT;
                } else if( msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP ) {
                    btn = is_down ? btn | MB_RIGHT : btn & ~MB_RIGHT;
                } else if( msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP ) {
                    btn = is_down ? btn | MB_MIDDLE : btn & ~MB_MIDDLE;
                } else {
                    return TRUE;
                }

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_BUTTON;
                data.mouse_button.state = btn;
                data.mouse_button.delta = global_win32_state->mb ^ btn;

                global_win32_state->mb = btn;

                cb();
            } return TRUE;

            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP: {
                UINT button      = GET_XBUTTON_WPARAM(wparam);
                _Bool is_down = msg == WM_XBUTTONDOWN;

                MouseButton btn = global_win32_state->mb;
                switch( button ) {
                    case XBUTTON1: {
                        btn = is_down ? btn | MB_EXTRA_1 : btn & ~MB_EXTRA_1;
                    } break;
                    case XBUTTON2: {
                        btn = is_down ? btn | MB_EXTRA_2 : btn & ~MB_EXTRA_2;
                    } break;
                    default: return TRUE;
                }

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_BUTTON;
                data.mouse_button.state = btn;
                data.mouse_button.delta = global_win32_state->mb ^ btn;

                global_win32_state->mb = btn;

                cb();
            } return TRUE;

            case WM_MOUSEWHEEL:
            case WM_MOUSEHWHEEL: {
                int delta = GET_WHEEL_DELTA_WPARAM(wparam);
                if( !delta ) {
                    return TRUE;
                }

                if( delta < 0 ) {
                    delta = -1;
                } else {
                    delta = 1;
                }

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_WHEEL;
                data.mouse_wheel.is_horizontal = msg == WM_MOUSEHWHEEL;
                data.mouse_wheel.delta         = delta;

                cb();
            } return TRUE;

            default: break;
        }
    }


    return DefWindowProcW( hwnd, msg, wparam, lparam );
    #undef cb
}
attr_media_api void cursor_type_set( SurfaceHandle* in_surface, CursorType cursor ) {
    struct Win32Surface* surface = in_surface;
    surface->cursor = cursor;
}
attr_media_api void cursor_center( SurfaceHandle* in_surface ) {
    struct Win32Surface* surface = in_surface;

    POINT pt;
    pt.x = surface->w / 2;
    pt.y = surface->h / 2;
    if( ClientToScreen( surface->hwnd, &pt ) ) {
        SetCursorPos( pt.x, pt.y );
    }
}

#endif /* Platform Windows */

