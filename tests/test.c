// IWYU pragma: begin_keep
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "media/surface.h"
#include "media/input.h"
#include "media/lib.h"
#include "media/opengl.h"
#include "media/prompt.h"
#include "media/audio.h"
#include "media/cursor.h"
// IWYU pragma: end_keep

#define text( lit ) sizeof(lit) - 1, lit

void logging_callback(
    MediaLoggingLevel level, uint32_t len, const char* message, void* params
) {
    unused(params);
    switch( level ) {
        case MEDIA_LOGGING_LEVEL_ERROR: {
            printf( "error: " );
        } break;
        case MEDIA_LOGGING_LEVEL_WARN: {
            printf( "warn: " );
        } break;
        case MEDIA_LOGGING_LEVEL_NONE:
          break;
    }
    printf( "%.*s\n", len, message );
}
static bool is_focused = true;
void surface_callback(
    const SurfaceHandle* surface,
    const SurfaceCallbackData* data,
    void* params
) {
    bool* is_running = params;

    const char* title = surface_query_title( surface, NULL );

    switch( data->type ) {
        case SURFACE_CALLBACK_TYPE_FOCUS: {
            is_focused = data->focus.gained;
            if( data->focus.gained ) {
                printf( "focus gained.\n" );
            } else {
                printf( "focus lost.\n" );
            }
        } break;
        case SURFACE_CALLBACK_TYPE_RESIZE: {
            /* printf( "resize: %i %i\n", data->resize.w, data->resize.h ); */
        } break;
        case SURFACE_CALLBACK_TYPE_POSITION: {
            /* printf( "move: %i %i\n", data->position.x, data->position.y ); */
        } break;
        case SURFACE_CALLBACK_TYPE_TEXT: {
            /* printf( */
            /*     "%c%c%c%c", */
            /*     data->text.utf8[0], data->text.utf8[1], */
            /*     data->text.utf8[2], data->text.utf8[3] ); */
        } break;
        case SURFACE_CALLBACK_TYPE_MOUSE_BUTTON: {
            if( !data->mouse_button.delta ) {
                return;
            }

            bool L  = data->mouse_button.state & MB_LEFT;
            bool M  = data->mouse_button.state & MB_MIDDLE;
            bool R  = data->mouse_button.state & MB_RIGHT;
            bool X1 = data->mouse_button.state & MB_EXTRA_1;
            bool X2 = data->mouse_button.state & MB_EXTRA_2;

            unused( L, M, R, X1, X2 );

            /* printf( "mb: L[%s] M[%s] R[%s] X1[%s] X2[%s]\n", */
            /*     L ? "X" : " ", */
            /*     M ? "X" : " ", */
            /*     R ? "X" : " ", */
            /*     X1 ? "X" : " ", */
            /*     X2 ? "X" : " " ); */
        } break;
        case SURFACE_CALLBACK_TYPE_MOUSE_MOVE: {
            /* printf( "m: %i, %i\n", data->mouse_move.x, data->mouse_move.y ); */
        } break;
        case SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA: {
            /* printf( "dm: %i, %i\n", data->mouse_move_delta.x, data->mouse_move_delta.y ); */
        } break;
        case SURFACE_CALLBACK_TYPE_MOUSE_WHEEL: {
            /* printf( */
            /*     "mwheel: %s delta[%2i]\n", */
            /*     data->mouse_wheel.is_horizontal ? "horizontal" : "vertical  ", */
            /*     data->mouse_wheel.delta ); */
        } break;
        case SURFACE_CALLBACK_TYPE_KEY: {
            KeyboardCode code = data->key.code;
            if( data->key.is_down ) {
                /* printf( "%s down.\n", keyboard_code_to_string( code, 0 ) ); */
            }

            if( code == KB_ESCAPE ) {
                goto surface_callback_close;
            }
        } break;
        case SURFACE_CALLBACK_TYPE_CLOSE: {
surface_callback_close:
            printf( "%s closing . . .\n", title );
            *is_running = false;
        } break;
    }
}

#if defined(MEDIA_PLATFORM_WINDOWS)
extern int SetConsoleOutputCP( unsigned int wCodePageID );
#define CP_UTF8 65001
#endif

double get_ms(void);

