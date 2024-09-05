#if !defined(MEDIA_TYPES_H)
#define MEDIA_TYPES_H
/**
 * @file   types.h
 * @brief  Types used throughout media library.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "media/defines.h" // IWYU pragma: export

typedef unsigned char  m_uint8;
typedef unsigned short m_uint16;
typedef unsigned int   m_uint32;

typedef signed char  m_int8;
typedef signed short m_int16;
typedef signed int   m_int32;

#if defined(__LP64__)
    typedef unsigned long m_uint64;
    typedef   signed long m_int64;
#else
    typedef unsigned long long m_uint64;
    typedef   signed long long m_int64;
#endif

#if defined(MEDIA_ARCH_64_BIT)
    typedef m_uint64 m_uintptr;
    typedef m_int64  m_intptr;
#else
    typedef m_uint32 m_uintptr;
    typedef m_int32  m_intptr;
#endif

typedef m_uint8  m_bool8;
typedef m_uint16 m_bool16;
typedef m_uint32 m_bool32;

struct GamepadState;
enum CursorType    : m_uint32;
enum GamepadButton : m_uint16;
enum KeyboardMod   : m_uint8;
enum KeyboardCode  : m_uint8;
enum MouseButton   : m_uint8;

/// @brief Opaque handle to a surface.
typedef void SurfaceHandle;

#endif /* header guard */
