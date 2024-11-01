/**
 * @file   lib.c
 * @brief  X11 lib implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 18, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_LINUX)
#include "impl/x11/common.h"
#include "media/lib.h"

struct X11Globals* global_x11 = NULL;

attr_media_api uintptr_t media_lib_query_memory_requirement(void) {
    return sizeof(struct X11Globals);
}

attr_media_api _Bool media_lib_initialize(
    MediaLoggingLevel       log_level,
    MediaLoggingCallbackFN* opt_log_callback,
    void*                   opt_log_callback_params,
    void*                   buffer
) {
    media_lib_set_logging_level( log_level );
    media_lib_set_logging_callback( opt_log_callback, opt_log_callback_params );

    global_x11 = buffer;

    global_x11->display = XOpenDisplay( NULL );
    global_x11->root    = XRootWindow( global_x11->display, 0 );

    return true;
}

attr_media_api void media_lib_shutdown(void) {
    XCloseDisplay( global_x11->display );
}

#endif /* Platform Linux */

