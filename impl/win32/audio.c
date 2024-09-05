/**
 * @file   audio.c
 * @brief  Windows Audio.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 24, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/audio.h"
#include "impl/win32/common.h"

#include <dshow.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys_devpkey.h>

#define REFTIMES_PER_MS (10000)

struct Win32AudioDevice {
    IMMDevice*           device;
    IAudioClient*        client;
    enum AudioDeviceType type;
    union {
        IAudioRenderClient*  render;
        IAudioCaptureClient* capture;
    };
    WAVEFORMATEXTENSIBLE fmt;
    m_uint32 frame_count;
    m_uint32 buffer_size;
};
struct Win32AudioDeviceList {
    IMMDeviceEnumerator* enumerator;
    IMMDeviceCollection* input_devices;
    IMMDeviceCollection* output_devices;
};

attr_media_api m_uintptr audio_device_list_query_memory_requirement(void) {
    return sizeof(struct Win32AudioDeviceList);
}
attr_media_api m_bool32 audio_device_list_create( AudioDeviceList* out_list ) {
    struct Win32AudioDeviceList* list = out_list;
    HRESULT hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
        &IID_IMMDeviceEnumerator, (void**)&list->enumerator );
    if( !CoCheck( hr ) ) {
        return false;
    }

    hr = list->enumerator->lpVtbl->EnumAudioEndpoints(
        list->enumerator, eCapture, DEVICE_STATE_ACTIVE, &list->input_devices );
    if( !CoCheck( hr ) ) {
        CoRelease( list->enumerator );
        return false;
    }

    hr = list->enumerator->lpVtbl->EnumAudioEndpoints(
        list->enumerator, eRender, DEVICE_STATE_ACTIVE, &list->output_devices );
    if( !CoCheck( hr ) ) {
        CoRelease( list->input_devices );
        CoRelease( list->enumerator );
        return false;
    }

    return true;
}
attr_media_api m_uint32 audio_device_list_query_count(
    AudioDeviceList* in_list, enum AudioDeviceType type
) {
    struct Win32AudioDeviceList* list = in_list;
    UINT res = 0;
    switch( type ) {
        case AUDIO_DEVICE_TYPE_INPUT: {
            list->input_devices->lpVtbl->GetCount( list->input_devices, &res );
        } break;
        case AUDIO_DEVICE_TYPE_OUTPUT: {
            list->output_devices->lpVtbl->GetCount( list->output_devices, &res );
        } break;
    }
    return res;
}
attr_media_api m_bool32 audio_device_list_query_name(
    AudioDeviceList* in_list,
    enum AudioDeviceType type, m_uint32 index,
    char out_name[260], m_uint32* out_name_len
) {
    struct Win32AudioDeviceList* list = in_list;
    HRESULT hr;

    IMMDevice* device = NULL;
    switch( type ) {
        case AUDIO_DEVICE_TYPE_INPUT: {
            hr = list->input_devices->lpVtbl->Item(
                list->input_devices, index, &device );
            if( !CoCheck( hr ) ) {
                return false;
            }
        } break;
        case AUDIO_DEVICE_TYPE_OUTPUT: {
            hr = list->output_devices->lpVtbl->Item(
                list->output_devices, index, &device );
            if( !CoCheck( hr ) ) {
                return false;
            }
        } break;
    }

    wchar_t* id;
    hr = device->lpVtbl->GetId( device, &id );
    if( !CoCheck( hr ) ) {
        CoRelease( device );
        return false;
    }

    IPropertyStore* store;
    hr = device->lpVtbl->OpenPropertyStore( device, STGM_READ, &store );
    if( !CoCheck( hr ) ) {
        CoTaskMemFree( id );
        CoRelease( device );
        return false;
    }

    PROPVARIANT name;
    memset( &name, 0, sizeof(name) );

    hr = store->lpVtbl->GetValue( store, &PKEY_DeviceInterface_FriendlyName, &name );
    if( !CoCheck( hr ) ) {
        CoRelease( store );
        CoTaskMemFree( id );
        CoRelease( device );
        return false;
    }

    if( name.vt == VT_EMPTY ) {
        *out_name_len = 0;
    } else {
        int len = WideCharToMultiByte(
            CP_UTF8, 0, name.pwszVal, -1, out_name, 260, 0, 0 );
        *out_name_len = len;
    }

    PropVariantClear( &name );
    CoRelease( store );
    CoTaskMemFree( id );
    CoRelease( device );

    return true;
}
attr_media_api void audio_device_list_destroy( AudioDeviceList* in_list ) {
    struct Win32AudioDeviceList* list = in_list;
    CoRelease( list->enumerator );
    CoRelease( list->input_devices );
    CoRelease( list->output_devices );

    memset( list, 0, sizeof(*list) );
}

attr_media_api m_uintptr audio_device_query_memory_requirement(void) {
    return sizeof(struct Win32AudioDevice);
}
attr_media_api m_bool32 audio_device_open(
    AudioDeviceList*          in_list,
    struct AudioBufferFormat* opt_format,
    m_uint32                  buffer_length_ms,
    enum AudioDeviceType      type,
    m_uint32                  device_index,
    AudioDevice*              out_device
) {
    struct Win32AudioDeviceList* list   = in_list;
    struct Win32AudioDevice*     device = out_device;

    HRESULT hr;
    device->type = type;

    switch( type ) {
        case AUDIO_DEVICE_TYPE_INPUT: {
            if( device_index == AUDIO_DEVICE_DEFAULT ) {
                hr = list->enumerator->lpVtbl->GetDefaultAudioEndpoint(
                    list->enumerator, eCapture, eConsole, &device->device );
            } else {
                hr = list->input_devices->lpVtbl->Item( 
                    list->input_devices, device_index, &device->device );
            }
            if( !CoCheck( hr ) ) {
                return false;
            }
        } break;
        case AUDIO_DEVICE_TYPE_OUTPUT: {
            if( device_index == AUDIO_DEVICE_DEFAULT ) {
                hr = list->enumerator->lpVtbl->GetDefaultAudioEndpoint(
                    list->enumerator, eRender, eConsole, &device->device );
            } else {
                hr = list->output_devices->lpVtbl->Item(
                    list->output_devices, device_index, &device->device );
            }
            if( !CoCheck( hr ) ) {
                return false;
            }
        } break;
    }

    hr = device->device->lpVtbl->Activate(
        device->device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&device->client );
    if( !CoCheck( hr ) ) {
        CoRelease( device->device );
        return false;
    }

    REFTIME buffer_length_reftime = buffer_length_ms * REFTIMES_PER_MS;

    if( opt_format ) {
        WAVEFORMATEX* fmt = &device->fmt.Format;
        fmt->cbSize          = 0;
        fmt->wFormatTag      = WAVE_FORMAT_PCM;
        fmt->nSamplesPerSec  = opt_format->samples_per_second;
        fmt->wBitsPerSample  = opt_format->bits_per_sample;
        fmt->nChannels       = opt_format->channel_count;
        fmt->nBlockAlign     = (fmt->nChannels * fmt->wBitsPerSample) / 8;
        fmt->nAvgBytesPerSec = fmt->nSamplesPerSec * fmt->nBlockAlign;
    } else {
        WAVEFORMATEX* fmt;
        hr = device->client->lpVtbl->GetMixFormat( device->client, &fmt );
        if( !CoCheck( hr ) ) {
            CoRelease( device->client );
            CoRelease( device->device );
            return false;
        }

        memcpy( &device->fmt, fmt, sizeof( device->fmt ) );
        CoTaskMemFree( fmt );
    }

    DWORD flags = 0;

    if( opt_format ) {
        flags |= AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY |
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM;
    }

    hr = device->client->lpVtbl->Initialize(
        device->client, AUDCLNT_SHAREMODE_SHARED,
        flags, buffer_length_reftime, 0, (WAVEFORMATEX*)&device->fmt, NULL );
    if( !CoCheck( hr ) ) {
        CoRelease( device->client );
        CoRelease( device->device );
        return false;
    }

    hr = device->client->lpVtbl->GetBufferSize( device->client, &device->frame_count );
    if( !CoCheck( hr ) ) {
        CoRelease( device->client );
        CoRelease( device->device );
        return false;
    }

    device->buffer_size =
        device->frame_count * (device->fmt.Format.nChannels *
        (device->fmt.Format.wBitsPerSample / 8 ) );

    switch( type ) {
        case AUDIO_DEVICE_TYPE_INPUT: {

        } break;
        case AUDIO_DEVICE_TYPE_OUTPUT: {
            hr = device->client->lpVtbl->GetService(
                device->client, &IID_IAudioRenderClient, (void**)&device->render );
            if( !CoCheck( hr ) ) {
                CoRelease( device->client );
                CoRelease( device->device );
                return false;
            }

            BYTE* buf = NULL;
            hr = device->render->lpVtbl->GetBuffer(
                device->render, device->frame_count, &buf );
            if( !CoCheck( hr ) ) {
                CoRelease( device->render );
                CoRelease( device->client );
                CoRelease( device->device );
                return false;
            }

            memset( buf, 0, device->buffer_size );

            hr = device->render->lpVtbl->ReleaseBuffer(
                device->render, device->frame_count, 0 );
            if( !CoCheck( hr ) ) {
                CoRelease( device->render );
                CoRelease( device->client );
                CoRelease( device->device );
                return false;
            }
        } break;
    }

    return true;
}
attr_media_api void audio_device_close( AudioDevice* in_device ) {
    struct Win32AudioDevice* device = in_device;
    device->client->lpVtbl->Stop( device->client );

    CoRelease( device->device );
    CoRelease( device->client );
    switch( device->type ) {
        case AUDIO_DEVICE_TYPE_INPUT: {
            CoRelease( device->capture );
        } break;
        case AUDIO_DEVICE_TYPE_OUTPUT: {
            CoRelease( device->render );
        } break;
    }

    memset( device, 0, sizeof(*device) );
}
attr_media_api void audio_device_query_format(
    AudioDevice* in_device, struct AudioBufferFormat* out_format
) {
    struct Win32AudioDevice* device = in_device;
    out_format->channel_count      = device->fmt.Format.nChannels;
    out_format->bits_per_sample    = device->fmt.Format.wBitsPerSample;
    out_format->samples_per_second = device->fmt.Format.nSamplesPerSec;
    out_format->sample_count       = device->frame_count;
}
attr_media_api m_bool32 audio_device_buffer_lock(
    AudioDevice* in_device, struct AudioBuffer* out_buffer
) {
    struct Win32AudioDevice* device = in_device;
    if( device->type != AUDIO_DEVICE_TYPE_OUTPUT ) {
        win32_error( "audio: attempted to write lock an input audio device!" );
        return false;
    }

    m_uint32 frame_padding_count = 0;
    HRESULT hr = device->client->lpVtbl->GetCurrentPadding(
        device->client, &frame_padding_count );
    if( !CoCheck( hr ) ) {
        return false;
    }

    if( frame_padding_count > device->frame_count ) {
        return false;
    }

    m_uint32 frames_request = device->frame_count - frame_padding_count;
    if( !frames_request ) {
        return false;
    }

    BYTE* buf = NULL;
    hr = device->render->lpVtbl->GetBuffer( device->render, frames_request, &buf );
    if( !CoCheck( hr ) ) {
        return false;
    }

    out_buffer->sample_count = frames_request;
    out_buffer->size         = device->fmt.Format.nBlockAlign * frames_request;
    out_buffer->start        = buf;

    return true;
}
attr_media_api void audio_device_buffer_unlock(
    AudioDevice* in_device, struct AudioBuffer* buffer
) {
    struct Win32AudioDevice* device = in_device;
    if( device->type != AUDIO_DEVICE_TYPE_OUTPUT ) {
        win32_error( "audio: attempted to write unlock an input audio device!" );
        return;
    }

    device->render->lpVtbl->ReleaseBuffer( device->render, buffer->sample_count, 0 );
    memset( buffer, 0, sizeof(*buffer) );
}
attr_media_api m_bool32 audio_device_start( AudioDevice* in_device ) {
    struct Win32AudioDevice* device = in_device;
    if( device->type != AUDIO_DEVICE_TYPE_OUTPUT ) {
        win32_error( "audio: attempted to start an input audio device!" );
        return false;
    }
    HRESULT hr = device->client->lpVtbl->Start( device->client );
    return CoCheck(hr);
}
attr_media_api void audio_device_stop( AudioDevice* in_device ) {
    struct Win32AudioDevice* device = in_device;
    if( device->type != AUDIO_DEVICE_TYPE_OUTPUT ) {
        win32_error( "audio: attempted to stop an input audio device!" );
        return;
    }
    device->client->lpVtbl->Stop( device->client );
}

#endif /* Platform Windows */
