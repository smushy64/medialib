#if !defined(MEDIA_INTERNAL_LOGGING_H)
#define MEDIA_INTERNAL_LOGGING_H
/**
 * @file   logging.h
 * @brief  Internal logging functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "core/types.h"
#include "core/attributes.h"

void media_log_va(
    int level, usize format_len,
    const char* format, va_list va );

attr_header void media_log(
    int level, usize format_len,
    const char* format, ...
) {
    va_list va;
    va_start( va, format );
    media_log_va( level, format_len, format, va );
    va_end( va );
}

#if defined(MEDIA_ENABLE_LOGGING)

#define media_debug( format, ... )\
    media_log(\
        MEDIA_LOGGING_LEVEL_DEBUG,\
        sizeof( str_blue( "[MEDIA DEBUG] " format ) ) - 1,\
        str_blue( "[MEDIA DEBUG] " format ), ##__VA_ARGS__ )

#define media_info( format, ... )\
    media_log(\
        MEDIA_LOGGING_LEVEL_INFO,\
        sizeof( "[MEDIA INFO] " format ) - 1,\
        "[MEDIA INFO] " format, ##__VA_ARGS__ )

#define media_warn( format, ... )\
    media_log(\
        MEDIA_LOGGING_LEVEL_WARN,\
        sizeof( str_yellow( "[MEDIA WARN] " format ) ) - 1,\
        str_yellow( "[MEDIA WARN] " format ), ##__VA_ARGS__ )

#define media_error( format, ... )\
    media_log(\
        MEDIA_LOGGING_LEVEL_ERROR,\
        sizeof( str_red( "[MEDIA ERROR] " format ) ) - 1,\
        str_red( "[MEDIA ERROR] " format ), ##__VA_ARGS__ )

#else

#define media_debug(...) unused(__VA_ARGS__)
#define media_info(...) unused(__VA_ARGS__)
#define media_warn(...) unused(__VA_ARGS__)
#define media_error(...) unused(__VA_ARGS__)

#endif

#endif /* header guard */
