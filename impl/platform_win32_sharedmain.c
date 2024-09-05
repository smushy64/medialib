/**
 * @file   platform_win32_sharedmain.c
 * @brief  Shared object main for Windows.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   June 26, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_WINDOWS) && !defined(MEDIA_ENABLE_STATIC_BUILD)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL WINAPI DllMainCRTStartup(
    HINSTANCE const instance,
    DWORD     const reason,
    LPVOID    const reserved
) {
    unused( instance, reserved );
    switch( reason ) {
        case DLL_PROCESS_ATTACH: {
        } break;
        case DLL_PROCESS_DETACH: {
        } break;
        case DLL_THREAD_ATTACH: break;
        case DLL_THREAD_DETACH: break;
    }
    return TRUE;
}

#endif
