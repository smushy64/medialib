/**
 * @file   lib.c
 * @brief  Media Library Configuration Implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "media/defines.h"
#include "media/types.h"
#include "media/lib.h"

#if !defined(MEDIA_LIB_VERSION_MAJOR)
    #define MEDIA_LIB_VERSION_MAJOR 0
    #warning "MEDIA_LIB_VERSION_MAJOR is undefined!" 
#endif
#if !defined(MEDIA_LIB_VERSION_MINOR)
    #define MEDIA_LIB_VERSION_MINOR 0
    #warning "MEDIA_LIB_VERSION_MINOR is undefined!"
#endif
#if !defined(MEDIA_LIB_VERSION_PATCH)
    #define MEDIA_LIB_VERSION_PATCH 0
    #warning "MEDIA_LIB_VERSION_PATCH is undefined!"
#endif

#if defined(MEDIA_ENABLE_LOGGING)
    attr_global MediaLoggingLevel global_media_logging_level = MEDIA_LOGGING_LEVEL_NONE;
    attr_global MediaLoggingCallbackFN* global_media_logging_callback = 0;
    attr_global void* global_media_logging_callback_params            = 0;
#endif

extern unsigned int external_media_library_command_line_len;
extern const char   external_media_library_command_line[];

attr_media_api m_uint32 media_lib_query_version(void) {
    return media_lib_create_version(
        MEDIA_LIB_VERSION_MAJOR, MEDIA_LIB_VERSION_MINOR, MEDIA_LIB_VERSION_PATCH );
}
attr_media_api const char* media_lib_query_command_line( m_uint32* opt_out_len ) {
    *opt_out_len = external_media_library_command_line_len;
    return external_media_library_command_line;
}
attr_media_api void media_lib_set_logging_level( MediaLoggingLevel level ) {
    unused(level);
#if defined(MEDIA_ENABLE_LOGGING)
    global_media_logging_level = level;
#endif
}
attr_media_api MediaLoggingLevel media_lib_query_logging_level(void) {
#if defined(MEDIA_ENABLE_LOGGING)
    return global_media_logging_level;
#else
    return MEDIA_LOGGING_LEVEL_NONE;
#endif
}
attr_media_api void media_lib_set_logging_callback(
    MediaLoggingCallbackFN* callback, void* params
) {
    unused( callback, params );
#if defined(MEDIA_ENABLE_LOGGING)
    global_media_logging_callback        = callback;
    global_media_logging_callback_params = params;
#endif
}

#define validate( level )\
( global_media_logging_level ? ( global_media_logging_level >= (level) ) : false )

void media_log( MediaLoggingLevel level, m_uint32 len, const char* message ) {
    unused(level, len, message);
#if defined(MEDIA_ENABLE_LOGGING)
    if( !(validate(level) && global_media_logging_callback) ) {
        return;
    }

    global_media_logging_callback(
        level, len, message, global_media_logging_callback_params );
#endif
}

#undef validate

