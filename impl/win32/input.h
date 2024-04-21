#if !defined(MEDIA_IMPL_WIN32_INPUT_H)
#define MEDIA_IMPL_WIN32_INPUT_H
/**
 * @file   input.h
 * @brief  Input handling for win32.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   April 21, 2024
*/
#include "core/defines.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "media/input.h"
#include "core/collections.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

struct Win32InputState {
    b8               active_gamepads[INPUT_GAMEPAD_COUNT];
    u16              gamepad_rumble[INPUT_GAMEPAD_COUNT][2];
    InputKeymod      keymod;
    b8               keys[packed_bool_memory_requirement(INPUT_KEYCODE_COUNT)];
    HWND             active;
    HWND             msg_wnd;
    WNDCLASSEXA      msg_wnd_class;
    BYTE             keys_vk[256];
    i32 abs_x, abs_y;
    i32 del_x, del_y;
    InputMouseButton mb;
    InputGamepadState gp[INPUT_GAMEPAD_COUNT];
};
extern volatile struct Win32InputState* global_media_win32_input_state;

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

#define WM_INPUT_KEYBOARD                (WM_USER + 1)
#define WM_INPUT_KEYBOARD_TEXT           (WM_USER + 2)
#define WM_INPUT_MOUSE_POSITION          (WM_USER + 3)
#define WM_INPUT_MOUSE_POSITION_RELATIVE (WM_USER + 4)
#define WM_INPUT_MOUSE_BUTTON            (WM_USER + 5)

#define WIN32_MESSAGE_WINDOW_RESULT_PENDING  (0)
#define WIN32_MESSAGE_WINDOW_RESULT_SUCCESS  (1)
#define WIN32_MESSAGE_WINDOW_RESULT_ERROR    (2)
#define WIN32_MESSAGE_WINDOW_RESULT_FINISHED (3)

#endif /* Platform Windows */

#endif /* header guard */
