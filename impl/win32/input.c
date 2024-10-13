/**
 * @file   input.c
 * @brief  Media Windows Input.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 12, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/surface.h"
#include "media/input.h"
#include "impl/win32/common.h"
#include "impl/win32/input.h"

#include <xinput.h>
#include <hidusage.h>

#define WIN32_INPUT_WINDOW_CLASS L"MediaInputWindow"
#define WIN32_INPUT_POLL_XINPUT_MS 2

struct Win32Input* global_win32_input = NULL;

#define def( ret, fn, ... )\
    typedef ret fn##FN( __VA_ARGS__ );\
    fn##FN* in_##fn = NULL

def( BOOL, RegisterRawInputDevices,
    PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize );
#define RegisterRawInputDevices in_RegisterRawInputDevices

def( UINT, GetRawInputData,
    HRAWINPUT hRawInput, UINT uiCommand,
    LPVOID pData, PUINT pcbSize, UINT cbSizeHeader );
#define GetRawInputData in_GetRawInputData

def( DWORD, XInputGetState, DWORD dwUserIndex, XINPUT_STATE* pState );
#define XInputGetState in_XInputGetState

def( DWORD, XInputSetState, DWORD dwUserIndex, XINPUT_VIBRATION* pVibration );
#define XInputSetState in_XInputSetState

DWORD XInputSetState_stub( DWORD dwUserIndex, XINPUT_VIBRATION* pVibration ) {
    unused( dwUserIndex, pVibration );
    return ERROR_SUCCESS;
}

LRESULT win32_winproc_input( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

DWORD win32_xinput_thread( LPVOID lpParameter ) {
    volatile long* atomic = (volatile long*)lpParameter;
    unused(atomic);

    for( ;; ) {
        MemoryBarrier();
        if( InterlockedCompareExchange( atomic, *atomic, 1 ) ) {
            break;
        }
        MemoryBarrier();

        XINPUT_STATE pState;
        for( DWORD i = 0; i < XUSER_MAX_COUNT; ++i ) {
            DWORD res = XInputGetState( i, &pState );
            global_win32_input->gp_connected[i] = res != ERROR_DEVICE_NOT_CONNECTED;
        }

        Sleep( WIN32_INPUT_POLL_XINPUT_MS );
    }

    InterlockedIncrement( atomic );
    return 0;
}

attr_media_api uintptr_t input_subsystem_query_memory_requirement(void) {
    return sizeof(*global_win32_input);
}
attr_media_api _Bool input_subsystem_initialize( void* buffer ) {
    #define load( lib, fn ) do {\
        if( !fn ) {\
            fn = (fn##FN*)GetProcAddress( global_win32_state->modules.lib, #fn );\
            if( !fn ) {\
                win32_error( "input_subsystem_initialize:"\
                    "failed to load " #fn " from " #lib "!");\
                return false;\
            }\
        }\
    } while(0)

    HMODULE module     = GetModuleHandleW(0);
    global_win32_input = buffer;

    load( USER32, RegisterRawInputDevices );
    load( USER32, GetRawInputData );

    _Bool xinput_1_3 = false;
    HMODULE xinput = LoadLibraryA( "XINPUT1_4.DLL" );
    if( !xinput ) {
        xinput = LoadLibraryA( "XINPUT9_1_0.DLL" );
        if( !xinput ) {
            xinput = LoadLibraryA( "XINPUT1_3.DLL" );
            if( !xinput ) {
                win32_error_message( GetLastError(), "failed to load XINPUT!" );
            } else {
                xinput_1_3 = true;
            }
        }
    }
    global_win32_state->modules.XINPUT = xinput;
    if( xinput ) {
        load( XINPUT, XInputGetState );
        if( xinput_1_3 ) {
            XInputSetState = XInputSetState_stub;
        } else {
            load( XINPUT, XInputSetState );
        }

        global_win32_input->thread =
            CreateThread(
                NULL, 0, win32_xinput_thread,
                (void*)&global_win32_input->thread_exit, 0, 0 );
        if( !global_win32_input->thread ) {
            win32_error_message( GetLastError(), "failed to create Xinput thread!" );
            return false;
        }
    }

    /* create input window */ {
        WNDCLASSEXW class;
        memset( &class, 0, sizeof(class) );
        class.cbSize        = sizeof(class);
        class.lpszClassName = WIN32_INPUT_WINDOW_CLASS;
        class.hInstance     = module;
        class.lpfnWndProc   = win32_winproc_input;

        if( !RegisterClassExW( &class ) ) {
            win32_error_message( GetLastError(),
                "input_subsystem_initialize: failed to register input window!" );
            return false;
        }

        global_win32_input->hwnd = CreateWindowExW(
            0, WIN32_INPUT_WINDOW_CLASS, NULL,
            0, 0, 0, 0, 0, HWND_MESSAGE, NULL, module, NULL );

        if( !global_win32_input->hwnd ) {
            win32_error_message(
                GetLastError(),
                "input_subsystem_initialize: failed to create input window!" );
            UnregisterClassW( WIN32_INPUT_WINDOW_CLASS, module );
            return false;
        }
    }

    /* register raw input */ {
        RAWINPUTDEVICE rid[2];
        memset( rid, 0, sizeof(rid) );
        rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[0].usUsage     = HID_USAGE_GENERIC_MOUSE;
        rid[0].dwFlags     = RIDEV_INPUTSINK;
        rid[0].hwndTarget  = global_win32_input->hwnd;

        rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[1].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
        rid[1].dwFlags     = RIDEV_INPUTSINK;
        rid[1].hwndTarget  = global_win32_input->hwnd;

        if( !RegisterRawInputDevices(
            rid, sizeof(rid) / sizeof(rid[0]), sizeof(rid[0]) 
        ) ) {
            win32_error_message(
                GetLastError(),
                "input_subsystem_initialize: failed to register input devices!" );
            DestroyWindow( global_win32_input->hwnd );
            UnregisterClassW( WIN32_INPUT_WINDOW_CLASS, module );
            return false;
        }
    }

    #undef load
    return true;
}
attr_media_api void input_subsystem_shutdown(void) {
    InterlockedIncrement( &global_win32_input->thread_exit );
    // NOTE(alicia): wait for xinput thread to return.
    for( ;; ) {
        MemoryBarrier();
        if( InterlockedCompareExchange(
            &global_win32_input->thread_exit,
            global_win32_input->thread_exit, 2
        ) == 2 ) {
            break;
        }
    }
    MemoryBarrier();

    CloseHandle( global_win32_input->thread );

    HMODULE module = GetModuleHandleW(0);
    DestroyWindow( global_win32_input->hwnd );
    UnregisterClassW( WIN32_INPUT_WINDOW_CLASS, module );

    memset( global_win32_input, 0, sizeof(*global_win32_input) );
    global_win32_input = NULL;
}
attr_media_api void input_subsystem_update(void) {
    XINPUT_STATE xinput_state;
    memset( &xinput_state, 0, sizeof(xinput_state) );
    for( DWORD i = 0; i < XUSER_MAX_COUNT; ++i ) {
        if( !global_win32_input->gp_connected[i] ) {
            continue;
        }

        DWORD res = XInputGetState( i, &xinput_state );
        if( res == ERROR_DEVICE_NOT_CONNECTED ) {
            global_win32_input->gp_connected[i] = false;
            memset(
                global_win32_input->rumble[i],
                0, sizeof(global_win32_input->rumble[0]));
            continue;
        }

        XINPUT_GAMEPAD* xgp = &xinput_state.Gamepad;
        GamepadState*   gp  = global_win32_input->gp + i;

        gp->buttons       = xgp->wButtons;
        gp->stick_left_x  = xgp->sThumbLX;
        gp->stick_left_y  = xgp->sThumbLY;
        gp->stick_right_x = xgp->sThumbRX;
        gp->stick_right_y = xgp->sThumbRY;
        gp->trigger_left  = xgp->bLeftTrigger;
        gp->trigger_right = xgp->bRightTrigger;

        memset( &xinput_state, 0, sizeof(xinput_state) );
    }

    HWND focused = win32_get_focused_window();
    POINT point;
    GetCursorPos( &point );
    if( focused ) {
        if(
            point.x != global_win32_input->mb_x ||
            point.y != global_win32_input->mb_y
        ) {
            PostMessageW(
                focused, WM_CUSTOM_MOUSE_POS,
                win32_mouse_x_to_wparam( point.x ),
                win32_mouse_y_to_lparam( point.y ) );
        }
    }

    global_win32_input->mb_x = point.x;
    global_win32_input->mb_y = point.y;

    global_win32_input->mb_dx = 0;
    global_win32_input->mb_dy = 0;

    MSG message;
    memset( &message, 0, sizeof(message) );
    while( PeekMessageW(
        &message, global_win32_input->hwnd,
        WM_INPUT, WM_INPUT, PM_REMOVE
    ) ) {
        DispatchMessageW( &message );
    }
}
attr_media_api KeyboardMod input_keyboard_query_mod(void) {
    return global_win32_state->mod;
}
attr_media_api _Bool input_keyboard_query_key( KeyboardCode keycode ) {
    return keyboard_state_get_key( &global_win32_input->kb, keycode );
}
attr_media_api void input_keyboard_copy_state( KeyboardState* out_state ) {
    memcpy( out_state, &global_win32_input->kb, sizeof(*out_state) );
}
attr_media_api MouseButton input_mouse_query_buttons(void) {
    return global_win32_state->mb;
}
attr_media_api void input_mouse_query_position( int32_t* out_x, int32_t* out_y ) {
    *out_x = global_win32_input->mb_x;
    *out_y = global_win32_input->mb_y;
}
attr_media_api void input_mouse_position_to_client(
    SurfaceHandle* surface, int32_t* in_out_x, int32_t* in_out_y
) {
    HWND hwnd = (HWND)surface_get_platform_handle( surface );
    POINT pos;
    pos.x = *in_out_x;
    pos.y = *in_out_y;
    ScreenToClient( hwnd, &pos );

    int32_t w, h;
    surface_query_dimensions( surface, &w, &h );

    if( pos.x < 0 ) {
        pos.x = 0;
    }
    if( pos.x > w ) {
        pos.x = w;
    }
    if( pos.y < 0 ) {
        pos.y = 0;
    }
    if( pos.y > h ) {
        pos.y = h;
    }

    *in_out_x = pos.x;
    *in_out_y = h - pos.y;
}
attr_media_api void input_mouse_query_delta( int32_t* out_x, int32_t* out_y ) {
    *out_x = global_win32_input->mb_dx;
    *out_y = global_win32_input->mb_dy;
}
attr_media_api _Bool input_gamepad_query_state(
    uint32_t index, GamepadState* out_state
) {
    if( index >= XUSER_MAX_COUNT || !global_win32_input->gp_connected[index] ) {
        return false;
    }

    memcpy( out_state, global_win32_input->gp + index, sizeof(*out_state) );
    return true;
}
attr_media_api _Bool input_gamepad_rumble_set(
    uint32_t index, uint16_t motor_left, uint16_t motor_right
) {
    if( index >= XUSER_MAX_COUNT || !global_win32_input->gp_connected[index] ) {
        return false;
    }
    XINPUT_VIBRATION vib;
    vib.wLeftMotorSpeed  = motor_left;
    vib.wRightMotorSpeed = motor_right;
    if( XInputSetState( index, &vib ) == ERROR_DEVICE_NOT_CONNECTED ) {
        global_win32_input->gp_connected[index] = false;
        return false;
    }
    return true;
}

