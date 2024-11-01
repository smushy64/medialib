#if !defined(MEDIA_IMPL_X11_KEY_H)
#define MEDIA_IMPL_X11_KEY_H
/**
 * @file   key.h
 * @brief  Conversion between media input constants to X11 input constants.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 22, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_LINUX)
#include "media/input/keyboard.h"

unsigned long x11_key_to_keysym( KeyboardCode key );
KeyboardCode x11_keysym_to_key( unsigned long keysym );

#endif /* Platform Linux */
#endif /* header guard */
