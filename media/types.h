#if !defined(MEDIA_TYPES_H)
#define MEDIA_TYPES_H
/**
 * @file   types.h
 * @brief  Types used throughout media library.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
// IWYU pragma: begin_exports
#include "media/defines.h" 
#include <stdint.h> 
#include <stdbool.h> 
// IWYU pragma: end_exports

struct GamepadState;
enum CursorType    : uint32_t;
enum GamepadButton : uint16_t;
enum KeyboardMod   : uint8_t;
enum KeyboardCode  : uint8_t;
enum MouseButton   : uint8_t;

/// @brief Opaque handle to a surface.
typedef void SurfaceHandle;

#endif /* header guard */
