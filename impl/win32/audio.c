/**
 * @file   audio.c
 * @brief  Win32: Media Library Audio implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "impl/win32/common.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "core/prelude.h"
#include "core/memory.h"

#include "media/internal/logging.h"
#include "media/audio.h"

#include <initguid.h>
#include <dshow.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

struct Win32AudioContext {
    IAudioClient*        client;
    IAudioRenderClient*  render_client;
    IMMDeviceEnumerator* device_enum;
    IMMDevice*           device;

    WAVEFORMATEX format;

    u32 buffer_frame_count;
    u32 buffer_size;
};
#define ctx_to_win32( in_ctx )\
    struct Win32AudioContext* ctx = (struct Win32AudioContext*)(in_ctx)

#define def( ret, fn, ... )\
typedef ret fn##FN( __VA_ARGS__ );\
attr_internal fn##FN* in_##fn = NULL

def( HRESULT, CoInitialize, LPVOID );
#define CoInitialize in_CoInitialize

def( void, CoUninitialize, void );
#define CoUninitialize in_CoUninitialize

def( HRESULT, CoCreateInstance, REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID );
#define CoCreateInstance in_CoCreateInstance

def( void, CoTaskMemFree, LPVOID );
#define CoTaskMemFree in_CoTaskMemFree

#undef def

#define IMMDeviceEnumeratorGetDefaultAudioEndpoint( device_enum, dataFlow, role, ppEndpoint )\
    device_enum->lpVtbl->GetDefaultAudioEndpoint( device_enum, dataFlow, role, ppEndpoint )

#define IMMDeviceActivate( device, iid, dwClsCtx, pActivationParams, ppInterface )\
    device->lpVtbl->Activate( device, iid, dwClsCtx, pActivationParams, ppInterface )

#define IAudioClientInitialize( client, ShareMode, StreamFlags, hnsBufferDuration, hnsPeriodicity, pFormat, AudioSessionGuid )\
    client->lpVtbl->Initialize( client, ShareMode, StreamFlags, hnsBufferDuration, hnsPeriodicity, pFormat, AudioSessionGuid )

#define IAudioClientGetBufferSize( client, pNumBufferFrames )\
    client->lpVtbl->GetBufferSize( client, pNumBufferFrames )

#define IAudioClientGetService( client, riid, ppv )\
    client->lpVtbl->GetService( client, riid, ppv )

#define IAudioClientStart( client )\
    client->lpVtbl->Start( client )

#define IAudioClientStop( client )\
    client->lpVtbl->Stop( client )

#define IAudioClientGetCurrentPadding( client, pNumPaddingFrames )\
    client->lpVtbl->GetCurrentPadding( client, pNumPaddingFrames )

#define IAudioRenderClientGetBuffer( client, NumFramesRequested, ppData )\
    client->lpVtbl->GetBuffer( client, NumFramesRequested, ppData )

#define IAudioRenderClientReleaseBuffer( client, NumFramesWritten, dwFlags )\
    client->lpVtbl->ReleaseBuffer( client, NumFramesWritten, dwFlags )

attr_media_api MediaAudioContext* media_audio_initialize( u64 buffer_length_ms ) {
    struct Win32AudioContext* ctx =
        HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*ctx) );
    if( !ctx ) {
        media_error(
            "[WIN32] failed to allocate {f,.2,m} for audio context!",
            (f64)sizeof(*ctx) );
        return NULL;
    }

    #define load( mod, name ) do {\
        name = (name##FN*)GetProcAddress( global_win32_state->mod, #name );\
        if( !name ) {\
            win32_error( "failed to load " #name " from library " #mod "!" );\
            goto media_audio_initialize_failed;\
        }\
    } while(0)

    #define check( hr ) do {\
        if( FAILED( hr ) ) {\
            win32_error( stringify_macro(__LINE__) ": audio initialize failed! HRESULT: {i,X}", hr );\
            goto media_audio_initialize_failed;\
        }\
    } while(0)

    global_win32_state->OLE32 = LoadLibraryA( "OLE32.DLL" );
    if( !global_win32_state->OLE32 ) {
        win32_error( "failed to open library OLE32.DLL!" );
        goto media_audio_initialize_failed;
    }

    load( OLE32, CoInitialize );
    load( OLE32, CoUninitialize );
    load( OLE32, CoCreateInstance );
    load( OLE32, CoTaskMemFree );

    HRESULT hr = CoInitialize( NULL );
    check( hr );

    hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
        &IID_IMMDeviceEnumerator, (void**)&ctx->device_enum );
    check( hr );

    hr = IMMDeviceEnumeratorGetDefaultAudioEndpoint(
        ctx->device_enum, eRender, eConsole, &ctx->device );
    check( hr );

    hr = IMMDeviceActivate(
        ctx->device, &IID_IAudioClient,
        CLSCTX_ALL, NULL, (void**)&ctx->client );
    check( hr );

    WAVEFORMATEX* fmt = &ctx->format;
    fmt->nChannels       = 2;
    fmt->wFormatTag      = WAVE_FORMAT_PCM;
    fmt->nSamplesPerSec  = 44100;
    fmt->wBitsPerSample  = 16;
    fmt->nBlockAlign     = ( fmt->nChannels * fmt->wBitsPerSample ) / 8;
    fmt->nAvgBytesPerSec = fmt->nSamplesPerSec * fmt->nBlockAlign;
    fmt->cbSize          = 0;

    #define REFTIMES_PER_MS (10000)

    REFTIME buffer_length_reftime = buffer_length_ms * REFTIMES_PER_MS;
    DWORD flags =
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
        AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

    hr = IAudioClientInitialize(
        ctx->client, AUDCLNT_SHAREMODE_SHARED,
        flags, buffer_length_reftime, 0, &ctx->format, NULL );
    check( hr );

    hr = IAudioClientGetBufferSize( ctx->client, &ctx->buffer_frame_count );
    check( hr );

    ctx->buffer_size = ctx->buffer_frame_count *
        ( fmt->nChannels * (fmt->wBitsPerSample / 8) );

    hr = IAudioClientGetService(
        ctx->client, &IID_IAudioRenderClient, (void**)&ctx->render_client );
    check( hr );

    BYTE* buf = NULL;
    hr = IAudioRenderClientGetBuffer(
        ctx->render_client, ctx->buffer_frame_count, &buf );
    check( hr );

    memory_zero( buf, ctx->buffer_size );

    hr = IAudioRenderClientReleaseBuffer(
        ctx->render_client, ctx->buffer_frame_count, 0 );
    check( hr );

    hr = IAudioClientStart( ctx->client );
    check( hr );

    #undef REFTIMES_PER_MS
    #undef load
    #undef check
    return ctx;

media_audio_initialize_failed:
    #define COMRelease( punk ) do {\
        if( (punk) != NULL ) {\
            (punk)->lpVtbl->Release( (punk) );\
            (punk) = NULL;\
        }\
    } while(0)

    COMRelease( ctx->device_enum );
    COMRelease( ctx->device );
    COMRelease( ctx->client );
    COMRelease( ctx->render_client );

    CoUninitialize();

    HeapFree( GetProcessHeap(), 0, ctx );

    #undef COMRelease
    return NULL;
}
attr_media_api void media_audio_shutdown( MediaAudioContext* in_ctx ) {
    ctx_to_win32( in_ctx );

    #define COMRelease( punk ) do {\
        if( (punk) != NULL ) {\
            (punk)->lpVtbl->Release( (punk) );\
            (punk) = NULL;\
        }\
    } while(0)

    IAudioClientStop( ctx->client );

    COMRelease( ctx->device_enum );
    COMRelease( ctx->device );
    COMRelease( ctx->client );
    COMRelease( ctx->render_client );

    CoUninitialize();

    HeapFree( GetProcessHeap(), 0, ctx );

    #undef COMRelease
}
attr_media_api MediaAudioBufferFormat media_audio_query_buffer_format(
    MediaAudioContext* in_ctx
) {
    ctx_to_win32(in_ctx);

    MediaAudioBufferFormat fmt = {};
    fmt.channel_count       = ctx->format.nChannels;
    fmt.bits_per_sample     = ctx->format.wBitsPerSample;
    fmt.samples_per_second  = ctx->format.nSamplesPerSec;
    fmt.buffer_sample_count = ctx->buffer_frame_count;

    return fmt;
}
attr_media_api b32 media_audio_buffer_lock(
    MediaAudioContext* in_ctx, MediaAudioBuffer* out_buffer
) {
    ctx_to_win32( in_ctx );

    u32 buffer_frame_padding_count = 0;
    IAudioClientGetCurrentPadding( ctx->client, &buffer_frame_padding_count );

    if( buffer_frame_padding_count > ctx->buffer_frame_count ) {
        return false;
    }

    u32 frames_to_request = ctx->buffer_frame_count - buffer_frame_padding_count;
    if( !frames_to_request ) {
        return false;
    }

    BYTE* buf = NULL;
    IAudioRenderClientGetBuffer( ctx->render_client, frames_to_request, &buf );

    if( !buf ) {
        return false;
    }

    out_buffer->buffer       = buf;
    out_buffer->sample_count = frames_to_request;
    out_buffer->buffer_size  = out_buffer->sample_count * ctx->format.nBlockAlign;

    return true;
}
attr_media_api void media_audio_buffer_unlock(
    MediaAudioContext* in_ctx, MediaAudioBuffer* buffer
) {
    ctx_to_win32( in_ctx );

    IAudioRenderClientReleaseBuffer( ctx->render_client, buffer->sample_count, 0 );
    memory_zero( buffer, sizeof(*buffer) );
}
attr_media_api void media_audio_play( MediaAudioContext* in_ctx ) {
    ctx_to_win32( in_ctx );
    IAudioClientStart( ctx->client );
}
attr_media_api void media_audio_pause( MediaAudioContext* in_ctx ) {
    ctx_to_win32( in_ctx );
    IAudioClientStop( ctx->client );
}

#endif /* Platform Windows */

