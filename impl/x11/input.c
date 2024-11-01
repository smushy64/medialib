/**
 * @file   input.c
 * @brief  
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 22, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_LINUX)
#include "media/input.h"
#include "media/surface.h"
#include "media/cursor.h"
#include "media/internal/logging.h"
#include "impl/x11/common.h"
#include "impl/x11/surface.h"
#include "impl/x11/input.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xfixes.h>

struct X11Input {
    KeyboardState kb;
    struct {
        int32_t x, y, dx, dy;
    } mb;
    uint8_t      gp_connected[GAMEPAD_MAX_COUNT];
    GamepadState gp[GAMEPAD_MAX_COUNT];

    Display* display;
    Window   window;
};
struct X11Input* global_x11_input = NULL;

attr_media_api uintptr_t input_subsystem_query_memory_requirement(void) {
    // TODO(alicia): 
    return sizeof(struct X11Input);
}
attr_media_api _Bool input_subsystem_initialize( void* buffer ) {
    // TODO(alicia): 
    unused(buffer);
    return true;
}
attr_media_api void input_subsystem_update(void) {
    // TODO(alicia): 
}
attr_media_api void input_subsystem_shutdown(void) {
    // TODO(alicia): 
}

attr_media_api KeyboardMod input_keyboard_query_mod(void) {
    // TODO(alicia): 
    return 0;
}
attr_media_api _Bool input_keyboard_query_key( KeyboardCode keycode ) {
    // TODO(alicia): 
    unused(keycode);
    return false;
}
attr_media_api void input_keyboard_copy_state( KeyboardState* out_state ) {
    // TODO(alicia): 
    unused(out_state);
}
attr_media_api MouseButton input_mouse_query_buttons(void) {
    // TODO(alicia): 
    return 0;
}
attr_media_api void input_mouse_query_position( int32_t* out_x, int32_t* out_y ) {
    // TODO(alicia): 
    unused(out_x,out_y);
}
attr_media_api void input_mouse_position_to_client(
    SurfaceHandle* surface, int32_t* in_out_x, int32_t* in_out_y
) {
    // TODO(alicia): 
    unused(surface,in_out_x,in_out_y);
}
attr_media_api void input_mouse_query_delta( int32_t* out_x, int32_t* out_y ) {
    // TODO(alicia): 
    unused(out_x,out_y);
}

attr_media_api _Bool input_gamepad_query_state(
    uint32_t index, GamepadState* out_state
) {
    // TODO(alicia): 
    unused(index,out_state);
    return false;
}
attr_media_api _Bool input_gamepad_rumble_set(
    uint32_t index, uint16_t motor_left, uint16_t motor_right
) {
    // TODO(alicia): 
    unused(index,motor_left,motor_right);
    return false;
}


attr_media_api void cursor_type_set( SurfaceHandle* surface, CursorType cursor ) {
    unused( surface, cursor );
    // <X11/Xcursor/Xcursor.h>
    /*

    Arrow                   : "arrow"
    I-beam (Text cursor)    : "xterm"
    Crosshair               : "cross"
    Hand                    : "hand2"
    Resize Horizontal       : "sb_h_double_arrow"
    Resize Vertical         : "sb_v_double_arrow"
    Move (Grabbing)         : "fleur"
    Busy (Wait)             : "watch"
    Not Allowed (Forbidden) : "not-allowed"
    
    Cursor = XcursorLibraryLoadCursor( Display*, const char* );
    XDefineCursor( Display*, Window, Cursor );
    
    */
}
attr_media_api void cursor_center( SurfaceHandle* surface ) {
    struct X11Surface* surf = surface;

    XWarpPointer(
        surf->display,
        None, surf->window,      // src window, dst window
        0, 0,                    // src x, y
        0, 0,                    // src w, h
        surf->w / 2, surf->h / 2 // dst x, y
    );
}
attr_media_api void cursor_set_visible( SurfaceHandle* surface, _Bool is_visible ) {
    struct X11Surface* surf = surface;
    if( is_visible ) {
        XFixesShowCursor( surf->display, surf->window );
    } else {
        XFixesHideCursor( surf->display, surf->window );
    }
}

