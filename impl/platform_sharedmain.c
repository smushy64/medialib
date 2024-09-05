/**
 * @file   platform_dllmain.c
 * @brief  Shared object main.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 04, 2024
*/
#include "media/defines.h"

// NOTE(alicia): keeping this around for when linux layer is implemented.
// #if defined(CORE_PLATFORM_LINUX) && defined(CORE_ENABLE_SHARED_EXECUTABLE)
//     const char interp[] __attribute__((section(".interp"))) = "/lib64/ld-linux-x86-64.so.2";
// #endif

#if defined(MEDIA_PLATFORM_WINDOWS)
    #include "impl/platform_win32_sharedmain.c"
#endif


