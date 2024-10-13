#if !defined(MEDIA_AUDIO_H)
#define MEDIA_AUDIO_H
/**
 * @file   audio.h
 * @brief  Basic audio output functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "media/defines.h" // IWYU pragma: keep
#include "media/types.h"

/// @brief Maximum allowed device name length.
#define AUDIO_DEVICE_NAME_CAP (260)
/// @brief Device index for picking system default audio device.
#define AUDIO_DEVICE_DEFAULT  (0xFFFFFFFF)

/// @brief Structure defining an audio buffer's format.
struct AudioBufferFormat {
    /// @brief Number of channels.
    /// @note Channel samples are interleaved.
    uint8_t  channel_count;
    /// @brief Bits per sample.
    /// @details Common values are 16, 24 and 32.
    uint32_t bits_per_sample;
    /// @brief Number of samples in a second.
    /// @details Common values are 441000 and 48000
    uint32_t samples_per_second;
    /// @brief Number of total samples a given audio device has allocated.
    /// @details
    /// A sample includes each channel.
    /// Total sample size would then be @c channel_count * ( @c bits_per_sample / 8 )
    uint32_t sample_count;
};
/// @brief Structure representing a locked portion of audio device's buffer.
struct AudioBuffer {
    /// @brief Number of samples locked.
    uint32_t sample_count;
    /// @brief Size of locked portion in bytes.
    uint32_t size;
    /// @brief Pointer to start of locked audio buffer.
    void* start;
};
/// @brief Opaque pointer to audio device list.
typedef void AudioDeviceList;
/// @brief Opaque pointer to an audio device handle.
typedef void AudioDeviceHandle;
/// @brief Types of audio devices.
enum AudioDeviceType {
    /// @brief Input audio device, typically microphones.
    AUDIO_DEVICE_TYPE_INPUT,
    /// @brief Output audio device, typically speakers or headphones.
    AUDIO_DEVICE_TYPE_OUTPUT
};
/// @brief Opaque pointer to an audio input/output device.
typedef void AudioDevice;

/// @brief Query memory requirement for retrieving a list of available devices.
/// @return Bytes required to store device list.
attr_media_api uintptr_t audio_device_list_query_memory_requirement(void);
/// @brief Create a list of audio devices.
/// @param[in,out] in_out_list Pointer to buffer allocated for holding device list.
/// Must be able to hold result of audio_device_list_query_memory_requirement().
/// @return
///     - true  : Audio device list created successfully.
///     - false : Failed to create audio device list.
attr_media_api _Bool audio_device_list_create( AudioDeviceList* in_out_list );
/// @brief Query number of devices in audio device list.
/// @param[in] list Pointer to an audio device list.
/// @param     type Filter result by type of audio device.
/// @return Number of available devices for given type.
attr_media_api uint32_t audio_device_list_query_count(
    AudioDeviceList* list, enum AudioDeviceType type );
/// @brief Query the name of a given audio device in list.
/// @param[in]  list         Pointer to an audio device list.
/// @param      type         Type of audio device to query name of.
/// @param      index        Index of audio device to query name of.
/// @param[out] out_name     Pointer to character buffer to write audio device name to.
/// @param[out] out_name_len Pointer to write length of audio device name.
/// @return
///     - true  : Successfully obtained audio device name.
///     - false : Failed to obtain audio device name.
attr_media_api _Bool audio_device_list_query_name(
    AudioDeviceList* list,
    enum AudioDeviceType type, uint32_t index,
    char out_name[AUDIO_DEVICE_NAME_CAP], uint32_t* out_name_len );
/// @brief Destroy audio device list object.
/// @param[in] list Pointer to audio device list to destroy. 
/// Its memory can be freed after calling this function.
attr_media_api void audio_device_list_destroy( AudioDeviceList* list );

/// @brief Query audio device memory requirement.
/// @return Bytes required for audio device.
attr_media_api uintptr_t audio_device_query_memory_requirement(void);
/// @brief Open an audio device.
/// @param[in]     list             Audio device list.
/// @param[in]     opt_format       (optional) Format that audio device should be opened with.
/// @param         buffer_length_ms Length of audio device buffer in milliseconds.
/// @param         type             Type of audio device to open.
/// @param         device_index     Index of device to open or #AUDIO_DEVICE_DEFAULT.
/// @param[in,out] in_out_device    Pointer to store audio device.
/// Must be able to hold result of audio_device_query_memory_requirement().
/// @return
///     - true  : Opened audio device successfully.
///     - false : Failed to open audio device.
attr_media_api _Bool audio_device_open(
    AudioDeviceList* list, struct AudioBufferFormat* opt_format,
    uint32_t buffer_length_ms, enum AudioDeviceType type,
    uint32_t device_index, AudioDevice* out_device );
/// @brief Close audio device.
/// @param[in] device Pointer to audio device to close.
attr_media_api void audio_device_close( AudioDevice* device );
/// @brief Query audio device buffer format.
/// @param[in]  device Pointer to audio device.
/// @param[out] out_format Pointer to write out audio device format.
attr_media_api void audio_device_query_format(
    AudioDevice* device, struct AudioBufferFormat* out_format );
/// @brief Start playing audio device's buffer.
/// @param[in] device Pointer to audio device to start playing.
/// @return
///     - true  : Started audio device.
///     - false : Failed to start audio device.
attr_media_api _Bool audio_device_start( AudioDevice* device );
/// @brief Stop playing audio device.
/// @param[in] device Pointer to audio device to stop.
attr_media_api void audio_device_stop( AudioDevice* device );
/// @brief Lock audio device's buffer.
/// @param[in]  device     Device to lock.
/// @param[out] out_buffer Pointer to write information about portion of buffer locked.
/// @return
///     - true  : Locked buffer successfully.
///     - false : Failed to lock audio device buffer.
attr_media_api _Bool audio_device_buffer_lock(
    AudioDevice* device, struct AudioBuffer* out_buffer );
/// @brief Unlock audio device buffer.
/// @param[in] device Device to unlock.
/// @param[in] buffer Pointer to buffer structure defining part of buffer that was locked.
attr_media_api void audio_device_buffer_unlock(
    AudioDevice* device, struct AudioBuffer* buffer );

#endif /* header guard */
