#if !defined(MEDIA_INPUT_KEYBOARD_H)
#define MEDIA_INPUT_KEYBOARD_H
/**
 * @file   keyboard.h
 * @brief  Keyboard input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 20, 2024
*/
#include "media/types.h"
#include "media/attributes.h"
#include "core/collections.h"

/// @brief Key modifiers bitfield.
typedef enum InputKeymod : u16 {
    /// @brief Left/right shift key is down.
    INPUT_KEYMOD_SHIFT  = (1 << 0),
    /// @brief Left/right control key is down.
    INPUT_KEYMOD_CTRL   = (1 << 1),
    /// @brief Left/right alt key is down.
    INPUT_KEYMOD_ALT    = (1 << 2),
    /// @brief Capslock is on.
    INPUT_KEYMOD_CAPSLK = (1 << 3),
    /// @brief Scroll lock is on.
    INPUT_KEYMOD_SCRLK  = (1 << 4),
    /// @brief Numlock is on.
    INPUT_KEYMOD_NUMLK  = (1 << 5),
} InputKeymod;
/// @brief Key code constants.
/// @note Key code names correspond to the appropriate key for a
/// US QWERTY keyboard. On other keyboard layouts, they may map to
/// a different key name but keep the same relative layout.
/// As an example: WASD in QWERTY maps to ZQSD in AZERTY.
typedef enum InputKeycode : u16 {
    /// @brief Unknown key.
    INPUT_KEYCODE_UNKNOWN,

    /// @brief Backspace key.
    INPUT_KEYCODE_BACKSPACE,
    /// @brief Tab key.
    INPUT_KEYCODE_TAB,

    /// @brief Enter key.
    INPUT_KEYCODE_ENTER,

    /// @brief Left side Shift key.
    INPUT_KEYCODE_SHIFT_LEFT,
    /// @brief Left side Control key.
    INPUT_KEYCODE_CONTROL_LEFT,
    /// @brief Left side Alt key.
    INPUT_KEYCODE_ALT_LEFT,
    /// @brief Pause key.
    INPUT_KEYCODE_PAUSE,
    /// @brief Capslock key.
    INPUT_KEYCODE_CAPSLOCK,

    /// @brief Escape key.
    INPUT_KEYCODE_ESCAPE,

    /// @brief Space key.
    INPUT_KEYCODE_SPACE,
    /// @brief Page Up key.
    INPUT_KEYCODE_PAGE_UP,
    /// @brief Page Down key.
    INPUT_KEYCODE_PAGE_DOWN,
    /// @brief End key.
    INPUT_KEYCODE_END,
    /// @brief Home key.
    INPUT_KEYCODE_HOME,
    /// @brief Left arrow key.
    INPUT_KEYCODE_ARROW_LEFT,
    /// @brief Up arrow key.
    INPUT_KEYCODE_ARROW_UP,
    /// @brief Right arrow key.
    INPUT_KEYCODE_ARROW_RIGHT,
    /// @brief Down arrow key.
    INPUT_KEYCODE_ARROW_DOWN,

    /// @brief Print Screen key.
    INPUT_KEYCODE_PRINT_SCREEN,
    /// @brief Insert key.
    INPUT_KEYCODE_INSERT,
    /// @brief Delete key.
    INPUT_KEYCODE_DELETE,

    /// @brief Top number row 0 key.
    INPUT_KEYCODE_0,
    /// @brief Top number row 1 key.
    INPUT_KEYCODE_1,
    /// @brief Top number row 2 key.
    INPUT_KEYCODE_2,
    /// @brief Top number row 3 key.
    INPUT_KEYCODE_3,
    /// @brief Top number row 4 key.
    INPUT_KEYCODE_4,
    /// @brief Top number row 5 key.
    INPUT_KEYCODE_5,
    /// @brief Top number row 6 key.
    INPUT_KEYCODE_6,
    /// @brief Top number row 7 key.
    INPUT_KEYCODE_7,
    /// @brief Top number row 8 key.
    INPUT_KEYCODE_8,
    /// @brief Top number row 9 key.
    INPUT_KEYCODE_9,

    /// @brief A key.
    INPUT_KEYCODE_A,
    /// @brief B key.
    INPUT_KEYCODE_B,
    /// @brief C key.
    INPUT_KEYCODE_C,
    /// @brief D key.
    INPUT_KEYCODE_D,
    /// @brief E key.
    INPUT_KEYCODE_E,
    /// @brief F key.
    INPUT_KEYCODE_F,
    /// @brief G key.
    INPUT_KEYCODE_G,
    /// @brief H key.
    INPUT_KEYCODE_H,
    /// @brief I key.
    INPUT_KEYCODE_I,
    /// @brief J key.
    INPUT_KEYCODE_J,
    /// @brief K key.
    INPUT_KEYCODE_K,
    /// @brief L key.
    INPUT_KEYCODE_L,
    /// @brief M key.
    INPUT_KEYCODE_M,
    /// @brief N key.
    INPUT_KEYCODE_N,
    /// @brief O key.
    INPUT_KEYCODE_O,
    /// @brief P key.
    INPUT_KEYCODE_P,
    /// @brief Q key.
    INPUT_KEYCODE_Q,
    /// @brief R key.
    INPUT_KEYCODE_R,
    /// @brief S key.
    INPUT_KEYCODE_S,
    /// @brief T key.
    INPUT_KEYCODE_T,
    /// @brief U key.
    INPUT_KEYCODE_U,
    /// @brief V key.
    INPUT_KEYCODE_V,
    /// @brief W key.
    INPUT_KEYCODE_W,
    /// @brief X key.
    INPUT_KEYCODE_X,
    /// @brief Y key.
    INPUT_KEYCODE_Y,
    /// @brief Z key.
    INPUT_KEYCODE_Z,
    /// @brief Left side Super key.
    /// @note On windows this is the @c Windows key.
    INPUT_KEYCODE_SUPER_LEFT,
    /// @brief Right side Super key.
    /// @note On windows this is the @c Windows key.
    INPUT_KEYCODE_SUPER_RIGHT,

    /// @brief Numpad 0 key.
    INPUT_KEYCODE_PAD_0,
    /// @brief Numpad 1 key.
    INPUT_KEYCODE_PAD_1,
    /// @brief Numpad 2 key.
    INPUT_KEYCODE_PAD_2,
    /// @brief Numpad 3 key.
    INPUT_KEYCODE_PAD_3,
    /// @brief Numpad 4 key.
    INPUT_KEYCODE_PAD_4,
    /// @brief Numpad 5 key.
    INPUT_KEYCODE_PAD_5,
    /// @brief Numpad 6 key.
    INPUT_KEYCODE_PAD_6,
    /// @brief Numpad 7 key.
    INPUT_KEYCODE_PAD_7,
    /// @brief Numpad 8 key.
    INPUT_KEYCODE_PAD_8,
    /// @brief Numpad 9 key.
    INPUT_KEYCODE_PAD_9,

    /// @brief F1 key.
    INPUT_KEYCODE_F1,
    /// @brief F2 key.
    INPUT_KEYCODE_F2,
    /// @brief F3 key.
    INPUT_KEYCODE_F3,
    /// @brief F4 key.
    INPUT_KEYCODE_F4,
    /// @brief F5 key.
    INPUT_KEYCODE_F5,
    /// @brief F6 key.
    INPUT_KEYCODE_F6,
    /// @brief F7 key.
    INPUT_KEYCODE_F7,
    /// @brief F8 key.
    INPUT_KEYCODE_F8,
    /// @brief F9 key.
    INPUT_KEYCODE_F9,
    /// @brief F10 key.
    INPUT_KEYCODE_F10,
    /// @brief F11 key.
    INPUT_KEYCODE_F11,
    /// @brief F12 key.
    INPUT_KEYCODE_F12,
    /// @brief F13 key.
    INPUT_KEYCODE_F13,
    /// @brief F14 key.
    INPUT_KEYCODE_F14,
    /// @brief F15 key.
    INPUT_KEYCODE_F15,
    /// @brief F16 key.
    INPUT_KEYCODE_F16,
    /// @brief F17 key.
    INPUT_KEYCODE_F17,
    /// @brief F18 key.
    INPUT_KEYCODE_F18,
    /// @brief F19 key.
    INPUT_KEYCODE_F19,
    /// @brief F20 key.
    INPUT_KEYCODE_F20,
    /// @brief F21 key.
    INPUT_KEYCODE_F21,
    /// @brief F22 key.
    INPUT_KEYCODE_F22,
    /// @brief F23 key.
    INPUT_KEYCODE_F23,
    /// @brief F24 key.
    INPUT_KEYCODE_F24,

    /// @brief Num lock key.
    INPUT_KEYCODE_NUM_LOCK,
    /// @brief Scroll lock key.
    INPUT_KEYCODE_SCROLL_LOCK,

    /// @brief Semicolon key.
    INPUT_KEYCODE_SEMICOLON,
    /// @brief Equals key.
    INPUT_KEYCODE_EQUALS,
    /// @brief Comma key.
    INPUT_KEYCODE_COMMA,
    /// @brief Minus key.
    INPUT_KEYCODE_MINUS,
    /// @brief Period key.
    INPUT_KEYCODE_PERIOD,
    /// @brief Forward slash key.
    INPUT_KEYCODE_SLASH,
    /// @brief Tick (Grave) key.
    INPUT_KEYCODE_BACKTICK,

    /// @brief Left bracket key.
    INPUT_KEYCODE_BRACKET_LEFT,
    /// @brief Backslash key.
    INPUT_KEYCODE_BACKSLASH,
    /// @brief Right bracket key.
    INPUT_KEYCODE_BRACKET_RIGHT,
    /// @brief Quote key.
    INPUT_KEYCODE_QUOTE,
    /// @brief Right side Shift key.
    INPUT_KEYCODE_SHIFT_RIGHT,
    /// @brief Right side Alt key.
    INPUT_KEYCODE_ALT_RIGHT,
    /// @brief Right side Control key.
    INPUT_KEYCODE_CONTROL_RIGHT,

    /// @brief Number of valid key codes.
    INPUT_KEYCODE_COUNT,
} InputKeycode;
/// @brief Query key modifiers currently pressed.
/// @return Bitfield containing key modifiers.
attr_media_api InputKeymod media_keyboard_query_mod(void);
/// @brief Query key state.
/// @param key Key to query.
/// @return True if key is down, false if it's up.
attr_media_api b32 media_keyboard_query_key( InputKeycode key );
/// @brief Query state of the entire keyboard.
/// @param[out] out_keyboard Packed boolean array to copy keyboard state to.
attr_media_api void media_keyboard_query_keyboard(
    b8 out_keyboard[packed_bool_memory_requirement(INPUT_KEYCODE_COUNT)] );

