#if !defined(MEDIA_PROMPT_H)
#define MEDIA_PROMPT_H
/**
 * @file   prompt.h
 * @brief  Create user prompts.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 14, 2024
*/
#include "core/string.h"
#include "core/path.h"
#include "media/attributes.h"
#include "media/types.h"

/// @brief Type of message box's message.
typedef enum MediaMessageBoxType {
    /// @brief Message box delivers information.
    MEDIA_MESSAGE_BOX_TYPE_INFO,
    /// @brief Message box delivers warning.
    MEDIA_MESSAGE_BOX_TYPE_WARN,
    /// @brief Message box delivers an error.
    MEDIA_MESSAGE_BOX_TYPE_ERROR,
} MediaMessageBoxType;
/// @brief Options that message box should display.
typedef enum MediaMessageBoxOptions {
    /// @brief Message box OK button.
    MEDIA_MESSAGE_BOX_OPTIONS_OK,
    /// @brief Message box OK and CANCEL buttons.
    MEDIA_MESSAGE_BOX_OPTIONS_OK_CANCEL,
    /// @brief Message box YES and NO buttons.
    MEDIA_MESSAGE_BOX_OPTIONS_YES_NO,
} MediaMessageBoxOptions;
/// @brief Result from message box.
typedef enum MediaMessageBoxResult {
    /// @brief An unknown error occurred when creating message box.
    MEDIA_MESSAGE_BOX_RESULT_ERROR,
    /// @brief OK button pressed.
    MEDIA_MESSAGE_BOX_RESULT_OK_PRESSED,
    /// @brief CANCEL button pressed.
    MEDIA_MESSAGE_BOX_RESULT_CANCEL_PRESSED,
    /// @brief YES button pressed.
    MEDIA_MESSAGE_BOX_RESULT_YES_PRESSED,
    /// @brief NO button pressed.
    MEDIA_MESSAGE_BOX_RESULT_NO_PRESSED,
} MediaMessageBoxResult;
/// @brief Result from open file prompt.
typedef enum MediaPromptFileOpenResult {
    /// @brief No error.
    MEDIA_PROMPT_FILE_OPEN_RESULT_SUCCESS,
    /// @brief User did not select a file.
    MEDIA_PROMPT_FILE_OPEN_RESULT_NO_FILE_SELECT,
    /// @brief Ran out of memory when trying to open prompt.
    MEDIA_PROMPT_FILE_OPEN_RESULT_ERROR_OUT_OF_MEMORY,
    /// @brief Opening prompt resulted in an error. Check logs for more info.
    MEDIA_PROMPT_FILE_OPEN_RESULT_ERROR_PROMPT,
    /// @brief @c out_path was too small.
    MEDIA_PROMPT_FILE_OPEN_RESULT_ERROR_OUT_PATH_TOO_SMALL,
} MediaPromptFileOpenResult;
/// @brief File open/save filter.
typedef struct MediaPromptFileFilter {
    /// @brief Name of filter.
    String name;
    /// @brief Number of filter patterns.
    usize len;
    /// @brief Filter patterns.
    ///
    /// Filter patterns are formatted like so:
    /// <tt>'file name'</tt><tt>.</tt><tt>'extension name'</tt>
    ///
    /// <tt>'file name'</tt>      can be any valid file name or wildcard (*).
    ///
    /// <tt>'extension name'</tt> can be any extension or wildcard (*).
    String* patterns;
} MediaPromptFileFilter;
/// @brief List of file open/save filters.
typedef struct MediaPromptFileFilterList {
    /// @brief Number of filters in list.
    usize len;
    /// @brief Pointer to filters.
    MediaPromptFileFilter* filters;
} MediaPromptFileFilterList;
/// @brief Present a message box to user.
///
/// This message box blocks the calling thread.
/// @param[in] opt_parent (optional) Parent of message box surface.
/// @param title Title of message box.
/// @param message Message in message box client area.
/// @param type Type of message box.
/// @param options Options that message box presents to the user.
/// @warning @c title and @c message MUST be null-terminated!
/// @return Error or button that user pressed.
attr_media_api MediaMessageBoxResult media_message_box_blocking(
    MediaSurface* opt_parent, String title, String message,
    MediaMessageBoxType type, MediaMessageBoxOptions options );
/// @brief Create a file open prompt.
///
/// This prompt blocks the calling thread.
/// @param[in] opt_parent (optional) Parent surface of the prompt.
/// @param opt_prompt_title (optional) Title of prompt surface.
/// @param opt_initial_directory (optional) Directory to open prompt in.
/// @param opt_filters (optional) File filters.
/// @param[out] out_path Path to file that user opened.
/// @return Result of prompt.
/// @see MediaPromptFileFilterList
/// @see MediaPromptFileOpenResult
attr_no_discard
attr_media_api MediaPromptFileOpenResult media_prompt_file_open_blocking(
    MediaSurface* opt_parent, String opt_prompt_title,
    Path opt_initial_directory, MediaPromptFileFilterList* opt_filters,
    PathBuf* out_path );


#endif /* header guard */
