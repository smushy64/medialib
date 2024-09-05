#if !defined(MEDIA_PROMPT_H)
#define MEDIA_PROMPT_H
/**
 * @file   prompt.h
 * @brief  Create user prompts.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 14, 2024
*/
#include "media/defines.h"
#include "media/types.h"

/// @brief Type of message box's message.
typedef enum PromptMessageType {
    /// @brief Message box delivers information.
    PROMPT_MESSAGE_TYPE_INFO,
    /// @brief Message box delivers warning.
    PROMPT_MESSAGE_TYPE_WARN,
    /// @brief Message box delivers an error.
    PROMPT_MESSAGE_TYPE_ERROR,
} PromptMessageType;
/// @brief Options that message box should display.
typedef enum PromptMessageOptions {
    /// @brief Message box OK button.
    PROMPT_MESSAGE_OPTIONS_OK,
    /// @brief Message box OK and CANCEL buttons.
    PROMPT_MESSAGE_OPTIONS_OK_CANCEL,
    /// @brief Message box YES and NO buttons.
    PROMPT_MESSAGE_OPTIONS_YES_NO,
} PromptMessageOptions;
/// @brief Result from message box.
typedef enum PromptMessageResult {
    /// @brief An unknown error occurred when creating message box.
    PROMPT_MESSAGE_ERROR_UNKNOWN,
    /// @brief OK button pressed.
    PROMPT_MESSAGE_RESULT_OK_PRESSED,
    /// @brief CANCEL button pressed.
    PROMPT_MESSAGE_RESULT_CANCEL_PRESSED,
    /// @brief YES button pressed.
    PROMPT_MESSAGE_RESULT_YES_PRESSED,
    /// @brief NO button pressed.
    PROMPT_MESSAGE_RESULT_NO_PRESSED,
} PromptMessageResult;
/// @brief Result from file open prompt.
typedef enum PromptFileOpenResult {
    /// @brief Prompt created successfully and user selected a file.
    PROMPT_FILE_OPEN_RESULT_SUCCESS,
    /// @brief Prompt created successfully and user canceled selecting a file.
    PROMPT_FILE_OPEN_RESULT_CANCELED,
    /// @brief Failed to parse extension filters.
    PROMPT_FILE_OPEN_ERROR_PARSE_FILTERS,
    /// @brief Failed to allocate memory required for prompt.
    PROMPT_FILE_OPEN_ERROR_OUT_OF_MEMORY,
    /// @brief Failed to create prompt due to unknown system API error.
    PROMPT_FILE_OPEN_ERROR_UNKNOWN,
} PromptFileOpenResult;

/// @brief Present a message box to user.
/// @details
/// This prompt blocks the calling thread.
/// @note This function allocated on Windows.
/// @param     opt_title_len (optional) Length of title of message prompt.
/// @param[in] opt_title     (optional) Title of message box.
/// @param     message_len   Length of message.
/// @param[in] message       Message in message box client area.
/// @param     type          Type of message box.
/// @param     options       Options that message box presents to the user.
/// @return Error or button that user pressed.
attr_media_api PromptMessageResult prompt_message(
    m_uint32 opt_title_len, const char* opt_title,
    m_uint32 message_len, const char* message,
    PromptMessageType type, PromptMessageOptions options );
/// @brief Present a file open prompt to the user.
/// @details
/// This prompt only allows for selecting one file.
/// 
/// Extension filter formatting:
///
/// Filters are separated by a semicolon (;).
///
/// Each filter supports * wildcard for matching any number of characters.
///
/// Filters can also have an optional display name.
///
/// Display name comes before filter and is escaped with a colon (:).
///
/// First filter is considered the default filter in prompt.
///
/// Last filter does not need a semicolon.
///
/// Example:
/// "All:*.*;Source Files:*.c;Header Files:*.h"
/// @note This function allocates on Windows.
/// @param      opt_title_len       (optional) Length of prompt title (in bytes).
/// @param[in]  opt_title           (optional) Pointer to start of prompt title (UTF-8 encoded).
/// @param      opt_ext_filters_len (optional) Length of extension filters (in bytes).
/// @param[in]  opt_ext_filters     (optional) Pointer to start of extension filters. (UTF-8 encoded) See details for more information.
/// @param      result_buffer_cap   Capacity of result buffer (in bytes).
/// @param[out] out_result_len      Pointer to write length of result path.
/// @param[out] out_result_buffer   Pointer to write result path to.
/// @return Prompt File Open Result.
/// @see #PromptFileOpenResult
attr_media_api PromptFileOpenResult prompt_file_open(
    m_uint32 opt_title_len,
    const char* opt_title,
    m_uint32 opt_ext_filters_len,
    const char* opt_ext_filters,
    m_uint32 result_buffer_cap,
    m_uint32* out_result_len,
    char* out_result_buffer );

#endif /* header guard */
