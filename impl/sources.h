#if !defined(MEDIA_IMPL_SOURCES_H)
#define MEDIA_IMPL_SOURCES_H
/**
 * @file    sources.h
 * @brief   Header that includes all required sources.
 * @warning Only include if compiling in C.
 * @author  Alicia Amarilla (smushyaa@gmail.com)
 * @date    March 13, 2024
*/
#include "core/defines.h"

#include "impl/lib.c"

#if defined(CORE_PLATFORM_WINDOWS)
    #include "impl/win32/audio.c"
    #include "impl/win32/common.c"
    #include "impl/win32/gamepad.c"
    #include "impl/win32/input.c"
    #include "impl/win32/opengl.c"
    #include "impl/win32/surface.c"
#endif

#include "corelib/impl/platform_dllmain.c"

#endif /* header guard */
