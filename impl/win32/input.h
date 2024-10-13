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
    int32_t       mb_x, mb_y, mb_dx, mb_dy;
    uint16_t      rumble[GAMEPAD_MAX_COUNT][2];
    uint8_t       gp_connected[GAMEPAD_MAX_COUNT];
    GamepadState  gp[GAMEPAD_MAX_COUNT];
    HWND          hwnd;
    HANDLE        thread;
    volatile long thread_exit;
};
struct Win32KeyWParam {
    uint16_t keycode;
    uint16_t is_down;
#if defined(MEDIA_ARCH_64_BIT)
    uint32_t __padding;
#endif
};
struct Win32MousePositionParam {
    int32_t v;
#if defined(MEDIA_ARCH_64_BIT)
    uint32_t __padding;
#endif
};
struct Win32MouseButtonWParam {
    uint8_t state;
    uint8_t delta;
    int8_t  scroll;
    uint8_t is_scroll_horizontal;
#if defined(MEDIA_ARCH_64_BIT)
    uint32_t __padding;
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
