#if !defined(MEDIA_CURSOR_H)
#define MEDIA_CURSOR_H
/**
 * @file   cursor.h
 * @brief  Mouse cursor functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 19, 2024
*/
#include "media/types.h"
#include "media/attributes.h"

/// @brief Cursor shapes.
typedef enum MediaCursor {
    /// @brief Arrow cursor.
    /// @details This is the default cursor.
    MEDIA_CURSOR_ARROW,
    /// @brief Hand cursor
    /// @details Hovering on clickable object (ex: hyperlink).
    MEDIA_CURSOR_HAND,
    /// @brief Text cursor
    /// @details I-beam when hovering editable text.
    MEDIA_CURSOR_TEXT,
    /// @brief Wait cursor.
    /// @details When waiting for operation to finish.
    MEDIA_CURSOR_WAIT,
    /// @brief Arrow + wait cursor.
    /// @details Same as MEDIA_CURSOR_WAIT except arrow cursor is also showing.
    MEDIA_CURSOR_ARROW_WAIT,
    /// @brief Resize cursor.
    /// @details When resizing in all directions.
    MEDIA_CURSOR_SIZE_ALL,
    /// @brief Vertical resize cursor.
    /// @details When resizing vertically.
    MEDIA_CURSOR_SIZE_V,
    /// @brief Horizontal resize cursor.
    /// @details When resizing horizontally.
    MEDIA_CURSOR_SIZE_H,
    /// @brief Left diagonal resize cursor.
    /// @details When resizing diagonally.
    /// Top of cursor points left, bottom points right.
    MEDIA_CURSOR_SIZE_L,
    /// @brief Right diagonal resize cursor.
    /// @details When resizing diagonally.
    /// Top of cursor points right, bottom points left.
    MEDIA_CURSOR_SIZE_R,
    /// @brief Number of cursor shapes.
    MEDIA_CURSOR_COUNT,
} MediaCursor;

/// @brief Set client area cursor shape.
/// @param surface Surface to apply cursor shape in.
/// @param cursor Shape to set cursor to.
attr_media_api void media_cursor_set( MediaSurface* surface, MediaCursor cursor );
/// @brief Center cursor within surface client area.
/// @param[in] surface Surface to center in.
attr_media_api void media_cursor_center( MediaSurface* surface );
/// @brief Lock/unlock cursor to bounds of surface client are.
/// @param[in] surface Surface to lock cursor to.
/// @param is_locked Whether to lock or unlock cursor.
attr_media_api void media_cursor_lock( MediaSurface* surface, b32 is_locked );
/// @brief Show/hide cursor.
/// @param is_visible If true, show cursor. If false, hide cursor.
attr_media_api void media_cursor_set_visible( b32 is_visible );

#endif /* header guard */
