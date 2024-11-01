#if !defined(MEDIA_IMPL_X11_SURFACE_H)
#define MEDIA_IMPL_X11_SURFACE_H
/**
 * @file   surface.h
 * @brief  X11 Surface.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 23, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_LINUX)
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "media/surface.h"
#include "media/input.h"

struct X11Surface {
    Display*           display;
    Window             window;
    Atom               WM_DELETE_WINDOW;
    SurfaceCallbackFN* callback;
    void*              callback_params;

    GLXFBConfig fb_config;

    SurfaceStateFlags flags;

    int x, y, w, h;
    int mx, my;

    KeyboardState keys;
    MouseButton   mb_state;
};

#endif /* Platform Linux */
#endif /* header guard */
