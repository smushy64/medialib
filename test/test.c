/**
 * @file   test.c
 * @brief  Test file for media lib.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
// IWYU pragma: begin_keep
#include "core/prelude.h"
#include "core/print.h"
#include "core/thread.h"
#include "core/sync.h"
#include "core/memory.h"
#include "core/math.h"
#include "core/collections.h"
#define CORE_ENABLE_TIME_GLOBAL_ALIAS
#include "core/time.h"
#include "media/lib.h"
#include "media/surface.h"
#include "media/prompt.h"
#include "media/render.h"
#include "media/cursor.h"
#include "media/keyboard.h"
#include "media/gamepad.h"
#include "media/mouse.h"
#include "media/audio.h"
// IWYU pragma: end_keep

#define success( format, ... )\
    println( str_green( format ), ##__VA_ARGS__ )
#define error( format, ... )\
    println( str_red( format ), ##__VA_ARGS__ )

void log_callback(
    CoreLoggingLevel level, usize len,
    const char* msg, va_list va, void* params
) {
    Mutex* mtx = params;
    mutex_lock( mtx );
    switch( level ) {
        case CORE_LOGGING_LEVEL_ERROR: {
            print_err( CONSOLE_COLOR_RED "error: " );
            print_err_text_va( len, msg, va );
            println_err( CONSOLE_COLOR_RESET );
        } break;
        case CORE_LOGGING_LEVEL_WARN: {
            print( CONSOLE_COLOR_YELLOW "warning: " );
            print_text_va( len, msg, va );
            println( CONSOLE_COLOR_RESET );
        } break;
        default: break;
    }
    mutex_unlock( mtx );
}

void callback(
    const MediaSurface* surface, const MediaSurfaceCallbackData* data,
    void* params
) {
    b32* should_run = params;
    String name = media_surface_query_name( surface );
    unused(name);

    switch( data->type ) {
        case MEDIA_SURFACE_CALLBACK_TYPE_CLOSE: {
            if( should_run ) {
                *should_run = false;
            }
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_FOCUS: {
            // if( data->focus_update.is_focused ) {
            //     println( str_cyan( "{s}: gained focus!" ), name );
            // } else {
            //     println( str_magenta( "{s}: lost focus!" ), name );
            // }
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_POSITION: {
            // println(
            //     "{s}: {i},{i} -> {i},{i}",
            //     name, data->position_update.old_x, data->position_update.old_y,
            //     data->position_update.new_x, data->position_update.new_y );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_RESIZE: {
            // println(
            //     "{s}: {i}x{i} -> {i}x{i}",
            //     name, data->dimensions_update.old_w, data->dimensions_update.old_h,
            //     data->dimensions_update.new_w, data->dimensions_update.new_h );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_BUTTON: {
            // println(
            //     "{s}: [L: {b} R: {b} M: {b} X1: {b} X2: {b}]",
            //     name,
            //     bitfield_check( data->mouse_button.buttons_state, INPUT_MOUSE_BUTTON_LEFT ),
            //     bitfield_check( data->mouse_button.buttons_state, INPUT_MOUSE_BUTTON_RIGHT ),
            //     bitfield_check( data->mouse_button.buttons_state, INPUT_MOUSE_BUTTON_MIDDLE ),
            //     bitfield_check( data->mouse_button.buttons_state, INPUT_MOUSE_BUTTON_EXTRA_1 ),
            //     bitfield_check( data->mouse_button.buttons_state, INPUT_MOUSE_BUTTON_EXTRA_2 )
            // );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE: {
            // println( "{s}: mouse: {i,6}, {i,6}",
            //     name, data->mouse_move.x, data->mouse_move.y );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA: {
            // println( "{s}: mouse: {i,4}, {i,4}",
            //     name, data->mouse_move_rel.rel_x, data->mouse_move_rel.rel_y );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_WHEEL: {
            // println(
            //     "{s}: wheel {cc}: {i,2}",
            //     name, data->mouse_wheel.is_horizontal ? "x" : "y",
            //     data->mouse_wheel.delta );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_KEY: {
            // String keyname;
            // keyname.cc = input_keycode_to_string( data->key.code, &keyname.len );
            // println(
            //     "{s}: {s} {cc}",
            //     name, keyname, data->key.is_down ? "is down" : "is up" );
        } break;
        case MEDIA_SURFACE_CALLBACK_TYPE_TEXT: {
            // String text = surface_callback_data_text_to_string( data );
            // print( "{s}", text );
        } break;
    }
}

typedef u32 GLenum;
typedef i32 GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef float GLfloat;

typedef void glViewportFN( GLint x, GLint y, GLsizei width, GLsizei height );
typedef void glClearFN( GLbitfield mask );
typedef void glClearColorFN( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );

#define GL_DEPTH_BUFFER_BIT   0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT   0x00004000

struct SurfaceThreadParams {
    MediaSurface* surface;
    volatile i32 status;
    volatile i32 end;
};

int surface_thread( u32 thread_id, void* in_params ) {
    unused(thread_id);
    struct SurfaceThreadParams* params = in_params;

    b32 should_run = true;
    params->surface = media_surface_create(
        string_text( "[0]" ), 900, 0, 800, 600,
        MEDIA_SURFACE_CREATE_FLAG_OPENGL      |
        MEDIA_SURFACE_CREATE_FLAG_RESIZEABLE  |
        MEDIA_SURFACE_CREATE_FLAG_DARK_MODE,
        callback, &should_run, NULL );

    if( !params->surface ) {
        read_write_fence();
        params->status = -1;
        return -1;
    }

    read_write_fence();
    params->status = 1;
    while( should_run ) {
        media_surface_pump_events( params->surface );
    }

    read_write_fence();
    params->status = 0;

    return 0;
}

int main( int argc, char** argv ) {
    Mutex mtx = {};
    mutex_create( &mtx );
    b32 result = media_initialize( CORE_LOGGING_LEVEL_WARN, log_callback, &mtx );
    if( !result ) {
        mutex_destroy( &mtx );
        return 0;
    }

    MediaAudioContext* audio = media_audio_initialize( 800 );
    if( !audio ) {
        media_shutdown();
        return 0;
    }

    struct SurfaceThreadParams thread_params = {};
    read_write_fence();
    Thread* thread =
        thread_create( surface_thread, &thread_params, mebibytes(1) );
    if( !thread ) {
        error( "failed to create surface thread!" );
        return -1;
    }

    read_write_fence();

    while( !thread_params.status ) {}
    if( thread_params.status < 0 ) {
        return -1;
    }

    read_write_fence();

    if( !media_render_gl_initialize() ) {
        error( "failed to initialize gl!" );
        return -1;
    }

    OpenGLAttributes* attr = media_render_gl_attr_create();
    if( !attr ) {
        error( "failed to create OpenGL attributes!" );
        return -1;
    }
    media_render_gl_attr_set(
        attr, MEDIA_OPENGL_ATTR_PROFILE, MEDIA_OPENGL_PROFILE_CORE );
    media_render_gl_attr_set( attr, MEDIA_OPENGL_ATTR_MAJOR, 4 );
    media_render_gl_attr_set( attr, MEDIA_OPENGL_ATTR_MINOR, 5 );
    media_render_gl_attr_set( attr, MEDIA_OPENGL_ATTR_DEBUG, false );
    media_render_gl_attr_set( attr, MEDIA_OPENGL_ATTR_FORWARD_COMPATIBILITY, false );
    OpenGLRenderContext* glrc =
        media_render_gl_context_create( thread_params.surface, attr );

    media_render_gl_attr_destroy( attr );

    if( !glrc ) {
        return -1;
    }
    if( !media_render_gl_context_bind( thread_params.surface, glrc ) ) {
        error( "failed to bind context!" );
        return -1;
    }

    #define load( fn ) \
    fn##FN* fn = NULL;\
    do {\
        fn = (fn##FN*)media_render_gl_load_proc( #fn );\
        if( !fn ) {\
            error( "failed to load " #fn "!" );\
            return -1;\
        }\
    } while(0)

    load( glViewport );
    load( glClear );
    load( glClearColor );

    hsl bg     = hsl( 0.0f, 1.0f, 0.8f );
    rgb bg_rgb;

    #define update_bg()\
        bg_rgb = hsl_to_rgb( bg );\
        glClearColor( bg_rgb.r, bg_rgb.g, bg_rgb.b, 1.0f )

    i32 w = 0, h = 0;
    update_bg();

    StringBuf title;
    string_buf_new_alloc( 255, 0, &title );

    media_render_gl_swap_interval( thread_params.surface, 1 );

    time_global_timekeeping_initialize();
    // media_cursor_lock( thread_params.surface, true );

    #define UPDATE_COLOR_TIME (0.02f)
    while( thread_params.status ) {

        time_global_timekeeping_update();
        f32 dt  = delta_time();
        f32 fps = dt == 0.0f ? 0.0f : 1.0f / dt;

        string_buf_fmt( &title, "{f,.2} FPS{0}", fps );
        media_surface_set_name( thread_params.surface, title.slice );

        i32 cw, ch;
        media_surface_query_dimensions( thread_params.surface, &cw, &ch );
        if( cw != w || ch != h ) {
            w = cw; h = ch;
            glViewport( 0, 0, w, h );
        }

        glClear( GL_COLOR_BUFFER_BIT );
        media_render_gl_swap_buffers( thread_params.surface );

        title.len = 0;
    }

    success( "everything seems good!" );
    media_render_gl_context_unbind();
    media_render_gl_context_destroy( glrc );
    media_surface_destroy( thread_params.surface );

    media_audio_shutdown( audio );
    media_shutdown();

    string_buf_free( &title );
    unused( argc, argv );
    mutex_destroy( &mtx );
    return 0;
}

