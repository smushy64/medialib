/**
 * @file   input.c
 * @brief  Input handling in win32 implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   April 21, 2024
*/
#include "impl/win32/input.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "media/internal/logging.h"
#include "impl/win32/common.h"

#include "core/prelude.h"
#include "core/memory.h"
#include "core/math/macros.h"

#include <xinput.h>
#include <hidusage.h>

#if defined(MEDIA_ENABLE_LOGGING) && defined(CORE_ENABLE_ASSERTIONS)
#define media_assert( condition, format, ... ) do {\
    if( !(condition) ) {\
        media_error( "assertion failed: "\
            #condition " message: " format, ##__VA_ARGS__ );\
        panic();\
    }\
} while(0)
#else /* Logging && Assertions Enabled */
#define media_assert( ... ) unused( __VA_ARGS__ )
#endif /* Logging && Assertions Disabled */

volatile struct Win32InputState* global_media_win32_input_state = NULL;

attr_internal InputKeycode win32_vk_to_keycode( DWORD vk );

attr_media_api usize media_input_query_memory_requirement(void) {
    return sizeof(struct Win32InputState);
}
attr_media_api b32  media_input_initialize( void* buffer ) {
    if( !buffer ) {
        media_error( "Attempted to initialize input with NULL buffer!" );
        return false;
    }
    global_media_win32_input_state = buffer;

    /* Initialize Lock Keys State */ {
        b32 caps = GetKeyState( VK_CAPITAL ) & 0x0001;
        global_media_win32_input_state->keymod = caps ? INPUT_KEYMOD_CAPSLK : 0;

        global_media_win32_input_state->keys_vk[VK_CAPITAL] = caps ? 0x0001 : 0;

        global_media_win32_input_state->keymod |=
            (GetKeyState( VK_SCROLL ) & 0x0001) ? INPUT_KEYMOD_SCRLK : 0;
        global_media_win32_input_state->keymod |=
            (GetKeyState( VK_NUMLOCK ) & 0x0001) ? INPUT_KEYMOD_NUMLK : 0;
    }

    HMODULE module = GetModuleHandleA( NULL );

    /* create window */ {
        WNDCLASSEXA message_class   = global_win32_state->def_wndclass;
        message_class.lpszClassName = "LibMediaMessageWindowClass";
        message_class.lpfnWndProc   = win32_message_proc;

        if( !RegisterClassExA( &message_class ) ) {
            win32_error( "failed to register message window class!" );
            return false;
        }

        HWND message_window = CreateWindowExA(
            0, message_class.lpszClassName, NULL,
            0, 0, 0, 0, 0, HWND_MESSAGE, NULL, module, NULL  );
        if( !message_window ) {
            win32_error( "failed to create message window!" );
            UnregisterClassA( message_class.lpszClassName, module );
            return false;
        }

        global_media_win32_input_state->msg_wnd       = message_window;
        global_media_win32_input_state->msg_wnd_class = message_class;
    }

    /* Register Raw Input */ {
        RAWINPUTDEVICE rid[2];
        rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[0].usUsage     = HID_USAGE_GENERIC_MOUSE;
        rid[0].dwFlags     = RIDEV_INPUTSINK;
        rid[0].hwndTarget  = global_media_win32_input_state->msg_wnd;

        rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[1].usUsage     = HID_USAGE_GENERIC_KEYBOARD;
        rid[1].dwFlags     = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
        rid[1].hwndTarget  = global_media_win32_input_state->msg_wnd;
        if( RegisterRawInputDevices(
            rid, static_array_len(rid), sizeof( rid[0] )
        ) == FALSE ) {
            win32_error( "failed to register raw input devices!" );
            DestroyWindow( global_media_win32_input_state->msg_wnd );
            UnregisterClassA(
                global_media_win32_input_state->msg_wnd_class.lpszClassName, module );
            return false;
        }
    }

    return true;
}
attr_media_api void media_input_update(void) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to update media input without "
        "first initializing media input!" );

    XINPUT_STATE xinput;
    for( u32 i = 0; i < INPUT_GAMEPAD_COUNT; ++i ) {
        // TODO(alicia): avoid XInputGetState stall on inactive gamepad!
        if( XInputGetState( i, &xinput ) != ERROR_SUCCESS ) {
            global_media_win32_input_state->active_gamepads[i] = false;
            global_media_win32_input_state->gamepad_rumble[i][0] = 0;
            global_media_win32_input_state->gamepad_rumble[i][1] = 0;
            continue;
        }
        global_media_win32_input_state->active_gamepads[i] = true;
        InputGamepadState* state =
            (InputGamepadState*)global_media_win32_input_state->gp + i;

        XINPUT_GAMEPAD* pad = &xinput.Gamepad;

        state->buttons = bitfield_clear( pad->wButtons,
            INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_LEFT |
            INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_RIGHT );

        state->buttons |= pad->bLeftTrigger > (U8_MAX / 2) ?
            INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_LEFT : 0;
        state->buttons |= pad->bRightTrigger > (U8_MAX / 2) ?
            INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_RIGHT : 0;

        state->trigger_left  = pad->bLeftTrigger;
        state->trigger_right = pad->bRightTrigger;

        state->stick_left_x  = pad->sThumbLX;
        state->stick_left_y  = pad->sThumbLY;
        state->stick_right_x = pad->sThumbRX;
        state->stick_right_y = pad->sThumbRY;
    }

    global_media_win32_input_state->active = win32_get_active_window();

    if( global_media_win32_input_state->active != NULL ) {
        POINT new_pos;
        GetCursorPos( &new_pos );
        if(
            new_pos.x != global_media_win32_input_state->abs_x ||
            new_pos.y != global_media_win32_input_state->abs_y
        ) {
            PostMessageA(
                global_media_win32_input_state->active, WM_INPUT_MOUSE_POSITION,
                win32_make_mouse_pos_wparam( new_pos.x ),
                win32_make_mouse_pos_lparam( new_pos.y ) );
            global_media_win32_input_state->abs_x = new_pos.x;
            global_media_win32_input_state->abs_y = new_pos.y;
        }
    }

    MSG message = {};
    // win32_message_proc should only process WM_INPUT messages.
    while( PeekMessageA(
        &message, global_media_win32_input_state->msg_wnd,
        WM_INPUT, WM_INPUT, PM_REMOVE
    ) ) {
        DispatchMessageA( &message );
    }

}
attr_media_api void media_input_shutdown(void) {
    if( !global_media_win32_input_state ) {
        return;
    }
    HMODULE module = GetModuleHandleA( NULL );
    DestroyWindow( global_media_win32_input_state->msg_wnd );
    UnregisterClassA(
        global_media_win32_input_state->msg_wnd_class.lpszClassName, module );
    memory_zero(
        (void*)global_media_win32_input_state,
        sizeof(*global_media_win32_input_state) );
    global_media_win32_input_state = NULL;
}
attr_media_api InputKeymod media_keyboard_query_mod(void) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query key mod state without "
        "first initializing media input!" );
    return global_media_win32_input_state->keymod;
}
attr_media_api b32 media_keyboard_query_key( InputKeycode key ) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query key state without "
        "first initializing media input!" );
    return packed_bool_get( global_media_win32_input_state->keys, key );
}
attr_media_api void media_keyboard_query_keyboard(
    b8 out_keyboard[packed_bool_memory_requirement(INPUT_KEYCODE_COUNT)]
) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query keyboard state without "
        "first initializing media input!" );
    memory_copy(
        out_keyboard, (const void*)global_media_win32_input_state->keys,
        sizeof(global_media_win32_input_state->keys) );
}
attr_media_api InputMouseButton media_mouse_query_buttons(void) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query mouse button state without "
        "first initializing media input!" );
    return global_media_win32_input_state->mb;
}
attr_media_api void media_mouse_query_absolute( i32* out_x, i32* out_y ) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query mouse absolute position without "
        "first initializing media input!" );
    if( out_x ) {
        *out_x = global_media_win32_input_state->abs_x;
    }
    if( out_y ) {
        *out_y = global_media_win32_input_state->abs_y;
    }
}
attr_media_api void media_mouse_query_delta( i32* out_x, i32* out_y ) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query mouse delta position without "
        "first initializing media input!" );
    if( out_x ) {
        *out_x = global_media_win32_input_state->del_x;
    }
    if( out_y ) {
        *out_y = global_media_win32_input_state->del_y;
    }
}
attr_media_api b32 media_gamepad_query_state(
    u32 index, InputGamepadState* out_state
) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to query gamepad state without "
        "first initializing media input!" );
    media_assert(
        index < INPUT_GAMEPAD_COUNT,
        "Attempted to query gamepad state at an invalid index! Index: {u}", index );
    if( !global_media_win32_input_state->active_gamepads[index] ) {
        return false;
    }

    memory_copy( out_state,
        (const void*)(global_media_win32_input_state->gp + index),
        sizeof(*out_state) );
    return true;
}
attr_media_api void media_gamepad_set_rumble(
    u32 index, u16 motor_left, u16 motor_right
) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to set gamepad rumble without "
        "first initializing media input!" );
    media_assert(
        index < INPUT_GAMEPAD_COUNT,
        "Attempted to set rumble for gamepad at an invalid index! Index: {u}", index );
