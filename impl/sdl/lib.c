/**
 * @file   lib.c
 * @brief  Posix medialib implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 14, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_POSIX)
#include "media/lib.h"
#include <SDL3/SDL.h>

attr_media_api uintptr_t media_lib_query_memory_requirement(void) {
    return 0;
}
attr_media_api _Bool media_lib_initialize(
    MediaLoggingLevel       log_level,
    MediaLoggingCallbackFN* opt_log_callback,
    void*                   opt_log_callback_params,
    void*                   buffer
) {
    media_lib_set_logging_level( log_level );
    media_lib_set_logging_callback( opt_log_callback, opt_log_callback_params );
    unused(buffer);

    return SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
}
attr_media_api void media_lib_shutdown(void) {
    SDL_Quit();
}

#endif /* Platform POSIX */