int main( int argc, char** argv ) {
    unused(argc, argv);

#if defined(MEDIA_PLATFORM_WINDOWS)
    SetConsoleOutputCP( CP_UTF8 );
#endif

    uintptr_t lib_size     = media_lib_query_memory_requirement();
    uintptr_t surface_size = surface_query_memory_requirement();
    uintptr_t input_size   = input_subsystem_query_memory_requirement();
    uintptr_t buf_size =
        input_size +
        lib_size   +
        surface_size;
    void* buf = malloc( buf_size );
    memset( buf, 0, buf_size );

    void* lib_buf = buf;
    if( !media_lib_initialize(
        MEDIA_LOGGING_LEVEL_WARN, logging_callback,
        0, lib_buf
    ) ) {
        printf("failed to initialize media lib!\n");
        return -1;
    }

    SurfaceHandle* surface = (uint8_t*)lib_buf + lib_size;

    void* input_buf = (void*)surface + surface_size;

    if( !input_subsystem_initialize( input_buf ) ) {
        printf("failed to initialize input subsystem!\n");
        return -1;
    }

    if( !opengl_initialize() ) {
        printf( "failed to initialize opengl subsystem!\n" );
        return -1;
    }

    uintptr_t audio_device_list_size   = audio_device_list_query_memory_requirement();
    AudioDeviceList* audio_device_list = malloc( audio_device_list_size );
    memset( audio_device_list, 0, audio_device_list_size );

    audio_device_list_create( audio_device_list );

    uint32_t input_devices = audio_device_list_query_count(
        audio_device_list, AUDIO_DEVICE_TYPE_INPUT );

    printf( "input devices:\n" );
    for( uint32_t i = 0; i < input_devices; ++i ) {
        char name[260];
        memset( name, 0, sizeof(name) );
        uint32_t len = 0;

        if( audio_device_list_query_name(
            audio_device_list, AUDIO_DEVICE_TYPE_INPUT, i, name, &len
        ) ) {
            printf( "    %u: %.*s\n", i, len, name );
        }
    }

    uint32_t output_devices = audio_device_list_query_count(
        audio_device_list, AUDIO_DEVICE_TYPE_OUTPUT );

    printf( "output devices:\n" );
    for( uint32_t i = 0; i < output_devices; ++i ) {
        char name[260];
        memset( name, 0, sizeof(name) );
        uint32_t len = 0;

        if( audio_device_list_query_name(
            audio_device_list, AUDIO_DEVICE_TYPE_OUTPUT, i, name, &len
        ) ) {
            printf( "    %u: %.*s\n", i, len, name );
        }
    }
    printf( "    %u: default\n", output_devices );

    uintptr_t audio_device_size = audio_device_query_memory_requirement();
    AudioDevice* audio_device   = malloc( audio_device_size );
    memset( audio_device, 0, audio_device_size );

    struct AudioBufferFormat format;
    memset( &format, 0, sizeof(format) );

    format.samples_per_second = 41000;
    format.bits_per_sample    = 16;
    format.channel_count      = 2;

    if( !audio_device_open(
        audio_device_list, &format,
        1000, AUDIO_DEVICE_TYPE_OUTPUT,
        AUDIO_DEVICE_DEFAULT, audio_device
    ) ) {
        return -1;
    }
    audio_device_list_destroy( audio_device_list );
    free( audio_device_list );

    SurfaceCreateFlags flags = 0;

    flags |= SURFACE_CREATE_FLAG_RESIZEABLE;
    flags |= SURFACE_CREATE_FLAG_DARK_MODE;
    flags |= SURFACE_CREATE_FLAG_X_CENTERED;
    flags |= SURFACE_CREATE_FLAG_Y_CENTERED;

    bool is_running = true;

    bool res = surface_create(
        text("Test Surface"),
        0, 0,
        0, 0,
        flags,
        surface_callback, &is_running,
        0, surface );

    if( !res ) {
        goto main_end;
    }

    OpenGLRenderContext* rc = opengl_context_create( surface, NULL );
    if( !rc ) {
        return -1;
    }
    opengl_context_bind( surface, rc );

    typedef void glClearColorFN( float red, float green, float blue, float alpha );
    typedef void glClearFN( unsigned int glenum );
    typedef void glViewportFN( int x, int y, int width, int height );
    #define GL_COLOR_BUFFER_BIT   0x00004000

    glClearFN*      glClear      = opengl_load_proc( "glClear" );
    glClearColorFN* glClearColor = opengl_load_proc( "glClearColor" );
    glViewportFN*   glViewport   = opengl_load_proc( "glViewport" );

    glClearColor( 1.0f, 0, 0, 1.0f );

    audio_device_query_format( audio_device, &format );

    printf( "channel_count:      %u\n", format.channel_count );
    printf( "bits_per_sample:    %u\n", format.bits_per_sample );
    printf( "samples_per_second: %u\n", format.samples_per_second );
    printf( "sample_count:       %u\n", format.sample_count );

    double last_ms = get_ms();

    uint32_t channel_sample_size =
        (format.bits_per_sample / 8);
    printf( "channel_sample_size: %u\n", channel_sample_size );

    float r = 0;
    float g = 0;
    float b = 0;
    double offset0 = 0.0;
    double offset1 = 1.2;
    double offset2 = 1.3;

    const double tone_hz = 256.0;
    int16_t tone_volume  = 3000;

    printf( "tone volume: %i\n", tone_volume );

    int wave_period = format.samples_per_second / tone_hz;

    printf( "wave period: %i\n", wave_period );
    
    double t_sine = 0.0;

    if( !audio_device_start( audio_device ) ) {
        return -1;
    }

    bool lock    = false;
    while( is_running ) {
        input_subsystem_update();
        surface_pump_events();

        int32_t x, y;
        input_mouse_query_position( &x, &y );
        input_mouse_position_to_client( surface, &x, &y );

        int32_t w, h;
        surface_query_dimensions( surface, &w, &h );

        if( x > 0 && x < w && y > 0 && y < h ) {
            if( input_mouse_query_buttons() & MB_LEFT ) {
                lock = true;
            }
        }

        if( input_keyboard_query_key( KB_ESCAPE ) ) {
            lock = false;
        }

        if( lock && is_focused ) {
            cursor_center( surface );
        } else {
        }

        glViewport( 0, 0, w, h );

        glClearColor( r, g, b, 1.0 );
        glClear( GL_COLOR_BUFFER_BIT );

        double elapsed = get_ms() - last_ms;
        elapsed *= 0.001;
        r = (sin( elapsed + offset0 ) + 1.0) / 2.0;
        g = (cos( elapsed + offset1 ) + 1.0) / 2.0;
        b = (sin( elapsed + offset2 ) + 1.0) / 2.0;

        offset1 += 0.00001;
        offset2 += 0.00002;

        struct AudioBuffer buf;
        memset( &buf, 0, sizeof(buf) );
        if( audio_device_buffer_lock( audio_device, &buf ) ) {
            int16_t* out = buf.start;
            for( uint32_t sample = 0; sample < buf.sample_count; ++sample ) {
                double sine_sample = sinf( t_sine );
                int16_t sample_value = (int16_t)(sine_sample * tone_volume);

                switch( format.bits_per_sample ) {
                    case 16: {
                        *out++ = sample_value;
                        *out++ = sample_value;
                    } break;
                }

                t_sine += 2.0 * 3.14159 * 1.0 / (double)wave_period;
            }

            audio_device_buffer_unlock( audio_device, &buf );
        }

        opengl_swap_buffers( surface );
    }

    audio_device_stop( audio_device );
    audio_device_close( audio_device );
    free( audio_device );

    opengl_context_unbind();
    opengl_context_destroy( rc );

    surface_destroy( surface );

main_end:
    input_subsystem_shutdown();
    media_lib_shutdown();

    free( buf );
    return 0;
}

#if defined(MEDIA_PLATFORM_WINDOWS)
#include <windows.h>
double get_ms(void) {
    LARGE_INTEGER qpc, qpf;
    QueryPerformanceCounter( &qpc );
    QueryPerformanceFrequency( &qpf );

    return ((double)qpc.QuadPart / (double)qpf.QuadPart) * 1000.0;
}
#else
#include <time.h>
double get_ms(void) {
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    return ((double)ts.tv_nsec / 1000000.0) + ((double)ts.tv_sec * 1000.0);
}
#endif

