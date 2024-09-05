#if !defined(MEDIA_CURSOR_H)
#define MEDIA_CURSOR_H
/**
 * @file   cursor.h
 * @brief  Mouse cursor functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 19, 2024
*/
#include "media/defines.h"
#include "media/types.h"

/// @brief Cursor shapes.
typedef enum CursorType : m_uint32 {
    /// @brief Arrow cursor.
    /// @details This is the default cursor.
    CURSOR_TYPE_ARROW,
    /// @brief Hand cursor
    /// @details Hovering on clickable object (ex: hyperlink).
    CURSOR_TYPE_HAND,
    /// @brief Text cursor
    /// @details I-beam when hovering editable text.
    CURSOR_TYPE_TEXT,
    /// @brief Wait cursor.
    /// @details When waiting for operation to finish.
    CURSOR_TYPE_WAIT,
    /// @brief Arrow + wait cursor.
    /// @details Same as CURSOR_TYPE_WAIT except arrow cursor is also showing.
    CURSOR_TYPE_ARROW_WAIT,
    /// @brief Resize cursor.
    /// @details When resizing in all directions.
    CURSOR_TYPE_SIZE_ALL,
    /// @brief Vertical resize cursor.
    /// @details When resizing vertically.
    CURSOR_TYPE_SIZE_V,
    /// @brief Horizontal resize cursor.
    /// @details When resizing horizontally.
    CURSOR_TYPE_SIZE_H,
    /// @brief Left diagonal resize cursor.
    /// @details When resizing diagonally.
    /// Top of cursor points left, bottom points right.
    CURSOR_TYPE_SIZE_L,
    /// @brief Right diagonal resize cursor.
    /// @details When resizing diagonally.
    /// Top of cursor points right, bottom points left.
    CURSOR_TYPE_SIZE_R,
    /// @brief Number of cursor shapes.
    CURSOR_TYPE_COUNT,
} CursorType;

/// @brief Set client area cursor shape.
/// @param surface Surface to apply cursor shape in.
/// @param cursor Shape to set cursor to.
attr_media_api void cursor_type_set( SurfaceHandle* surface, CursorType cursor );
/// @brief Center cursor within surface client area.
/// @param[in] surface Surface to center in.
attr_media_api void cursor_center( SurfaceHandle* surface );
/// @brief Show/hide cursor.
/// @param is_visible If true, show cursor. If false, hide cursor.
attr_media_api void cursor_set_visible( m_bool32 is_visible );

#endif /* header guard */
