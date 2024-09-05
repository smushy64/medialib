/**
 * @file   sources.h
 * @brief  Media lib sources.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 10, 2024
*/
#include "media/defines.h"

#include "impl/cstdlib.c"
#include "impl/lib.c"
#include "impl/platform_sharedmain.c"

#if defined(MEDIA_PLATFORM_WINDOWS)
    #include "impl/win32/common.c"
    #include "impl/win32/prompt.c"
    #include "impl/win32/surface.c"
    #include "impl/win32/input.c"
    #include "impl/win32/opengl.c"
    #include "impl/win32/audio.c"
#endif