unsigned long x11_key_to_keysym( KeyboardCode key ) {
    switch( key ) {
        case KB_BACKSPACE        : return XK_BackSpace;
        case KB_TAB              : return XK_Tab;
        case KB_ENTER            : return XK_Return;
        case KB_SHIFT_LEFT       : return XK_Shift_L;
        case KB_CONTROL_LEFT     : return XK_Control_L;
        case KB_ALT_LEFT         : return XK_Alt_L;
        case KB_PAUSE            : return XK_Pause;
        case KB_CAPSLOCK         : return XK_Caps_Lock;
        case KB_ESCAPE           : return XK_Escape;
        case KB_SPACE            : return XK_space;
        case KB_PAGE_UP          : return XK_Page_Up;
        case KB_PAGE_DOWN        : return XK_Page_Down;
        case KB_END              : return XK_End;
        case KB_HOME             : return XK_Home;
        case KB_ARROW_LEFT       : return XK_Left;
        case KB_ARROW_UP         : return XK_Up;
        case KB_ARROW_RIGHT      : return XK_Right;
        case KB_ARROW_DOWN       : return XK_Down;
        case KB_PRINT_SCREEN     : return XK_Print;
        case KB_INSERT           : return XK_Insert;
        case KB_DELETE           : return XK_Delete;
        case KB_0                : return XK_0;
        case KB_1                : return XK_1;
        case KB_2                : return XK_2;
        case KB_3                : return XK_3;
        case KB_4                : return XK_4;
        case KB_5                : return XK_5;
        case KB_6                : return XK_6;
        case KB_7                : return XK_7;
        case KB_8                : return XK_8;
        case KB_9                : return XK_9;
        case KB_A                : return XK_a;
        case KB_B                : return XK_b;
        case KB_C                : return XK_c;
        case KB_D                : return XK_d;
        case KB_E                : return XK_e;
        case KB_F                : return XK_f;
        case KB_G                : return XK_g;
        case KB_H                : return XK_h;
        case KB_I                : return XK_i;
        case KB_J                : return XK_j;
        case KB_K                : return XK_k;
        case KB_L                : return XK_l;
        case KB_M                : return XK_m;
        case KB_N                : return XK_n;
        case KB_O                : return XK_o;
        case KB_P                : return XK_p;
        case KB_Q                : return XK_q;
        case KB_R                : return XK_r;
        case KB_S                : return XK_s;
        case KB_T                : return XK_t;
        case KB_U                : return XK_u;
        case KB_V                : return XK_v;
        case KB_W                : return XK_w;
        case KB_X                : return XK_x;
        case KB_Y                : return XK_y;
        case KB_Z                : return XK_z;
        case KB_SUPER_LEFT       : return XK_Super_L;
        case KB_SUPER_RIGHT      : return XK_Super_R;
        case KB_PAD_0            : return XK_KP_0;
        case KB_PAD_1            : return XK_KP_1;
        case KB_PAD_2            : return XK_KP_2;
        case KB_PAD_3            : return XK_KP_3;
        case KB_PAD_4            : return XK_KP_4;
        case KB_PAD_5            : return XK_KP_5;
        case KB_PAD_6            : return XK_KP_6;
        case KB_PAD_7            : return XK_KP_7;
        case KB_PAD_8            : return XK_KP_8;
        case KB_PAD_9            : return XK_KP_9;
        case KB_PAD_ADD          : return XK_KP_Add;
        case KB_PAD_MULTIPLY     : return XK_KP_Multiply;
        case KB_PAD_SUBTRACT     : return XK_KP_Subtract;
        case KB_PAD_DIVIDE       : return XK_KP_Divide;
        case KB_PAD_DOT          : return XK_KP_Decimal;
        case KB_F1               : return XK_F1;
        case KB_F2               : return XK_F2;
        case KB_F3               : return XK_F3;
        case KB_F4               : return XK_F4;
        case KB_F5               : return XK_F5;
        case KB_F6               : return XK_F6;
        case KB_F7               : return XK_F7;
        case KB_F8               : return XK_F8;
        case KB_F9               : return XK_F9;
        case KB_F10              : return XK_F10;
        case KB_F11              : return XK_F11;
        case KB_F12              : return XK_F12;
        case KB_F13              : return XK_F13;
        case KB_F14              : return XK_F14;
        case KB_F15              : return XK_F15;
        case KB_F16              : return XK_F16;
        case KB_F17              : return XK_F17;
        case KB_F18              : return XK_F18;
        case KB_F19              : return XK_F19;
        case KB_F20              : return XK_F20;
        case KB_F21              : return XK_F21;
        case KB_F22              : return XK_F22;
        case KB_F23              : return XK_F23;
        case KB_F24              : return XK_F24;
        case KB_NUM_LOCK         : return XK_Num_Lock;
        case KB_SCROLL_LOCK      : return XK_Scroll_Lock;
        case KB_SEMICOLON        : return XK_semicolon;
        case KB_EQUALS           : return XK_equal;
        case KB_COMMA            : return XK_comma;
        case KB_MINUS            : return XK_minus;
        case KB_PERIOD           : return XK_period;
        case KB_SLASH            : return XK_slash;
        case KB_BACKTICK         : return XK_grave;
        case KB_BRACKET_LEFT     : return XK_bracketleft;
        case KB_BACKSLASH        : return XK_backslash;
        case KB_BRACKET_RIGHT    : return XK_bracketright;
        case KB_QUOTE            : return XK_quotedbl;
        case KB_SHIFT_RIGHT      : return XK_Shift_R;
        case KB_ALT_RIGHT        : return XK_Alt_R;
        case KB_CONTROL_RIGHT    : return XK_Control_R;

        case KB_RIGHT_CLICK_MENU:
        case KB_COUNT:
        case KB_UNKNOWN: return 0; // NoSymbol
    }
    return 0; // NoSymbol
}
KeyboardCode x11_keysym_to_key( unsigned long keysym ) {
    switch( keysym ) {
        case XK_BackSpace    : return KB_BACKSPACE;
        case XK_Tab          : return KB_TAB;
        case XK_KP_Enter:
        case XK_Return       : return KB_ENTER;
        case XK_Shift_L      : return KB_SHIFT_LEFT;
        case XK_Control_L    : return KB_CONTROL_LEFT;
        case XK_Alt_L        : return KB_ALT_LEFT;
        case XK_Pause        : return KB_PAUSE;
        case XK_Caps_Lock    : return KB_CAPSLOCK;
        case XK_Escape       : return KB_ESCAPE;
        case XK_space        : return KB_SPACE;
        case XK_Page_Up      : return KB_PAGE_UP;
        case XK_Page_Down    : return KB_PAGE_DOWN;
        case XK_End          : return KB_END;
        case XK_Home         : return KB_HOME;
        case XK_Left         : return KB_ARROW_LEFT;
        case XK_Up           : return KB_ARROW_UP;
        case XK_Right        : return KB_ARROW_RIGHT;
        case XK_Down         : return KB_ARROW_DOWN;
        case XK_Print        : return KB_PRINT_SCREEN;
        case XK_Insert       : return KB_INSERT;
        case XK_Delete       : return KB_DELETE;
        case XK_0            : return KB_0;
        case XK_1            : return KB_1;
        case XK_2            : return KB_2;
        case XK_3            : return KB_3;
        case XK_4            : return KB_4;
        case XK_5            : return KB_5;
        case XK_6            : return KB_6;
        case XK_7            : return KB_7;
        case XK_8            : return KB_8;
        case XK_9            : return KB_9;

        case XK_A:
        case XK_a            : return KB_A;
        case XK_B:
        case XK_b            : return KB_B;
        case XK_C:
        case XK_c            : return KB_C;
        case XK_D:
        case XK_d            : return KB_D;
        case XK_E:
        case XK_e            : return KB_E;
        case XK_F:
        case XK_f            : return KB_F;
        case XK_G:
        case XK_g            : return KB_G;
        case XK_H:
        case XK_h            : return KB_H;
        case XK_I:
        case XK_i            : return KB_I;
        case XK_J:
        case XK_j            : return KB_J;
        case XK_K:
        case XK_k            : return KB_K;
        case XK_L:
        case XK_l            : return KB_L;
        case XK_M:
        case XK_m            : return KB_M;
        case XK_N:
        case XK_n            : return KB_N;
        case XK_O:
        case XK_o            : return KB_O;
        case XK_P:
        case XK_p            : return KB_P;
        case XK_Q:
        case XK_q            : return KB_Q;
        case XK_R:
        case XK_r            : return KB_R;
        case XK_S:
        case XK_s            : return KB_S;
        case XK_T:
        case XK_t            : return KB_T;
        case XK_U:
        case XK_u            : return KB_U;
        case XK_V:
        case XK_v            : return KB_V;
        case XK_W:
        case XK_w            : return KB_W;
        case XK_X:
        case XK_x            : return KB_X;
        case XK_Y:
        case XK_y            : return KB_Y;
        case XK_Z:
        case XK_z            : return KB_Z;

        case XK_Super_L      : return KB_SUPER_LEFT;
        case XK_Super_R      : return KB_SUPER_RIGHT;

        case XK_KP_Insert:
        case XK_KP_0         : return KB_PAD_0;

        case XK_KP_End:
        case XK_KP_1         : return KB_PAD_1;

        case XK_KP_Down:
        case XK_KP_2         : return KB_PAD_2;

        case XK_KP_Page_Down:
        case XK_KP_3         : return KB_PAD_3;

        case XK_KP_Left:
        case XK_KP_4         : return KB_PAD_4;

        case XK_KP_Begin:
        case XK_KP_5         : return KB_PAD_5;

        case XK_KP_Right:
        case XK_KP_6         : return KB_PAD_6;

        case XK_KP_Home:
        case XK_KP_7         : return KB_PAD_7;

        case XK_KP_Up:
        case XK_KP_8         : return KB_PAD_8;

        case XK_KP_Page_Up:
        case XK_KP_9         : return KB_PAD_9;

        case XK_KP_Add       : return KB_PAD_ADD;
        case XK_KP_Multiply  : return KB_PAD_MULTIPLY;
        case XK_KP_Subtract  : return KB_PAD_SUBTRACT;
        case XK_KP_Divide    : return KB_PAD_DIVIDE;

        case XK_KP_Delete:
        case XK_KP_Decimal   : return KB_PAD_DOT;

        case XK_F1           : return KB_F1;
        case XK_F2           : return KB_F2;
        case XK_F3           : return KB_F3;
        case XK_F4           : return KB_F4;
        case XK_F5           : return KB_F5;
        case XK_F6           : return KB_F6;
        case XK_F7           : return KB_F7;
        case XK_F8           : return KB_F8;
        case XK_F9           : return KB_F9;
        case XK_F10          : return KB_F10;
        case XK_F11          : return KB_F11;
        case XK_F12          : return KB_F12;
        case XK_F13          : return KB_F13;
        case XK_F14          : return KB_F14;
        case XK_F15          : return KB_F15;
        case XK_F16          : return KB_F16;
        case XK_F17          : return KB_F17;
        case XK_F18          : return KB_F18;
        case XK_F19          : return KB_F19;
        case XK_F20          : return KB_F20;
        case XK_F21          : return KB_F21;
        case XK_F22          : return KB_F22;
        case XK_F23          : return KB_F23;
        case XK_F24          : return KB_F24;
        case XK_Num_Lock     : return KB_NUM_LOCK;
        case XK_Scroll_Lock  : return KB_SCROLL_LOCK;
        case XK_semicolon    : return KB_SEMICOLON;
        case XK_equal        : return KB_EQUALS;
        case XK_comma        : return KB_COMMA;
        case XK_minus        : return KB_MINUS;
        case XK_period       : return KB_PERIOD;
        case XK_slash        : return KB_SLASH;
        case XK_grave        : return KB_BACKTICK;
        case XK_bracketleft  : return KB_BRACKET_LEFT;
        case XK_backslash    : return KB_BACKSLASH;
        case XK_bracketright : return KB_BRACKET_RIGHT;
        
        case XK_apostrophe:
        case XK_quotedbl     : return KB_QUOTE;

        case XK_Shift_R      : return KB_SHIFT_RIGHT;
        case XK_Alt_R        : return KB_ALT_RIGHT;
        case XK_Control_R    : return KB_CONTROL_RIGHT;

        default: break;
    }
    return KB_UNKNOWN;
}

#endif /* Platform Linux */
