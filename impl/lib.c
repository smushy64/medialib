/**
 * @file   lib.c
 * @brief  Media Library Configuration Implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "core/prelude.h"
#include "media/attributes.h"
#include "media/lib.h"
#include "core/sync.h"
#include "core/print.h"

#if !defined(MEDIA_LIB_VERSION_MAJOR)
    #define MEDIA_LIB_VERSION_MAJOR 0
    pragma_warn( "MEDIA_LIB_VERSION_MAJOR is undefined!" )
#endif
#if !defined(MEDIA_LIB_VERSION_MINOR)
    #define MEDIA_LIB_VERSION_MINOR 0
    pragma_warn( "MEDIA_LIB_VERSION_MINOR is undefined!" )
#endif
#if !defined(MEDIA_LIB_VERSION_PATCH)
    #define MEDIA_LIB_VERSION_PATCH 0
    pragma_warn( "MEDIA_LIB_VERSION_PATCH is undefined!" )
#endif

#define MEDIA_LIB_VERSION_STRING\
    stringify_macro(MEDIA_LIB_VERSION_MAJOR) "."\
    stringify_macro(MEDIA_LIB_VERSION_MINOR) "."\
    stringify_macro(MEDIA_LIB_VERSION_PATCH)

attr_global u32 const global_medialib_version =
    media_create_version(
        MEDIA_LIB_VERSION_MAJOR, MEDIA_LIB_VERSION_MINOR, MEDIA_LIB_VERSION_PATCH );

attr_media_api u32 media_version(void) {
    return global_medialib_version;
}
attr_media_api const char* media_version_string( usize* opt_out_len ) {
    if( opt_out_len ) {
        *opt_out_len = sizeof(MEDIA_LIB_VERSION_STRING) - 1;
    }
    return MEDIA_LIB_VERSION_STRING;
}

attr_global const char global_medialib_build_description[] =
#if defined(CORE_STATIC_BUILD)
    "Statically compiled. "
#else
    "Dynamic library. "
#endif
    "Compiled with "
    CORE_COMPILER_VERSION
    "targetting "

#if defined(CORE_PLATFORM_WINDOWS)
    "win32 "
#elif defined(CORE_PLATFORM_LINUX)
    "linux "
#elif defined(CORE_PLATFORM_ANDROID)
    "android "
#elif defined(CORE_PLATFORM_IOS)
    "iOS "
#elif defined(CORE_PLATFORM_MACOS)
    "macOS "
#else
    "unknown platform "
#endif
#if defined(CORE_ARCH_X86)
    "x86"
    #if defined(CORE_ARCH_64_BIT)
        "-64"
    #else
        ""
    #endif
    #if defined(CORE_ENABLE_SSE_INSTRUCTIONS)
        " with SSE instructions. "
    #elif defined(CORE_ENABLE_AVX_INSTRUCTIONS)
        " with AVX instructions. "
    #else
        ". "
    #endif /* no x86 simd */
#elif defined(CORE_ARCH_ARM)
    "Arm "
    #if defined(CORE_ARCH_64_BIT)
        "32-bit "
    #else
        "64-bit "
    #endif
    #if defined(CORE_ARCH_LITTLE_ENDIAN)
        "little endian"
    #else
        "big endian"
    #endif
    #if defined(CORE_ENABLE_NEON_INSTRUCTIONS)
        " with NEON instructions. "
    #else
        " . "
    #endif
#else
    "unknown architecture. "
#endif
    "Compiled on " __DATE__ "."
;

attr_media_api const char* media_build_description( usize* opt_out_len ) {
    if( opt_out_len ) {
        *opt_out_len = sizeof( global_medialib_build_description ) - 1;
    }
    return global_medialib_build_description;
}

#if !defined(MEDIA_COMMAND_LINE)
    #define MEDIA_COMMAND_LINE ""
#endif

attr_global const char global_medialib_command_line[] = MEDIA_COMMAND_LINE;
attr_media_api const char* media_command_line( usize* opt_out_len ) {
    if( opt_out_len ) {
        *opt_out_len = sizeof(global_medialib_command_line) - 1;
    }
    return global_medialib_command_line;
}

attr_global enum MediaLoggingLevel global_media_logging_level = MEDIA_LOGGING_LEVEL_NONE;
attr_global Mutex global_media_logging_mutex;
attr_global b32 global_media_logging_mutex_created = false;
attr_global MediaLoggingCallbackFN* global_media_logging_callback = NULL;
attr_global void* global_media_logging_callback_params = NULL;

attr_media_api void media_set_logging_level( enum MediaLoggingLevel level ) {
    if( !global_media_logging_mutex_created ) {
        assert_log(
            mutex_create( &global_media_logging_mutex ),
            "failed to create media logging mutex!" );
        global_media_logging_mutex_created = true;
    }
    mutex_lock( &global_media_logging_mutex );

    global_media_logging_level = level;

    mutex_unlock( &global_media_logging_mutex );
}
attr_media_api enum MediaLoggingLevel media_query_logging_level(void) {
    return global_media_logging_level;
}
attr_media_api void media_set_logging_callback(
    MediaLoggingCallbackFN* callback, void* params
) {
    if( !global_media_logging_mutex_created ) {
        assert_log(
            mutex_create( &global_media_logging_mutex ),
            "failed to create media logging mutex!" );
        global_media_logging_mutex_created = true;
    }

    mutex_lock( &global_media_logging_mutex );

    global_media_logging_callback        = callback;
    global_media_logging_callback_params = params;

    mutex_unlock( &global_media_logging_mutex );
}
attr_media_api void media_clear_logging_callback(void) {
    if( !global_media_logging_mutex_created ) {
        assert_log(
            mutex_create( &global_media_logging_mutex ),
            "failed to create logging mutex!" );
        global_media_logging_mutex_created = true;
    }

    mutex_lock( &global_media_logging_mutex );

    global_media_logging_callback        = NULL;
    global_media_logging_callback_params = NULL;

    mutex_unlock( &global_media_logging_mutex );
}
attr_unused
attr_internal usize internal_media_stream_logging_message(
    void* target, usize count, const void* bytes
) {
    enum MediaLoggingLevel level = rcast( enum MediaLoggingLevel, target );
    global_media_logging_callback(
        level, count, (const char*)bytes,
        global_media_logging_callback_params );

    return 0;
}
attr_unused
attr_always_inline inline
attr_internal b32 internal_media_logging_level_valid( enum MediaLoggingLevel level ) {
    return bitfield_check( global_media_logging_level, level ) != 0;
}
void media_log_va(
    int level, usize format_len,
    const char* format, va_list va
) {
#if defined(MEDIA_ENABLE_LOGGING)
    if( !global_media_logging_mutex_created ) {
        return;
    }

    if(
        !internal_media_logging_level_valid( level ) ||
        !global_media_logging_callback
    ) {
        return;
    }

    mutex_lock( &global_media_logging_mutex );

    fmt_text_va(
        internal_media_stream_logging_message, &level, format_len, format, va );

    char c = '\n';
    internal_media_stream_logging_message( &level, 1, &c );

    mutex_unlock( &global_media_logging_mutex );
#else
    unused( level, format_len, format, va );
#endif
}

