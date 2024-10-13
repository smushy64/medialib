#if !defined(MEDIA_INPUT_KEYBOARD_H)
#define MEDIA_INPUT_KEYBOARD_H
/**
 * @file   keyboard.h
 * @brief  Keyboard input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 20, 2024
*/
#include "media/types.h"

/// @brief Key modifiers bitfield.
typedef enum KeyboardMod : uint8_t {
    /// @brief Left/right shift key is down.
    KBMOD_SHIFT  = (1 << 0),
    /// @brief Left/right control key is down.
    KBMOD_CTRL   = (1 << 1),
    /// @brief Left/right alt key is down.
    KBMOD_ALT    = (1 << 2),
    /// @brief Capslock is on.
    KBMOD_CAPSLK = (1 << 3),
    /// @brief Scroll lock is on.
    KBMOD_SCRLK  = (1 << 4),
    /// @brief Numlock is on.
    KBMOD_NUMLK  = (1 << 5),
} KeyboardMod;
/// @brief Key code constants.
/// @note Key code names correspond to the appropriate key for a
/// US QWERTY keyboard. On other keyboard layouts, they may map to
/// a different key name but keep the same relative layout.
/// As an example: WASD in QWERTY maps to ZQSD in AZERTY.
typedef enum KeyboardCode : uint8_t {
    /// @brief Unknown key.
    KB_UNKNOWN,

    /// @brief Backspace key.
    KB_BACKSPACE,
    /// @brief Tab key.
    KB_TAB,

    /// @brief Enter key.
    KB_ENTER,

    /// @brief Left side Shift key.
    KB_SHIFT_LEFT,
    /// @brief Left side Control key.
    KB_CONTROL_LEFT,
    /// @brief Left side Alt key.
    KB_ALT_LEFT,
    /// @brief Pause key.
    KB_PAUSE,
    /// @brief Capslock key.
    KB_CAPSLOCK,

    /// @brief Escape key.
    KB_ESCAPE,

    /// @brief Space key.
    KB_SPACE,
    /// @brief Page Up key.
    KB_PAGE_UP,
    /// @brief Page Down key.
    KB_PAGE_DOWN,
    /// @brief End key.
    KB_END,
    /// @brief Home key.
    KB_HOME,
    /// @brief Left arrow key.
    KB_ARROW_LEFT,
    /// @brief Up arrow key.
    KB_ARROW_UP,
    /// @brief Right arrow key.
    KB_ARROW_RIGHT,
    /// @brief Down arrow key.
    KB_ARROW_DOWN,

    /// @brief Print Screen key.
    KB_PRINT_SCREEN,
    /// @brief Insert key.
    KB_INSERT,
    /// @brief Delete key.
    KB_DELETE,

    /// @brief Top number row 0 key.
    KB_0,
    /// @brief Top number row 1 key.
    KB_1,
    /// @brief Top number row 2 key.
    KB_2,
    /// @brief Top number row 3 key.
    KB_3,
    /// @brief Top number row 4 key.
    KB_4,
    /// @brief Top number row 5 key.
    KB_5,
    /// @brief Top number row 6 key.
    KB_6,
    /// @brief Top number row 7 key.
    KB_7,
    /// @brief Top number row 8 key.
    KB_8,
    /// @brief Top number row 9 key.
    KB_9,

    /// @brief A key.
    KB_A,
    /// @brief B key.
    KB_B,
    /// @brief C key.
    KB_C,
    /// @brief D key.
    KB_D,
    /// @brief E key.
    KB_E,
    /// @brief F key.
    KB_F,
    /// @brief G key.
    KB_G,
    /// @brief H key.
    KB_H,
    /// @brief I key.
    KB_I,
    /// @brief J key.
    KB_J,
    /// @brief K key.
    KB_K,
    /// @brief L key.
    KB_L,
    /// @brief M key.
    KB_M,
    /// @brief N key.
    KB_N,
    /// @brief O key.
    KB_O,
    /// @brief P key.
    KB_P,
    /// @brief Q key.
    KB_Q,
    /// @brief R key.
    KB_R,
    /// @brief S key.
    KB_S,
    /// @brief T key.
    KB_T,
    /// @brief U key.
    KB_U,
    /// @brief V key.
    KB_V,
    /// @brief W key.
    KB_W,
    /// @brief X key.
    KB_X,
    /// @brief Y key.
    KB_Y,
    /// @brief Z key.
    KB_Z,
    /// @brief Left side Super key.
    /// @note On windows this is the @c Windows key.
    KB_SUPER_LEFT,
    /// @brief Right side Super key.
    /// @note On windows this is the @c Windows key.
    KB_SUPER_RIGHT,

    /// @brief Numpad 0 key.
    KB_PAD_0,
    /// @brief Numpad 1 key.
    KB_PAD_1,
    /// @brief Numpad 2 key.
    KB_PAD_2,
    /// @brief Numpad 3 key.
    KB_PAD_3,
    /// @brief Numpad 4 key.
    KB_PAD_4,
    /// @brief Numpad 5 key.
    KB_PAD_5,
    /// @brief Numpad 6 key.
    KB_PAD_6,
    /// @brief Numpad 7 key.
    KB_PAD_7,
    /// @brief Numpad 8 key.
    KB_PAD_8,
    /// @brief Numpad 9 key.
    KB_PAD_9,

    /// @brief Numpad add key.
    KB_PAD_ADD,
    /// @brief Numpad multiply key.
    KB_PAD_MULTIPLY,
    /// @brief Numpad subtract key.
    KB_PAD_SUBTRACT,
    /// @brief Numpad divide key.
    KB_PAD_DIVIDE,
    /// @brief Numpad dot key.
    KB_PAD_DOT,

    /// @brief F1 key.
    KB_F1,
    /// @brief F2 key.
    KB_F2,
    /// @brief F3 key.
    KB_F3,
    /// @brief F4 key.
    KB_F4,
    /// @brief F5 key.
    KB_F5,
    /// @brief F6 key.
    KB_F6,
    /// @brief F7 key.
    KB_F7,
    /// @brief F8 key.
    KB_F8,
    /// @brief F9 key.
    KB_F9,
    /// @brief F10 key.
    KB_F10,
    /// @brief F11 key.
    KB_F11,
    /// @brief F12 key.
    KB_F12,
    /// @brief F13 key.
    KB_F13,
    /// @brief F14 key.
    KB_F14,
    /// @brief F15 key.
    KB_F15,
    /// @brief F16 key.
    KB_F16,
    /// @brief F17 key.
    KB_F17,
    /// @brief F18 key.
    KB_F18,
    /// @brief F19 key.
    KB_F19,
    /// @brief F20 key.
    KB_F20,
    /// @brief F21 key.
    KB_F21,
    /// @brief F22 key.
    KB_F22,
    /// @brief F23 key.
    KB_F23,
    /// @brief F24 key.
    KB_F24,

    /// @brief Num lock key.
    KB_NUM_LOCK,
    /// @brief Scroll lock key.
    KB_SCROLL_LOCK,

    /// @brief Semicolon key.
    KB_SEMICOLON,
    /// @brief Equals key.
    KB_EQUALS,
    /// @brief Comma key.
    KB_COMMA,
    /// @brief Minus key.
    KB_MINUS,
    /// @brief Period key.
    KB_PERIOD,
    /// @brief Forward slash key.
    KB_SLASH,
    /// @brief Tick (Grave) key.
    KB_BACKTICK,

    /// @brief Left bracket key.
    KB_BRACKET_LEFT,
    /// @brief Backslash key.
    KB_BACKSLASH,
    /// @brief Right bracket key.
    KB_BRACKET_RIGHT,
    /// @brief Quote key.
    KB_QUOTE,
    /// @brief Right side Shift key.
    KB_SHIFT_RIGHT,
    /// @brief Right side Alt key.
    KB_ALT_RIGHT,
    /// @brief Right side Control key.
    KB_CONTROL_RIGHT,

    /// @brief Right click menu.
    KB_RIGHT_CLICK_MENU,

    /// @brief Number of valid key codes.
    KB_COUNT = 128,
} KeyboardCode;

