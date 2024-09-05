#if !defined(MEDIA_IMPL_WIN32_SURFACE_H)
#define MEDIA_IMPL_WIN32_SURFACE_H
/**
 * @file   surface.h
 * @brief  Media Windows Surface.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 11, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/types.h"
#include "media/surface.h"
#include "impl/win32/common.h" // IWYU pragma: keep

#define WIN32_SURFACE_TITLE_UCS2_CAP (SURFACE_MAX_TITLE_LEN + 1)
#define WIN32_SURFACE_TITLE_SIZE (sizeof(wchar_t) * WIN32_SURFACE_TITLE_UCS2_CAP)

struct Win32Surface {
    HWND hwnd;
    HDC  hdc;

    m_int32 x, y, w, h;

    WINDOWPLACEMENT placement;

    CursorType cursor;
    SurfaceCreateFlags create_flags;
    SurfaceStateFlags  state;

    SurfaceCallbackFN* callback;
    void* callback_params;

    m_uint8 title_len;
    union {
        wchar_t title_ucs2[WIN32_SURFACE_TITLE_UCS2_CAP];
        char title_utf8[WIN32_SURFACE_TITLE_SIZE];
    };
};

#endif /* Platform Windows */
#endif /* header guard */