#if defined(MEDIA_ENABLE_LOGGING)
    if( !global_media_win32_input_state->active_gamepads[index] ) {
        media_warn(
            "Attempted to set rumble for gamepad "
            "'{u}' but it's not available!", index );
        return;
    }
#endif
    global_media_win32_input_state->gamepad_rumble[index][0] = motor_left;
    global_media_win32_input_state->gamepad_rumble[index][1] = motor_right;

    XINPUT_VIBRATION vib = { motor_left, motor_right };
    XInputSetState( index, &vib );
}
attr_media_api void media_gamepad_query_rumble(
    u32 index, u16* out_motor_left, u16* out_motor_right
) {
    media_assert(
        global_media_win32_input_state,
        "Attempted to set gamepad rumble without "
        "first initializing media input!" );
    media_assert(
        index < INPUT_GAMEPAD_COUNT,
        "Attempted to set rumble for gamepad at an invalid index! Index: {u}", index );

    if( out_motor_left ) {
        *out_motor_left = global_media_win32_input_state->gamepad_rumble[index][0];
    }
    if( out_motor_right ) {
        *out_motor_right = global_media_win32_input_state->gamepad_rumble[index][1];
    }
}

LRESULT win32_message_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
    volatile struct Win32InputState* state = global_media_win32_input_state;

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
                        state->keymod |= INPUT_KEYMOD_SHIFT;
                    } break;
                    case VK_CONTROL: {
                        state->keymod |= INPUT_KEYMOD_CTRL;
                    } break;
                    case VK_MENU: {
                        state->keymod |= INPUT_KEYMOD_ALT;
                    } break;
                    case VK_CAPITAL: {
                        state->keymod =
                            bitfield_check(
                                state->keymod, INPUT_KEYMOD_CAPSLK ) ?
                            bitfield_clear(
                                state->keymod, INPUT_KEYMOD_CAPSLK ) :
                            bitfield_set(
                                state->keymod, INPUT_KEYMOD_CAPSLK );
                    } break;
                    case VK_SCROLL: {
                        state->keymod =
                            bitfield_check(
                                state->keymod, INPUT_KEYMOD_SCRLK ) ?
                            bitfield_clear(
                                state->keymod, INPUT_KEYMOD_SCRLK ) :
                            bitfield_set(
                                state->keymod, INPUT_KEYMOD_SCRLK );
                    } break;
                    case VK_NUMLOCK: {
                        state->keymod =
                            bitfield_check(
                                state->keymod, INPUT_KEYMOD_NUMLK ) ?
                            bitfield_clear(
                                state->keymod, INPUT_KEYMOD_NUMLK ) :
                            bitfield_set(
                                state->keymod, INPUT_KEYMOD_NUMLK );
                    } break;
                    default: break;
                }
            } else if( up ) {
                switch( kb->VKey ) {
                    case VK_SHIFT: {
                        state->keymod =
                            bitfield_clear(
                                state->keymod, INPUT_KEYMOD_SHIFT );
                    } break;
                    case VK_CONTROL: {
                        state->keymod =
                            bitfield_clear(
                                state->keymod, INPUT_KEYMOD_CTRL );
                    } break;
                    case VK_MENU: {
                        state->keymod =
                            bitfield_clear(
                                state->keymod, INPUT_KEYMOD_ALT );
                    } break;
                    default: break;
                }
                
            }

            InputKeycode key  = win32_vk_to_keycode( vk );
            b32 send_keyboard = true;

            if( down ) {
                if( packed_bool_get( state->keys, key ) ) {
                    send_keyboard = false;
                }
                packed_bool_set( (b8*)state->keys, key, true );
            } else if( up ) {
                packed_bool_set( (b8*)state->keys, key, false );
            } else {
                send_keyboard = false;
            }
            if( key == INPUT_KEYCODE_UNKNOWN ) {
                send_keyboard = false;
            }

            if( down ) {
                state->keys_vk[vk] = 1 << 7;
            } else if( up ) {
                state->keys_vk[vk] = 0;
            }
            switch( vk ) {
                case VK_SHIFT:
                case VK_LSHIFT:
                case VK_RSHIFT: {
                    state->keys_vk[VK_SHIFT]  = state->keys_vk[vk];
                    state->keys_vk[VK_LSHIFT] = state->keys_vk[vk];
                    state->keys_vk[VK_RSHIFT] = state->keys_vk[vk];
                } break;
                case VK_CONTROL:
                case VK_LCONTROL:
                case VK_RCONTROL: {
                    state->keys_vk[VK_CONTROL]  = state->keys_vk[vk];
                    state->keys_vk[VK_LCONTROL] = state->keys_vk[vk];
                    state->keys_vk[VK_RCONTROL] = state->keys_vk[vk];
                } break;
                case VK_MENU:
                case VK_LMENU:
                case VK_RMENU: {
                    state->keys_vk[VK_MENU]  = state->keys_vk[vk];
                    state->keys_vk[VK_LMENU] = state->keys_vk[vk];
                    state->keys_vk[VK_RMENU] = state->keys_vk[vk];
                } break;
                case VK_CAPITAL: {
                    if( bitfield_check(
                        state->keymod, INPUT_KEYMOD_CAPSLK
                    ) ) {
                        state->keys_vk[vk] |= 1 << 0;
                    } else {
                        state->keys_vk[vk] &= ~(1 << 0);
                    }
                } break;
                default: break;
            }

            if( state->active ) {
                if( send_keyboard ) {
                    WPARAM w = win32_make_key_wparam( key, down );
                    PostMessageA( state->active, WM_INPUT_KEYBOARD, w, 0 );
                }
                if( down ) {
                    WPARAM w = win32_make_text_wparam( vk, scan_translate );
                    LPARAM l = win32_make_text_lparam( state->keys_vk );
                    PostMessageA(
                        state->active, WM_INPUT_KEYBOARD_TEXT, w, l );
                }
            }
        } break;
        case RIM_TYPEMOUSE: {
            RAWMOUSE* mb = &raw->data.mouse;
            WPARAM rel_x = win32_make_mouse_pos_wparam( mb->lLastX );
            LPARAM rel_y = win32_make_mouse_pos_lparam( mb->lLastY );

            u16 flags = mb->usButtonFlags;
            u8 buttons_state = state->mb;
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

            u8 buttons_changed = state->mb ^ buttons_state;
            state->mb = buttons_state;

            i16 scroll = signum( rcast( i16, &mb->usButtonData ) );
            WPARAM buttons = win32_make_mouse_button_wparam(
                buttons_state, buttons_changed,
                scroll, bitfield_check( mb->usButtonFlags, RI_MOUSE_HWHEEL ) );

            if( state->active ) {
                PostMessageA(
                    state->active, WM_INPUT_MOUSE_POSITION_RELATIVE, rel_x, rel_y );
            }
            if( buttons_changed || scroll ) {
                PostMessageA(
                    state->active, WM_INPUT_MOUSE_BUTTON,
                    buttons, 0 );
            }

            state->del_x = rel_x;
            state->del_y = rel_y;
        } break;
    }

    if( GET_RAWINPUT_CODE_WPARAM( wparam ) == RIM_INPUT ) {
        return DefWindowProcA( hwnd, msg, wparam, lparam );
    }
    return 0;
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