/// @brief Packed boolean structure representing all key states.
typedef struct KeyboardState {
    /// @brief Packed boolean array.
    uint8_t keys[(KB_COUNT / 8) + ((KB_COUNT % 8) ? 1 : 0)];
} KeyboardState;

attr_header const char* keyboard_code_to_string(
    KeyboardCode code, uintptr_t* opt_out_len
) {
    #define result( text ) {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(text) - 1;\
        }\
    } return text

    switch( code ) {
        case KB_BACKSPACE     : result("Backspace");
        case KB_TAB           : result("Tab");
        case KB_ENTER         : result("Enter");
        case KB_SHIFT_LEFT    : result("Left Shift");
        case KB_CONTROL_LEFT  : result("Left Control");
        case KB_ALT_LEFT      : result("Left Alt");
        case KB_PAUSE         : result("Pause");
        case KB_CAPSLOCK      : result("Capslock");
        case KB_ESCAPE        : result("Escape");
        case KB_SPACE         : result("Space");
        case KB_PAGE_UP       : result("Page Up");
        case KB_PAGE_DOWN     : result("Page Down");
        case KB_END           : result("End");
        case KB_HOME          : result("Home");
        case KB_ARROW_LEFT    : result("Left");
        case KB_ARROW_UP      : result("Up");
        case KB_ARROW_RIGHT   : result("Right");
        case KB_ARROW_DOWN    : result("Down");
        case KB_PRINT_SCREEN  : result("Print Screen");
        case KB_INSERT        : result("Insert");
        case KB_DELETE        : result("Delete");
        case KB_0             : result("0");
        case KB_1             : result("1");
        case KB_2             : result("2");
        case KB_3             : result("3");
        case KB_4             : result("4");
        case KB_5             : result("5");
        case KB_6             : result("6");
        case KB_7             : result("7");
        case KB_8             : result("8");
        case KB_9             : result("9");
        case KB_A             : result("A");
        case KB_B             : result("B");
        case KB_C             : result("C");
        case KB_D             : result("D");
        case KB_E             : result("E");
        case KB_F             : result("F");
        case KB_G             : result("G");
        case KB_H             : result("H");
        case KB_I             : result("I");
        case KB_J             : result("J");
        case KB_K             : result("K");
        case KB_L             : result("L");
        case KB_M             : result("M");
        case KB_N             : result("N");
        case KB_O             : result("O");
        case KB_P             : result("P");
        case KB_Q             : result("Q");
        case KB_R             : result("R");
        case KB_S             : result("S");
        case KB_T             : result("T");
        case KB_U             : result("U");
        case KB_V             : result("V");
        case KB_W             : result("W");
        case KB_X             : result("X");
        case KB_Y             : result("Y");
        case KB_Z             : result("Z");
        case KB_SUPER_LEFT    : result("Left Super");
        case KB_SUPER_RIGHT   : result("Right Super");
        case KB_PAD_0         : result("Keypad 0");
        case KB_PAD_1         : result("Keypad 1");
        case KB_PAD_2         : result("Keypad 2");
        case KB_PAD_3         : result("Keypad 3");
        case KB_PAD_4         : result("Keypad 4");
        case KB_PAD_5         : result("Keypad 5");
        case KB_PAD_6         : result("Keypad 6");
        case KB_PAD_7         : result("Keypad 7");
        case KB_PAD_8         : result("Keypad 8");
        case KB_PAD_9         : result("Keypad 9");
        case KB_F1            : result("F1");
        case KB_F2            : result("F2");
        case KB_F3            : result("F3");
        case KB_F4            : result("F4");
        case KB_F5            : result("F5");
        case KB_F6            : result("F6");
        case KB_F7            : result("F7");
        case KB_F8            : result("F8");
        case KB_F9            : result("F9");
        case KB_F10           : result("F10");
        case KB_F11           : result("F11");
        case KB_F12           : result("F12");
        case KB_F13           : result("F13");
        case KB_F14           : result("F14");
        case KB_F15           : result("F15");
        case KB_F16           : result("F16");
        case KB_F17           : result("F17");
        case KB_F18           : result("F18");
        case KB_F19           : result("F19");
        case KB_F20           : result("F20");
        case KB_F21           : result("F21");
        case KB_F22           : result("F22");
        case KB_F23           : result("F23");
        case KB_F24           : result("F24");
        case KB_NUM_LOCK      : result("Number Lock");
        case KB_SCROLL_LOCK   : result("Scroll Lock");
        case KB_SEMICOLON     : result("Semicolon");
        case KB_EQUALS        : result("Equals");
        case KB_COMMA         : result("Comma");
        case KB_MINUS         : result("Minus");
        case KB_PERIOD        : result("Period");
        case KB_SLASH         : result("Forward Slash");
        case KB_BACKTICK      : result("Back Tick");
        case KB_BRACKET_LEFT  : result("Left Bracket");
        case KB_BACKSLASH     : result("Back Slash");
        case KB_BRACKET_RIGHT : result("Right Bracket");
        case KB_QUOTE         : result("Quote");
        case KB_SHIFT_RIGHT   : result("Right Shift");
        case KB_ALT_RIGHT     : result("Right Alt");
        case KB_CONTROL_RIGHT : result("Right Control");
        case KB_PAD_ADD       : result("Numpad Add");
        case KB_PAD_MULTIPLY  : result("Numpad Multiply");
        case KB_PAD_SUBTRACT  : result("Numpad Subtract");
        case KB_PAD_DIVIDE    : result("Numpad Divide");
        case KB_PAD_DOT       : result("Numpad Dot");
        case KB_RIGHT_CLICK_MENU : result("Right Click Menu");

        default: result( "Unknown" );
    }

#undef result
}

#endif /* header guard */
