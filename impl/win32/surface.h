#if !defined(MEDIA_IMPL_WIN32_SURFACE_H)
#define MEDIA_IMPL_WIN32_SURFACE_H
/**
 * @file   surface.h
 * @brief  Win32 Surface definition.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "impl/win32/common.h" // IWYU pragma: export

#if defined(CORE_PLATFORM_WINDOWS)
#include "media/surface.h"

typedef u16 Win32SurfaceState;
#define WIN32_SURFACE_STATE_IS_HIDDEN     (1 << 0)
#define WIN32_SURFACE_STATE_IS_FULLSCREEN (1 << 1)
#define WIN32_SURFACE_STATE_IS_FOCUSED    (1 << 2)

#define WIN32_SURFACE_NAME_CAP (255)

struct Win32Surface {
    HWND  hwnd;
    HDC   hdc;
    DWORD dwStyle, dwExStyle;

    i32 x, y, w, h;
    Win32SurfaceState state; // u16
    u16 flags;

    WINDOWPLACEMENT placement;

    MediaSurfaceCallbackFN* callback;
    void*                   callback_params;

    MediaCursor cursor;
    u8   name_len;
    char name[WIN32_SURFACE_NAME_CAP];
};
#define surface_to_win32( in_surface )\
    struct Win32Surface* surface = (struct Win32Surface*)(in_surface)

#endif /* Platform Windows */

#endif /* header guard */
