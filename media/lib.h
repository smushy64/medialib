#if !defined(MEDIA_LIB_H)
#define MEDIA_LIB_H
/**
 * @file   lib.h
 * @brief  Media Library Configuration.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "media/defines.h"
#include "media/types.h"

/// @brief Logging levels for Media library.
typedef enum MediaLoggingLevel {
    /// @brief Disable logging. This is never passed into logging callback.
    MEDIA_LOGGING_LEVEL_NONE,
    /// @brief Enable only error log messages.
    MEDIA_LOGGING_LEVEL_ERROR,
    /// @brief Enable error and warning log messages.
    MEDIA_LOGGING_LEVEL_WARN,
} MediaLoggingLevel;

/// @brief Function prototype for logging callback.
/// @param     level   Logging level of message.
/// @param     len     Length of message string.
/// @param[in] message Pointer to start of log message.
/// @param[in] params  Pointer to user parameters.
typedef void MediaLoggingCallbackFN(
    MediaLoggingLevel level, uint32_t len, const char* message, void* params );

/// @brief Create a 32-bit unsigned integer that encodes a version of medialib.
/// @param major (uint16_t) Major version.
/// @param minor (uint8_t) Minor version.
/// @param patch (uint8_t) Patch version.
/// @return Version encoded as a 32-bit unsigned integer.
#define media_lib_create_version( major, minor, patch )\
   ((uint32_t)((major) << 16u | (minor) << 8u | (patch)))
/// @brief Extract major version from a medialib version integer.
/// @param version (uint32_t) Medialib version encoded as an unsigned 32-bit integer.
/// @return Major version.
#define media_lib_major( version )\
   ((uint16_t)(((version) & 0xFFFF0000u) >> 16u))
/// @brief Extract minor version from a medialib version integer.
/// @param version (uint32_t) Medialib version encoded as an unsigned 32-bit integer.
/// @return Minor version.
#define media_lib_minor( version )\
   ((uint8_t)(((version) & 0x0000FF00u) >> 8u))
/// @brief Extract patch version from a medialib version integer.
/// @param version (uint32_t) Medialib version encoded as an unsigned 32-bit integer.
/// @return Patch version.
#define media_lib_patch( version )\
   ((uint8_t)((version) & 0x000000FFu))

/// @brief Query memory requirement for media library buffer.
/// @return Number of bytes required to initialize media library.
attr_media_api uintptr_t media_lib_query_memory_requirement(void);
/// @brief Initialize media library. Must be called before other library functions.
/// @param     log_level               Logging level. Only relevant when media library is compiled with logging enabled.
/// @param[in] opt_log_callback        (optional) Pointer to logging callback function.
/// @param[in] opt_log_callback_params (optional) Pointer to logging callback user parameters.
/// @param[in] buffer                  Pointer to buffer allocated for library. Must be large enough to hold result from media_lib_query_memory_requirement().
/// @return
///     - true  : Media library was successfully initialized.
///     - false : Failed to initialize media library.
attr_media_api _Bool media_lib_initialize(
    MediaLoggingLevel       log_level,
    MediaLoggingCallbackFN* opt_log_callback,
    void*                   opt_log_callback_params,
    void*                   buffer );
/// @brief Shutdown media library.
/// @details
/// Buffer passed into media_lib_initialize() can be freed after calling this function.
/// @warning Do not call any media library functions after this function!
attr_media_api void media_lib_shutdown(void);

/// @brief Query version of media library.
/// @return Bitpacked version of media library.
/// Use media_lib_major(), media_lib_minor() and media_lib_patch() to extract version numbers.
attr_media_api uint32_t media_lib_query_version(void);
/// @brief Query command line arguments used to compile media library.
/// @param[out] opt_out_len (optional) Pointer to write length of command line string to.
/// @return Pointer to start of command line arguments.
attr_media_api const char* media_lib_query_command_line( uint32_t* opt_out_len );
/// @brief Set logging level for media library. Does nothing if library was compiled without logging support.
/// @note This function is not thread safe.
/// @param level Logging level to set.
attr_media_api void media_lib_set_logging_level( MediaLoggingLevel level );
/// @brief Query logging level for media library.
/// @return Logging level.
attr_media_api MediaLoggingLevel media_lib_query_logging_level(void);
/// @brief Set logging callback for media library. Does nothing if library was compiled without logging support.
/// @note This function is not thread safe.
/// @param[in] callback Pointer to callback function.
/// @param[in] params   Pointer to callback user parameters.
attr_media_api void media_lib_set_logging_callback(
    MediaLoggingCallbackFN* callback, void* params );
/// @brief Clear logging callback for media library. Does nothing if library was compiled without logging support.
/// @note This function is not thread safe.
attr_header void media_lib_clear_logging_callback(void) {
    media_lib_set_logging_callback( 0, 0 );
}

#endif /* header guard */
