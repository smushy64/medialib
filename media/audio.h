#if !defined(MEDIA_AUDIO_H)
#define MEDIA_AUDIO_H
/**
 * @file   audio.h
 * @brief  Basic audio output functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "media/types.h"
#include "media/attributes.h"

/// @brief Audio buffer format.
typedef struct MediaAudioBufferFormat {
    /// @brief Number of channels.
    /// @details MUST be > 0.
    u8 channel_count;
    /// @brief Number of bits per sample.
    /// @details MUST be divisible by 8.
    u8 bits_per_sample;
    /// @brief Number of samples per second.
    u32 samples_per_second;
    /// @brief Number of samples that buffer can hold.
    u32 buffer_sample_count;
} MediaAudioBufferFormat;
/// @brief Calculate number of bytes per sample in audio buffer.
/// @param[in] fmt Audio buffer format.
/// @return Number of bytes per audio sample.
attr_always_inline
attr_header u32 media_audio_buffer_format_bytes_per_sample(
    const MediaAudioBufferFormat* fmt
) {
    return (fmt->bits_per_sample / 8);
}
/// @brief Calculate total audio buffer size.
/// @param[in] fmt Audio buffer format.
/// @return Size of audio buffer in bytes.
attr_always_inline
attr_header u32 media_audio_buffer_format_buffer_size(
    const MediaAudioBufferFormat* fmt
) {
    return fmt->channel_count *
        media_audio_buffer_format_bytes_per_sample( fmt ) *
        fmt->buffer_sample_count;
}
/// @brief Slice of audio buffer.
typedef struct MediaAudioBuffer {
    /// @brief Number of samples in slice.
    u32   sample_count;
    /// @brief Byte size of buffer slice.
    u32   buffer_size;
    /// @brief Pointer to audio buffer.
    void* buffer;
} MediaAudioBuffer;
/// @brief Opaque handle to audio context.
typedef void MediaAudioContext;

/// @brief Initialize audio context.
/// @warning Must only be called ONCE when audio is uninitialized.
/// @param buffer_length_ms Length of audio buffer in milliseconds.
/// @return Audio context handle, NULL if initialization failed.
attr_media_api MediaAudioContext* media_audio_initialize( u64 buffer_length_ms );
/// @brief Shutdown audio context.
/// @param[in] ctx Audio context.
attr_media_api void media_audio_shutdown( MediaAudioContext* ctx );
/// @brief Query audio context buffer format.
/// @param[in] ctx Audio context.
/// @return Format of audio buffer.
attr_media_api MediaAudioBufferFormat media_audio_query_buffer_format(
    MediaAudioContext* ctx );
/// @brief Lock a slice of audio buffer for writing.
/// @warning A successful lock must always be followed by media_audio_buffer_unlock()
/// @param[in] ctx Audio context.
/// @param[out] out_buffer Slice of audio buffer that was locked.
/// @return
/// - <tt>true</tt> Successfully locked audio buffer.
/// - <tt>false</tt> Failed to lock buffer:
///     - Attempted to lock too early.
///     - Buffer is already locked.
attr_media_api b32 media_audio_buffer_lock(
    MediaAudioContext* ctx, MediaAudioBuffer* out_buffer );
/// @brief Unlock audio buffer slice.
/// @param[in] ctx Audio context.
/// @param[in] buffer Slice of locked audio buffer.
attr_media_api void media_audio_buffer_unlock(
    MediaAudioContext* ctx, MediaAudioBuffer* buffer );
/// @brief Continue audio buffer playback.
/// @param[in] ctx Audio context.
attr_media_api void media_audio_play( MediaAudioContext* ctx );
/// @brief Pause audio buffer playback.
/// @param[in] ctx Audio context.
attr_media_api void media_audio_pause( MediaAudioContext* ctx );

#endif /* header guard */
