#if !defined(MEDIA_IMPL_WIN32_INPUT_H)
#define MEDIA_IMPL_WIN32_INPUT_H
/**
 * @file   input.h
 * @brief  Media Windows Input.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 13, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/input.h"
#include "impl/win32/common.h"

struct Win32Input {
    KeyboardState kb;
    m_int32       mb_x, mb_y, mb_dx, mb_dy;
    m_uint16      rumble[GAMEPAD_MAX_COUNT][2];
    m_bool32      gp_connected[GAMEPAD_MAX_COUNT];
    GamepadState  gp[GAMEPAD_MAX_COUNT];
    HWND          hwnd;
    HANDLE        thread;
    volatile long thread_exit;
};
struct Win32KeyWParam {
    m_uint16 keycode;
    m_bool16 is_down;
#if defined(MEDIA_ARCH_64_BIT)
    m_uint32 __padding;
#endif
};
struct Win32MousePositionParam {
    m_int32 v;
#if defined(MEDIA_ARCH_64_BIT)
    m_uint32 __padding;
#endif
};
struct Win32MouseButtonWParam {
    m_uint8 state;
    m_uint8 delta;
    m_int8  scroll;
    m_bool8 is_scroll_horizontal;
#if defined(MEDIA_ARCH_64_BIT)
    m_uint32 __padding;
#endif
};

#define WM_CUSTOM_KEYBOARD  (WM_USER + 1)
#define WM_CUSTOM_MOUSE_POS (WM_USER + 2)
#define WM_CUSTOM_MOUSE_DEL (WM_USER + 3)
#define WM_CUSTOM_MOUSE_BTN (WM_USER + 4)

#define win32_key_to_wparam( kc, down )\
    (*(WPARAM*)(&(struct Win32KeyWParam){.keycode=kc,.is_down=down}))
#define win32_key_from_wparam( wparam )\
    (*(struct Win32KeyWParam*)(&(wparam)))

#define win32_mouse_x_to_wparam( x )\
    (*(WPARAM*)(&(struct Win32MousePositionParam){.v=x}))
#define win32_mouse_x_from_wparam( wparam )\
    (*(struct Win32MousePositionParam*)&(wparam))
#define win32_mouse_y_to_lparam( y )\
    (*(WPARAM*)(&(struct Win32MousePositionParam){.v=y}))
#define win32_mouse_y_from_lparam( wparam )\
    (*(struct Win32MousePositionParam*)&(wparam))

#define win32_mouse_button_to_wparam( _state, _delta, _scroll, _scroll_hor )\
    (*(WPARAM*)(&(struct Win32MouseButtonWParam){\
        .state=_state,.delta=_delta,\
        .scroll=_scroll,.is_scroll_horizontal=_scroll_hor}))
#define win32_mouse_button_from_wparam( wparam )\
    (*(struct Win32MouseButtonWParam*)(&(wparam)))

struct Win32Input* win32_input(void);
DWORD        keyboard_code_to_vk( KeyboardCode code );
KeyboardCode vk_to_keyboard_code( DWORD vk );

#endif /* Platform Windows */
#endif /* header guard */
