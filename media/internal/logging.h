#if !defined(MEDIA_INTERNAL_LOGGING_H)
#define MEDIA_INTERNAL_LOGGING_H
/**
 * @file   logging.h
 * @brief  Internal logging functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "core/types.h"
#include "media/lib.h" // IWYU pragma: keep

void media_log_va( CoreLoggingLevel level, usize len, const char* buf, va_list va );
void media_log( CoreLoggingLevel level, usize len, const char* buf, ... );

#if defined(MEDIA_ENABLE_LOGGING)

#define media_warn( format, ... )\
    media_log( CORE_LOGGING_LEVEL_WARN, sizeof(format) - 1, format, ##__VA_ARGS__ )
#define media_error( format, ... )\
    media_log( CORE_LOGGING_LEVEL_ERROR, sizeof(format) - 1, format, ##__VA_ARGS__ )

#else

#define media_warn(...) unused(__VA_ARGS__)
#define media_error(...) unused(__VA_ARGS__)

#endif

#endif /* header guard */
