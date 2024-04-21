#if !defined(MEDIA_INPUT_H)
#define MEDIA_INPUT_H
/**
 * @file   input.h
 * @brief  Input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   April 20, 2024
*/
#include "media/attributes.h"
// IWYU pragma: begin_exports
#include "media/input/gamepad.h"
#include "media/input/keyboard.h"
#include "media/input/mouse.h"
// IWYU pragma: end_exports

/// @brief Query how much memory is required for input subsystem.
/// @return Size of input subsystem in bytes.
attr_media_api usize media_input_query_memory_requirement(void);

/// @brief Initialize input subsystem.
/// @warning MUST be called before querying for inputs.
/// @param buffer Buffer for input subsystem,
/// MUST be able to hold result of media_input_query_memory_requirement()!
/// @return True if succeeded, false if it didn't. Check logs for more info.
attr_media_api b32  media_input_initialize( void* buffer );
/// @brief Update input subsystem. Must be called from thread that initialized input subsystem.
/// @note MUST be called in order to update input subsystem.
attr_media_api void media_input_update(void);
/// @brief Shutdown input subsystem.
attr_media_api void media_input_shutdown(void);

#endif /* header guard */
