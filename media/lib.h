#if !defined(MEDIA_LIB_H)
#define MEDIA_LIB_H
/**
 * @file   lib.h
 * @brief  Media Library Configuration.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "core/types.h"
#include "media/attributes.h"
#include "core/macros.h"

/// @brief Create a 32-bit unsigned integer that encodes a version of medialib.
/// @param major (u16) Major version.
/// @param minor (u8) Minor version.
/// @param patch (u8) Patch version.
/// @return Version encoded as a 32-bit unsigned integer.
#define media_create_version( major, minor, patch ) core_create_version( major, minor, patch )
/// @brief Extract major version from a medialib version integer.
/// @param version (u32) Medialib version encoded as an unsigned 32-bit integer.
/// @return Major version.
#define media_get_major( version ) core_get_major( version )
/// @brief Extract minor version from a medialib version integer.
/// @param version (u32) Medialib version encoded as an unsigned 32-bit integer.
/// @return Minor version.
#define media_get_minor( version ) core_get_minor( version )
/// @brief Extract patch version from a medialib version integer.
/// @param version (u32) Medialib version encoded as an unsigned 32-bit integer.
/// @return Patch version.
#define media_get_patch( version ) core_get_patch( version )

/// @brief Media logging level bitfield.
///
/// Bitfield that defines which log messages are allowed to pass through
/// to #MediaLoggingCallbackFN.
enum MediaLoggingLevel {
    /// @brief No logging messages.
    ///
    /// Disables formatting and streaming logging messages.
    MEDIA_LOGGING_LEVEL_NONE  = (0),
    /// @brief Only allow debug messages.
    MEDIA_LOGGING_LEVEL_DEBUG = (1 << 0),
    /// @brief Only allow info messages.
    MEDIA_LOGGING_LEVEL_INFO  = (1 << 1),
    /// @brief Only allow warning messages.
    MEDIA_LOGGING_LEVEL_WARN  = (1 << 2),
    /// @brief Only allow error messages.
    MEDIA_LOGGING_LEVEL_ERROR = (1 << 3),
};

/// @brief Function prototype for logging callback.
/// @param level Log level of message.
/// @param message_length Length of message.
/// @param[in] message Message.
/// @param[in] params Additional parameters.
typedef void MediaLoggingCallbackFN(
    enum MediaLoggingLevel level, usize message_length,
    const char* message, void* params );

/// @brief Initialize media library.
/// @warning Must be called before other library functions.
/// @param     log_level (optional) Set logging level.
/// @param     log_callback (optional) Set callback for logging functions.
/// @param[in] log_callback_params (optional) Set log callback params.
/// @return True if initialization was successful.
attr_media_api b32 media_initialize(
    enum MediaLoggingLevel log_level,
    MediaLoggingCallbackFN* log_callback, void* log_callback_params );
/// @brief Shutdown media library.
/// @warning Must be called before program exit.
attr_media_api void media_shutdown(void);

/// @brief Get version of media lib.
///
/// High short:           Major version.
/// Low short, high byte: Minor version.
/// Low short, low byte:  Patch version.
///
/// Use #media_get_major(), #media_get_minor() and #media_get_patch()
/// to extract major, minor and patch version numbers.
/// @return Version integer.
attr_media_api u32 media_version(void);
/// @brief Get version of media lib as a string.
/// @param[out] opt_out_len (optional) Length of version string.
/// @return Version string.
attr_media_api const char* media_version_string( usize* opt_out_len );
/// @brief Get string that describes how library was built.
/// @param[out] opt_out_len (optional) Length of description string.
/// @return Null-terminated UTF-8 description string.
attr_media_api const char* media_build_description( usize* opt_out_len );
/// @brief Get string that describes command line used to build library.
/// @param[out] opt_out_len (optional) Length of command line string.
/// @return Null-terminated UTF-8 command line string.
attr_media_api const char* media_command_line( usize* opt_out_len );
/// @brief Set logging level.
///
/// If logging level is set to none, disables logging.
/// @note Logging can only be enabled if library is compiled with
/// MEDIA_ENABLE_LOGGING defined.
/// @param level Bitfield defining what logging messages are provided.
attr_media_api void media_set_logging_level( enum MediaLoggingLevel level );
/// @brief Query current logging level.
/// @return Bitfield of logging level.
attr_media_api enum MediaLoggingLevel media_query_logging_level(void);
/// @brief Set callback for receiving log messages.
/// @param[in] callback Function for receiving log messages.
/// @param[in] params Additional parameters for callback.
attr_media_api void media_set_logging_callback(
    MediaLoggingCallbackFN* callback, void* params );
/// @brief Clear logging callback.
attr_media_api void media_clear_logging_callback(void);

#endif /* header guard */
