#if !defined(MEDIA_IMPL_X11_COMMON_H)
#define MEDIA_IMPL_X11_COMMON_H
/**
 * @file   common.h
 * @brief  Common.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 23, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_LINUX)
#include <X11/Xlib.h>

struct X11Globals {
    Display* display;
    Window   root;
};

extern struct X11Globals* global_x11;

#endif /* Platform Linux */
#endif /* header guard */
