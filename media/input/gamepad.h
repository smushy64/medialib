#if !defined(MEDIA_INPUT_GAMEPAD_H)
#define MEDIA_INPUT_GAMEPAD_H
/**
 * @file   gamepad.h
 * @brief  Gamepad input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 18, 2024
*/
#include "core/types.h"
#include "media/attributes.h"

/// @brief Gamepad button bitfield.
typedef enum InputGamepadButton {
    /// @brief D-pad Up.
    INPUT_GAMEPAD_BUTTON_DPAD_UP           = (1 << 0),
    /// @brief D-pad Down.
    INPUT_GAMEPAD_BUTTON_DPAD_DOWN         = (1 << 1),
    /// @brief D-pad Left.
    INPUT_GAMEPAD_BUTTON_DPAD_LEFT         = (1 << 2),
    /// @brief D-pad Right.
    INPUT_GAMEPAD_BUTTON_DPAD_RIGHT        = (1 << 3),
    /// @brief Left menu button.
    /// @details
    /// - Xbox 360: @c Back
    /// - Xbox One/Xbox Series: @c View
    /// - Switch: @c Minus
    /// - Playstation: @c SHARE
    INPUT_GAMEPAD_BUTTON_MENU_LEFT         = (1 << 4),
    /// @brief Right menu button.
    /// @details
    /// - Xbox 360: @c Start
    /// - Xbox One/Xbox Series: @c Menu
    /// - Switch: @c Plus
    /// - Playstation: @c OPTIONS
    INPUT_GAMEPAD_BUTTON_MENU_RIGHT        = (1 << 5),
    /// @brief Left stick click.
    INPUT_GAMEPAD_BUTTON_STICK_LEFT_CLICK  = (1 << 6),
    /// @brief Right stick click.
    INPUT_GAMEPAD_BUTTON_STICK_RIGHT_CLICK = (1 << 7),
    /// @brief Left bumper.
    /// @details
    /// - Xbox: @c LB
    /// - Switch: @c L
    /// - Playstation: @c L1
    INPUT_GAMEPAD_BUTTON_BUMPER_LEFT       = (1 << 8),
    /// @brief Right bumper.
    /// @details
    /// - Xbox: @c RB
    /// - Switch: @c R
    /// - Playstation: @c R1
    INPUT_GAMEPAD_BUTTON_BUMPER_RIGHT      = (1 << 9),
    /// @brief Left trigger press.
    /// @details Set when trigger is >= 127.
    /// - Xbox: @c LT
    /// - Switch: @c ZL 
    /// - Playstation: @c L2
    INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_LEFT  = (1 << 10),
    /// @brief Right trigger press.
    /// @details Set when trigger is >= 127.
    /// - Xbox: @c RT
    /// - Switch: @c ZR 
    /// - Playstation: @c R2
    INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_RIGHT = (1 << 11),
    /// @brief Face button down.
    /// @details
    /// - Xbox: @c A
    /// - Switch: @c B
    /// - Playstation: @c X
    INPUT_GAMEPAD_BUTTON_FACE_DOWN = (1 << 12),
    /// @brief Face button right.
    /// @details
    /// - Xbox: @c B
    /// - Switch: @c A
    /// - Playstation: @c O
    INPUT_GAMEPAD_BUTTON_FACE_RIGHT = (1 << 13),
    /// @brief Face button left.
    /// @details
    /// - Xbox: @c X
    /// - Switch: @c Y
    /// - Playstation: @c Square
    INPUT_GAMEPAD_BUTTON_FACE_LEFT = (1 << 14),
    /// @brief Face button up.
    /// @details
    /// - Xbox: @c Y
    /// - Switch: @c X
    /// - Playstation: @c Triangle
    INPUT_GAMEPAD_BUTTON_FACE_UP = (1 << 15),
} InputGamepadButton;
/// @brief Number of gamepads.
#define INPUT_GAMEPAD_COUNT (4)
/// @brief Gamepad state.
typedef struct InputGamepadState {
    /// @brief Bitfield of buttons.
    InputGamepadButton buttons;
    /// @brief Left stick X-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    i16 stick_left_x;
    /// @brief Left stick Y-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    i16 stick_left_y;
    /// @brief Right stick X-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    i16 stick_right_x;
    /// @brief Right stick Y-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    i16 stick_right_y;
    /// @brief Left trigger.
    ///
    /// Range (0..U8_MAX)
    u8 trigger_left;
    /// @brief Right trigger.
    ///
    /// Range (0..U8_MAX)
    u8 trigger_right;
} InputGamepadState;

/// @brief Normalize stick left input to range (-1..1)
/// @param[in] state Gamepad state.
/// @param[out] out_x, out_y Pointers to hold X and Y.
attr_header
attr_always_inline void media_gamepad_state_stick_left(
    const InputGamepadState* state, f32* out_x, f32* out_y
) {
    *out_x = (state->stick_left_x < 0) ?
        ((f32)state->stick_left_x) / (32768.0f) :
        ((f32)state->stick_left_x) / (32767.0f);
    *out_y = (state->stick_left_y < 0) ?
        ((f32)state->stick_left_y) / (32768.0f) :
        ((f32)state->stick_left_y) / (32767.0f);
}
/// @brief Normalize stick right input to range (-1..1)
/// @param[in] state Gamepad state.
/// @param[out] out_x, out_y Pointers to hold X and Y.
attr_header
attr_always_inline void media_gamepad_state_stick_right(
    const InputGamepadState* state, f32* out_x, f32* out_y
) {
    *out_x = (state->stick_right_x < 0) ?
        ((f32)state->stick_right_x) / (32768.0f) :
        ((f32)state->stick_right_x) / (32767.0f);
    *out_y = (state->stick_right_y < 0) ?
        ((f32)state->stick_right_y) / (32768.0f) :
        ((f32)state->stick_right_y) / (32767.0f);
}
/// @brief Normalize trigger input to range (0..1)
/// @param[in] state Gamepad state.
/// @param[out] out_left, out_right Pointers to hold left and right trigger values.
attr_header
attr_always_inline void media_gamepad_state_triggers(
    const InputGamepadState* state, f32* out_left, f32* out_right
) {
    *out_left  = (f32)state->trigger_left  / 255.0f;
    *out_right = (f32)state->trigger_right / 255.0f;
}
/// @brief Query state of gamepad at given index.
/// @param index Index of gamepad. Must be <= #INPUT_GAMEPAD_COUNT
/// @param[out] out_state Pointer to #InputGamepadState to write to.
/// @return True if gamepad is active.
attr_media_api b32 media_gamepad_query_state(
    u32 index, InputGamepadState* out_state );
/// @brief Set gamepad rumble.
/// @param index Index of gamepad. Must be <= #INPUT_GAMEPAD_COUNT
/// @param motor_left, motor_right Motor values.
attr_media_api void media_gamepad_set_rumble(
    u32 index, u16 motor_left, u16 motor_right );
/// @brief Turn off gamepad rumble for given gamepad.
/// @param index Index of gamepad. Must be <= #INPUT_GAMEPAD_COUNT
attr_header
attr_always_inline void media_gamepad_rumble_clear( u32 index ) {
    media_gamepad_set_rumble( index, 0, 0 );
}
/// @brief Query gamepad rumble.
/// @param index Index of gamepad. Must be <= #INPUT_GAMEPAD_COUNT
/// @param[out] out_motor_left, out_motor_right Pointers to hold motor states.
attr_media_api void media_gamepad_query_rumble(
    u32 index, u16* out_motor_left, u16* out_motor_right );

#endif /* header guard */