/// @brief Get keycode name as a string.
/// @param key Keycode to get name of.
/// @param[out] opt_out_len (optional) Length of string.
/// @return Readonly string.
attr_header const char* input_keycode_to_string(
    InputKeycode key, usize* opt_out_len
) {
    const u32 string_len[] = {
        sizeof( "UNKNOWN" ) - 1,
        sizeof( "BACKSPACE" ) - 1,
        sizeof( "TAB" ) - 1,
        sizeof( "ENTER" ) - 1,
        sizeof( "LEFT SHIFT" ) - 1,
        sizeof( "LEFT CONTROL" ) - 1,
        sizeof( "LEFT ALT" ) - 1,
        sizeof( "PAUSE" ) - 1,
        sizeof( "CAPSLOCK" ) - 1,
        sizeof( "ESCAPE" ) - 1,
        sizeof( "SPACE" ) - 1,
        sizeof( "PAGE UP" ) - 1,
        sizeof( "PAGE DOWN" ) - 1,
        sizeof( "END" ) - 1,
        sizeof( "HOME" ) - 1,
        sizeof( "LEFT ARROW" ) - 1,
        sizeof( "UP ARROW" ) - 1,
        sizeof( "RIGHT ARROW" ) - 1,
        sizeof( "DOWN ARROW" ) - 1,
        sizeof( "PRINT SCREEN" ) - 1,
        sizeof( "INSERT" ) - 1,
        sizeof( "DELETE" ) - 1,
        sizeof( "0" ) - 1,
        sizeof( "1" ) - 1,
        sizeof( "2" ) - 1,
        sizeof( "3" ) - 1,
        sizeof( "4" ) - 1,
        sizeof( "5" ) - 1,
        sizeof( "6" ) - 1,
        sizeof( "7" ) - 1,
        sizeof( "8" ) - 1,
        sizeof( "9" ) - 1,
        sizeof( "A" ) - 1,
        sizeof( "B" ) - 1,
        sizeof( "C" ) - 1,
        sizeof( "D" ) - 1,
        sizeof( "E" ) - 1,
        sizeof( "F" ) - 1,
        sizeof( "G" ) - 1,
        sizeof( "H" ) - 1,
        sizeof( "I" ) - 1,
        sizeof( "J" ) - 1,
        sizeof( "K" ) - 1,
        sizeof( "L" ) - 1,
        sizeof( "M" ) - 1,
        sizeof( "N" ) - 1,
        sizeof( "O" ) - 1,
        sizeof( "P" ) - 1,
        sizeof( "Q" ) - 1,
        sizeof( "R" ) - 1,
        sizeof( "S" ) - 1,
        sizeof( "T" ) - 1,
        sizeof( "U" ) - 1,
        sizeof( "V" ) - 1,
        sizeof( "W" ) - 1,
        sizeof( "X" ) - 1,
        sizeof( "Y" ) - 1,
        sizeof( "Z" ) - 1,
        sizeof( "LEFT SUPER" ) - 1,
        sizeof( "RIGHT SUPER" ) - 1,
        sizeof( "KEYPAD 0" ) - 1,
        sizeof( "KEYPAD 1" ) - 1,
        sizeof( "KEYPAD 2" ) - 1,
        sizeof( "KEYPAD 3" ) - 1,
        sizeof( "KEYPAD 4" ) - 1,
        sizeof( "KEYPAD 5" ) - 1,
        sizeof( "KEYPAD 6" ) - 1,
        sizeof( "KEYPAD 7" ) - 1,
        sizeof( "KEYPAD 8" ) - 1,
        sizeof( "KEYPAD 9" ) - 1,
        sizeof( "F1" ) - 1,
        sizeof( "F2" ) - 1,
        sizeof( "F3" ) - 1,
        sizeof( "F4" ) - 1,
        sizeof( "F5" ) - 1,
        sizeof( "F6" ) - 1,
        sizeof( "F7" ) - 1,
        sizeof( "F8" ) - 1,
        sizeof( "F9" ) - 1,
        sizeof( "F10" ) - 1,
        sizeof( "F11" ) - 1,
        sizeof( "F12" ) - 1,
        sizeof( "F13" ) - 1,
        sizeof( "F14" ) - 1,
        sizeof( "F15" ) - 1,
        sizeof( "F16" ) - 1,
        sizeof( "F17" ) - 1,
        sizeof( "F18" ) - 1,
        sizeof( "F19" ) - 1,
        sizeof( "F20" ) - 1,
        sizeof( "F21" ) - 1,
        sizeof( "F22" ) - 1,
        sizeof( "F23" ) - 1,
        sizeof( "F24" ) - 1,
        sizeof( "NUMLOCK" ) - 1,
        sizeof( "SCROLL LOCK" ) - 1,
        sizeof( "SEMICOLON" ) - 1,
        sizeof( "EQUALS" ) - 1,
        sizeof( "COMMA" ) - 1,
        sizeof( "MINUS" ) - 1,
        sizeof( "PERIOD" ) - 1,
        sizeof( "SLASH" ) - 1,
        sizeof( "BACKTICK" ) - 1,
        sizeof( "LEFT BRACKET" ) - 1,
        sizeof( "BACKSLASH" ) - 1,
        sizeof( "RIGHT BRACKET" ) - 1,
        sizeof( "QUOTE" ) - 1,
        sizeof( "RIGHT SHIFT" ) - 1,
        sizeof( "RIGHT ALT" ) - 1,
        sizeof( "RIGHT CONTROL" ) - 1,
    };
    const char* strings[] = {
        "UNKNOWN",
        "BACKSPACE",
        "TAB",
        "ENTER",
        "LEFT SHIFT",
        "LEFT CONTROL",
        "LEFT ALT",
        "PAUSE",
        "CAPSLOCK",
        "ESCAPE",
        "SPACE",
        "PAGE UP",
        "PAGE DOWN",
        "END",
        "HOME",
        "LEFT ARROW",
        "UP ARROW",
        "RIGHT ARROW",
        "DOWN ARROW",
        "PRINT SCREEN",
        "INSERT",
        "DELETE",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "LEFT SUPER",
        "RIGHT SUPER",
        "KEYPAD 0",
        "KEYPAD 1",
        "KEYPAD 2",
        "KEYPAD 3",
        "KEYPAD 4",
        "KEYPAD 5",
        "KEYPAD 6",
        "KEYPAD 7",
        "KEYPAD 8",
        "KEYPAD 9",
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        "F13",
        "F14",
        "F15",
        "F16",
        "F17",
        "F18",
        "F19",
        "F20",
        "F21",
        "F22",
        "F23",
        "F24",
        "NUMLOCK",
        "SCROLL LOCK",
        "SEMICOLON",
        "EQUALS",
        "COMMA",
        "MINUS",
        "PERIOD",
        "SLASH",
        "BACKTICK",
        "LEFT BRACKET",
        "BACKSLASH",
        "RIGHT BRACKET",
        "QUOTE",
        "RIGHT SHIFT",
        "RIGHT ALT",
        "RIGHT CONTROL",
    };
    u32 index = key >= INPUT_KEYCODE_COUNT ? 0 : key;
    if( opt_out_len ) {
        *opt_out_len = string_len[index];
    }
    return strings[index];
}

#endif /* header guard */
