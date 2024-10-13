#if !defined(MEDIA_INPUT_MOUSE_H)
#define MEDIA_INPUT_MOUSE_H
/**
 * @file   mouse.h
 * @brief  Mouse input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "media/types.h"

/// @brief Mouse Buttons bitfield.
typedef enum MouseButton : uint8_t {
    /// @brief Left mouse button.
    MB_LEFT    = (1 << 0),
    /// @brief Middle mouse button.
    MB_MIDDLE  = (1 << 1),
    /// @brief Right mouse button.
    MB_RIGHT   = (1 << 2),
    /// @brief Extra mouse button 1.
    MB_EXTRA_1 = (1 << 3),
    /// @brief Extra mouse button 2.
    MB_EXTRA_2 = (1 << 4),
} MouseButton;

#endif /* header guard */
