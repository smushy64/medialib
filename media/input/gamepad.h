#if !defined(MEDIA_INPUT_GAMEPAD_H)
#define MEDIA_INPUT_GAMEPAD_H
/**
 * @file   gamepad.h
 * @brief  Gamepad input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 18, 2024
*/
#include "media/types.h"

/// @brief Gamepad button bitfield.
typedef enum GamepadButton : m_uint16 {
    /// @brief D-pad Up.
    GAMEPAD_BUTTON_DPAD_UP           = (1 << 0),
    /// @brief D-pad Down.
    GAMEPAD_BUTTON_DPAD_DOWN         = (1 << 1),
    /// @brief D-pad Left.
    GAMEPAD_BUTTON_DPAD_LEFT         = (1 << 2),
    /// @brief D-pad Right.
    GAMEPAD_BUTTON_DPAD_RIGHT        = (1 << 3),
    /// @brief Left menu button.
    /// @details
    /// - Xbox 360: @c Back
    /// - Xbox One/Xbox Series: @c View
    /// - Switch: @c Minus
    /// - Playstation: @c SHARE
    GAMEPAD_BUTTON_MENU_LEFT         = (1 << 4),
    /// @brief Right menu button.
    /// @details
    /// - Xbox 360: @c Start
    /// - Xbox One/Xbox Series: @c Menu
    /// - Switch: @c Plus
    /// - Playstation: @c OPTIONS
    GAMEPAD_BUTTON_MENU_RIGHT        = (1 << 5),
    /// @brief Left stick click.
    GAMEPAD_BUTTON_STICK_LEFT_CLICK  = (1 << 6),
    /// @brief Right stick click.
    GAMEPAD_BUTTON_STICK_RIGHT_CLICK = (1 << 7),
    /// @brief Left bumper.
    /// @details
    /// - Xbox: @c LB
    /// - Switch: @c L
    /// - Playstation: @c L1
    GAMEPAD_BUTTON_BUMPER_LEFT       = (1 << 8),
    /// @brief Right bumper.
    /// @details
    /// - Xbox: @c RB
    /// - Switch: @c R
    /// - Playstation: @c R1
    GAMEPAD_BUTTON_BUMPER_RIGHT      = (1 << 9),
    /// @brief Face button down.
    /// @details
    /// - Xbox: @c A
    /// - Switch: @c B
    /// - Playstation: @c X
    GAMEPAD_BUTTON_FACE_DOWN = (1 << 12),
    /// @brief Face button right.
    /// @details
    /// - Xbox: @c B
    /// - Switch: @c A
    /// - Playstation: @c O
    GAMEPAD_BUTTON_FACE_RIGHT = (1 << 13),
    /// @brief Face button left.
    /// @details
    /// - Xbox: @c X
    /// - Switch: @c Y
    /// - Playstation: @c Square
    GAMEPAD_BUTTON_FACE_LEFT = (1 << 14),
    /// @brief Face button up.
    /// @details
    /// - Xbox: @c Y
    /// - Switch: @c X
    /// - Playstation: @c Triangle
    GAMEPAD_BUTTON_FACE_UP = (1 << 15),
} GamepadButton;
/// @brief Number of gamepads.
#define GAMEPAD_MAX_COUNT (4)
/// @brief Gamepad state.
typedef struct GamepadState {
    /// @brief Bitfield of buttons.
    GamepadButton buttons;
    /// @brief Left stick X-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    m_int16 stick_left_x;
    /// @brief Left stick Y-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    m_int16 stick_left_y;
    /// @brief Right stick X-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    m_int16 stick_right_x;
    /// @brief Right stick Y-axis.
    ///
    /// Range (I16_MIN..I16_MAX)
    m_int16 stick_right_y;
    /// @brief Left trigger.
    ///
    /// Range (0..U8_MAX)
    m_uint8 trigger_left;
    /// @brief Right trigger.
    ///
    /// Range (0..U8_MAX)
    m_uint8 trigger_right;
} GamepadState;

#endif /* header guard */
