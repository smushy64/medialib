#if !defined(MEDIA_INTERNAL_LOGGING_H)
#define MEDIA_INTERNAL_LOGGING_H
/**
 * @file   logging.h
 * @brief  Internal logging functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "media/defines.h"
#include "media/types.h"
#include "media/lib.h"

void media_log( enum MediaLoggingLevel level, uint32_t len, const char* message );

#if defined(MEDIA_ENABLE_LOGGING)

#define media_warn( message )\
    media_log( MEDIA_LOGGING_LEVEL_WARN, sizeof(message) - 1, message )
#define media_error( message )\
    media_log( MEDIA_LOGGING_LEVEL_ERROR, sizeof(message) - 1, message )

#else

#define media_warn(...) unused(__VA_ARGS__)
#define media_error(...) unused(__VA_ARGS__)

#endif

#endif /* header guard */