LRESULT win32_winproc_input( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    BYTE lpb[sizeof(RAWINPUT)];
    memset( lpb, 0, sizeof(lpb) );
    UINT pcbSize = sizeof(lpb);

    UINT res = GetRawInputData(
        (HRAWINPUT)lparam, RID_INPUT, lpb, &pcbSize, sizeof(RAWINPUTHEADER) );
    if( res == (UINT)-1 ) {
        return DefWindowProcW( hwnd, msg, wparam, lparam );
    }

    RAWINPUT* raw = (RAWINPUT*)lpb;
    switch( raw->header.dwType ) {
        case RIM_TYPEKEYBOARD: {
            RAWKEYBOARD* kb = &raw->data.keyboard;

            if( kb->MakeCode == KEYBOARD_OVERRUN_MAKE_CODE ) {
                break;
            }
            /* uint16_t scan = kb->MakeCode; */
            uint16_t vk   = kb->VKey;

            _Bool is_e0 = ((kb->Flags & RI_KEY_E0) != 0);
            _Bool is_e1 = ((kb->Flags & RI_KEY_E1) != 0);
            unused(is_e1);

            uint16_t vk_translate   = vk;

            _Bool down           = !(kb->Flags & RI_KEY_BREAK);
            uint32_t scan_translate = MapVirtualKeyW( vk, MAPVK_VK_TO_VSC );

            switch( vk ) {
                case VK_PAUSE: {
                    scan_translate = 0x45;
                } break;
                case VK_CONTROL: {
                    if( is_e0 ) {
                        vk_translate = VK_RCONTROL;
                    } else {
                        vk_translate = VK_LCONTROL;
                    }

                    global_win32_state->mod = down ?
                        (global_win32_state->mod | KBMOD_CTRL) :
                        (global_win32_state->mod & ~KBMOD_CTRL);
                } break;
                case VK_MENU: {
                    if( is_e0 ) {
                        vk_translate = VK_RMENU;
                    } else {
                        vk_translate = VK_LMENU;
                    }
                    global_win32_state->mod = down ?
                        (global_win32_state->mod | KBMOD_ALT) :
                        (global_win32_state->mod & ~KBMOD_ALT);
                } break;
                case VK_SHIFT: {
                    vk_translate = MapVirtualKeyW( scan_translate, MAPVK_VSC_TO_VK_EX );

                    global_win32_state->mod = down ?
                        (global_win32_state->mod | KBMOD_SHIFT) :
                        (global_win32_state->mod & ~KBMOD_SHIFT);
                } break;
                case VK_CAPITAL: {
                    if( down ) {
                        global_win32_state->mod =
                            (global_win32_state->mod & KBMOD_CAPSLK) ?
                                (global_win32_state->mod & ~KBMOD_CAPSLK) :
                                (global_win32_state->mod |  KBMOD_CAPSLK);
                    }
                } break;
                case VK_SCROLL: {
                    if( down ) {
                        global_win32_state->mod =
                            (global_win32_state->mod & KBMOD_SCRLK) ?
                                (global_win32_state->mod & ~KBMOD_SCRLK) :
                                (global_win32_state->mod |  KBMOD_SCRLK);
                    }
                } break;
                case VK_NUMLOCK: {
                    scan_translate = (MapVirtualKeyW( vk, MAPVK_VK_TO_VSC ) | 0x100);
                    if( down ) {
                        global_win32_state->mod =
                            (global_win32_state->mod & KBMOD_NUMLK) ?
                                (global_win32_state->mod & ~KBMOD_NUMLK) :
                                (global_win32_state->mod |  KBMOD_NUMLK);
                    }
                } break;
            }

            KeyboardCode code = vk_to_keyboard_code( vk_translate );

            keyboard_state_set_key( &global_win32_input->kb, code, down );

            HWND focused = win32_get_focused_window();
            if( focused ) {
                WPARAM _wparam = win32_key_to_wparam( code, down );
                PostMessageW(
                    focused, WM_CUSTOM_KEYBOARD, _wparam, 0 );
            }
        } break;
        case RIM_TYPEMOUSE: {
            RAWMOUSE* mb = &raw->data.mouse;
            WPARAM dx = win32_mouse_x_to_wparam( mb->lLastX );
            LPARAM dy = win32_mouse_y_to_lparam( mb->lLastY );

            uint16_t    flags = mb->usButtonFlags;
            MouseButton btn   = global_win32_state->mb;

            if( (flags & RI_MOUSE_LEFT_BUTTON_DOWN) == RI_MOUSE_LEFT_BUTTON_DOWN ) {
                btn |= MB_LEFT;
            } else if( (flags & RI_MOUSE_LEFT_BUTTON_UP) == RI_MOUSE_LEFT_BUTTON_UP ) {
                btn &= ~MB_LEFT;
            }

            if( (flags & RI_MOUSE_RIGHT_BUTTON_DOWN) == RI_MOUSE_RIGHT_BUTTON_DOWN ) {
                btn |= MB_RIGHT;
            } else if(
                (flags & RI_MOUSE_RIGHT_BUTTON_UP) == RI_MOUSE_RIGHT_BUTTON_UP
            ) {
                btn &= ~MB_RIGHT;
            }

            if( (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN) == RI_MOUSE_MIDDLE_BUTTON_DOWN ) {
                btn |= MB_MIDDLE;
            } else if(
                (flags & RI_MOUSE_MIDDLE_BUTTON_UP) == RI_MOUSE_MIDDLE_BUTTON_UP 
            ) {
                btn &= ~MB_MIDDLE;
            }

            if( (flags & RI_MOUSE_BUTTON_4_DOWN) == RI_MOUSE_BUTTON_4_DOWN ) {
                btn |= MB_EXTRA_1;
            } else if( (flags & RI_MOUSE_BUTTON_4_UP) == RI_MOUSE_BUTTON_4_UP ) {
                btn &= ~MB_EXTRA_1;
            }

            if( (flags & RI_MOUSE_BUTTON_5_DOWN) == RI_MOUSE_BUTTON_5_DOWN ) {
                btn |= MB_EXTRA_2;
            } else if( (flags & RI_MOUSE_BUTTON_5_UP) == RI_MOUSE_BUTTON_5_UP ) {
                btn &= ~MB_EXTRA_2;
            }

            int16_t scroll = *(int16_t*)&mb->usButtonData;
            scroll = scroll < 0 ? -1 : 1;
            _Bool scroll_hor = (flags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL;

            MouseButton delta = btn ^ global_win32_state->mb;
            global_win32_state->mb = btn;

            WPARAM buttons = win32_mouse_button_to_wparam(
                btn, delta, scroll, scroll_hor );

            global_win32_input->mb_dx = mb->lLastX;
            global_win32_input->mb_dy = mb->lLastY;

            HWND focused = win32_get_focused_window();
            if( focused ) {
                PostMessageW( focused, WM_CUSTOM_MOUSE_DEL, dx, dy );
                PostMessageW( focused, WM_CUSTOM_MOUSE_BTN, buttons, 0 );
            }
        } break;
        default: break;
    }

    if( GET_RAWINPUT_CODE_WPARAM(wparam) == RIM_INPUT ) {
        return DefWindowProcW( hwnd, msg, wparam, lparam );
    }
    return 0;
}
DWORD keyboard_code_to_vk( KeyboardCode code ) {
    switch( code ) {
        case KB_BACKSPACE                 : return VK_BACK;
        case KB_TAB                       : return VK_TAB;
        case KB_ENTER                     : return VK_RETURN;
        case KB_SHIFT_LEFT                : return VK_SHIFT;
        case KB_SHIFT_RIGHT               : return VK_RSHIFT;
        case KB_CONTROL_LEFT              : return VK_CONTROL;
        case KB_CONTROL_RIGHT             : return VK_RCONTROL;
        case KB_ALT_LEFT                  : return VK_MENU;
        case KB_ALT_RIGHT                 : return VK_RMENU;
        case KB_PAUSE                     : return VK_PAUSE;
        case KB_CAPSLOCK                  : return VK_CAPITAL;
        case KB_ESCAPE                    : return VK_ESCAPE;
        case KB_SPACE                     : return VK_SPACE;
        case KB_PAGE_UP                   : return VK_PRIOR;
        case KB_PAGE_DOWN                 : return VK_NEXT;
        case KB_END                       : return VK_END;
        case KB_HOME                      : return VK_HOME;
        case KB_ARROW_LEFT                : return VK_LEFT;
        case KB_ARROW_UP                  : return VK_UP;
        case KB_ARROW_RIGHT               : return VK_RIGHT;
        case KB_ARROW_DOWN                : return VK_DOWN;
        case KB_0 ... KB_9                : return (code - KB_0) + 0x30;
        case KB_A ... KB_Z                : return (code - KB_A) + 0x41;
        case KB_SUPER_LEFT                : return VK_LWIN;
        case KB_SUPER_RIGHT               : return VK_RWIN;
        case KB_PAD_0 ... KB_PAD_9        : return (code - KB_PAD_0) + VK_NUMPAD0;
        case KB_F1 ... KB_F24             : return (code - KB_F1) + VK_F1;
        case KB_NUM_LOCK                  : return VK_NUMLOCK;
        case KB_SCROLL_LOCK               : return VK_SCROLL;
        case KB_SEMICOLON                 : return VK_OEM_1;
        case KB_EQUALS                    : return VK_OEM_PLUS;
        case KB_COMMA                     : return VK_OEM_COMMA;
        case KB_MINUS                     : return VK_OEM_MINUS;
        case KB_PERIOD                    : return VK_OEM_PERIOD;
        case KB_SLASH                     : return VK_OEM_2;
        case KB_BACKTICK                  : return VK_OEM_3;
        case KB_BRACKET_LEFT              : return VK_OEM_4;
        case KB_BACKSLASH                 : return VK_OEM_5;
        case KB_BRACKET_RIGHT             : return VK_OEM_6;
        case KB_QUOTE                     : return VK_OEM_7;
        case KB_PRINT_SCREEN              : return VK_SNAPSHOT;
        case KB_DELETE                    : return VK_DELETE;
        case KB_PAD_ADD                   : return VK_ADD;
        case KB_PAD_MULTIPLY              : return VK_MULTIPLY;
        case KB_PAD_SUBTRACT              : return VK_SUBTRACT;
        case KB_PAD_DIVIDE                : return VK_DIVIDE;
        case KB_PAD_DOT                   : return VK_DECIMAL;
        case KB_INSERT                    : return VK_INSERT;
        case KB_RIGHT_CLICK_MENU          : return VK_APPS;
        default: return 0;
    }
}
KeyboardCode vk_to_keyboard_code( DWORD vk ) {
    switch( vk ) {
        case VK_BACK                   : return KB_BACKSPACE;
        case VK_TAB                    : return KB_TAB;
        case VK_RETURN                 : return KB_ENTER;
        case VK_LSHIFT:
        case VK_SHIFT                  : return KB_SHIFT_LEFT;
        case VK_RSHIFT                 : return KB_SHIFT_RIGHT;
        case VK_LCONTROL:
        case VK_CONTROL                : return KB_CONTROL_LEFT;
        case VK_RCONTROL               : return KB_CONTROL_RIGHT;
        case VK_LMENU:
        case VK_MENU                   : return KB_ALT_LEFT;
        case VK_RMENU                  : return KB_ALT_RIGHT;
        case VK_PAUSE                  : return KB_PAUSE;
        case VK_CAPITAL                : return KB_CAPSLOCK;
        case VK_ESCAPE                 : return KB_ESCAPE;
        case VK_SPACE                  : return KB_SPACE;
        case VK_PRIOR                  : return KB_PAGE_UP;
        case VK_NEXT                   : return KB_PAGE_DOWN;
        case VK_END                    : return KB_END;
        case VK_HOME                   : return KB_HOME;
        case VK_LEFT                   : return KB_ARROW_LEFT;
        case VK_UP                     : return KB_ARROW_UP;
        case VK_RIGHT                  : return KB_ARROW_RIGHT;
        case VK_DOWN                   : return KB_ARROW_DOWN;
        case 0x30 ... 0x39             : return (vk - 0x30) + KB_0;
        case 0x41 ... 0x5A             : return (vk - 0x41) + KB_A;
        case VK_LWIN                   : return KB_SUPER_LEFT;
        case VK_RWIN                   : return KB_SUPER_RIGHT;
        case VK_NUMPAD0 ... VK_NUMPAD9 : return (vk - VK_NUMPAD0) + KB_PAD_0;
        case VK_F1 ... VK_F24          : return (vk - VK_F1) + KB_F1;
        case VK_NUMLOCK                : return KB_NUM_LOCK;
        case VK_SCROLL                 : return KB_SCROLL_LOCK;
        case VK_OEM_1                  : return KB_SEMICOLON;
        case VK_OEM_PLUS               : return KB_EQUALS;
        case VK_OEM_COMMA              : return KB_COMMA;
        case VK_OEM_MINUS              : return KB_MINUS;
        case VK_OEM_PERIOD             : return KB_PERIOD;
        case VK_OEM_2                  : return KB_SLASH;
        case VK_OEM_3                  : return KB_BACKTICK;
        case VK_OEM_4                  : return KB_BRACKET_LEFT;
        case VK_OEM_5                  : return KB_BACKSLASH;
        case VK_OEM_6                  : return KB_BRACKET_RIGHT;
        case VK_OEM_7                  : return KB_QUOTE;
        case VK_SNAPSHOT               : return KB_PRINT_SCREEN;
        case VK_DELETE                 : return KB_DELETE;
        case VK_ADD                    : return KB_PAD_ADD;
        case VK_MULTIPLY               : return KB_PAD_MULTIPLY;
        case VK_SUBTRACT               : return KB_PAD_SUBTRACT;
        case VK_DIVIDE                 : return KB_PAD_DIVIDE;
        case VK_DECIMAL                : return KB_PAD_DOT;
        case VK_INSERT                 : return KB_INSERT;
        case VK_APPS                   : return KB_RIGHT_CLICK_MENU;
        default                        : return KB_UNKNOWN;
    }
}

#undef def
#endif /* Platform Windows */

