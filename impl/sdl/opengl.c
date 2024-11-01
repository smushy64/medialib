/**
 * @file   opengl.c
 * @brief  Posix OpenGL implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 14, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_POSIX)
#include "media/opengl.h"
#include "impl/sdl/surface.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

struct SDLOpenGLContext {
    SDL_Window*   window;
    SDL_GLContext ctx;
};

attr_media_api _Bool opengl_attr_set( OpenGLAttribute name, int value ) {
    // NOTE(alicia): SDL documentation specifically says not to do this 
    // after creating a window but after reviewing the docs, it seems to
    // not really affect linux (wayland or X11) so it's fine.

    switch( name ) {
        case OPENGL_ATTR_RED_SIZE              : {
            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, value );
        } return true;
        case OPENGL_ATTR_GREEN_SIZE            : {
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, value );
        } return true;
        case OPENGL_ATTR_BLUE_SIZE             : {
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, value );
        } return true;
        case OPENGL_ATTR_ALPHA_SIZE            : {
            SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, value );
        } return true;
        case OPENGL_ATTR_DEPTH_SIZE            : {
            SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, value );
        } return true;
        case OPENGL_ATTR_STENCIL_SIZE          : {
            SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, value );
        } return true;
        case OPENGL_ATTR_PROFILE               : {
            SDL_GLprofile profile;
            switch( (OpenGLProfile)value ) {
                case OPENGL_PROFILE_CORE: {
                    profile = SDL_GL_CONTEXT_PROFILE_CORE;
                } break;
                case OPENGL_PROFILE_COMPATIBILITY: {
                    profile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
                } break;
                default: return false;
            }
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, profile );
        } return true;
        case OPENGL_ATTR_MAJOR                 : {
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, value );
        } return true;
        case OPENGL_ATTR_MINOR                 : {
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, value );
        } return true;
        case OPENGL_ATTR_DOUBLE_BUFFER         : {
            SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, value );
        } return true;
        case OPENGL_ATTR_DEBUG                 : {
            int prev = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_FLAGS, &prev );
            if( value ) {
                prev |= SDL_GL_CONTEXT_DEBUG_FLAG;
            } else {
                prev &= ~SDL_GL_CONTEXT_DEBUG_FLAG;
            }
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, prev );
        } return true;
        case OPENGL_ATTR_FORWARD_COMPATIBILITY : {
            int prev = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_FLAGS, &prev );
            if( value ) {
                prev |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
            } else {
                prev &= ~SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
            }
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, prev );
        } return true;
    }
    return false;
}
attr_media_api int32_t opengl_attr_get( OpenGLAttribute name ) {
    switch( name ) {
        case OPENGL_ATTR_RED_SIZE              : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &result );
            return result;
        } break;
        case OPENGL_ATTR_GREEN_SIZE            : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &result );
            return result;
        } break;
        case OPENGL_ATTR_BLUE_SIZE             : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &result );
            return result;
        } break;
        case OPENGL_ATTR_ALPHA_SIZE            : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &result );
            return result;
        } break;
        case OPENGL_ATTR_DEPTH_SIZE            : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &result );
            return result;
        } break;
        case OPENGL_ATTR_STENCIL_SIZE          : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &result );
            return result;
        } break;
        case OPENGL_ATTR_PROFILE               : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, &result );

            switch( (SDL_GLprofile)result ) {
                case SDL_GL_CONTEXT_PROFILE_CORE:
                    return OPENGL_PROFILE_CORE;
                case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
                    return OPENGL_PROFILE_COMPATIBILITY;
                case SDL_GL_CONTEXT_PROFILE_ES: return -1;
            }
        } break;
        case OPENGL_ATTR_MAJOR                 : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &result );
            return result;
        } break;
        case OPENGL_ATTR_MINOR                 : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &result );
            return result;
        } break;
        case OPENGL_ATTR_DOUBLE_BUFFER         : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &result );
            return result;
        } break;
        case OPENGL_ATTR_DEBUG                 : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_FLAGS, &result );
            return result & SDL_GL_CONTEXT_DEBUG_FLAG;
        } break;
        case OPENGL_ATTR_FORWARD_COMPATIBILITY : {
            int32_t result = 0;
            SDL_GL_GetAttribute( SDL_GL_CONTEXT_FLAGS, &result );
            return result & SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
        } break;
    }
    return -1;
}
attr_media_api _Bool opengl_initialize(void) {
    opengl_attr_set( OPENGL_ATTR_RED_SIZE, 8 );
    opengl_attr_set( OPENGL_ATTR_GREEN_SIZE, 8 );
    opengl_attr_set( OPENGL_ATTR_BLUE_SIZE, 8 );
    opengl_attr_set( OPENGL_ATTR_ALPHA_SIZE, 8 );
    opengl_attr_set( OPENGL_ATTR_DEPTH_SIZE, 24 );
    opengl_attr_set( OPENGL_ATTR_STENCIL_SIZE, 0 );
    opengl_attr_set( OPENGL_ATTR_PROFILE, OPENGL_PROFILE_CORE );
    opengl_attr_set( OPENGL_ATTR_MAJOR, 3 );
    opengl_attr_set( OPENGL_ATTR_MINOR, 2 );
    opengl_attr_set( OPENGL_ATTR_DOUBLE_BUFFER, 1 );
    opengl_attr_set( OPENGL_ATTR_DEBUG, 0 );
    opengl_attr_set( OPENGL_ATTR_FORWARD_COMPATIBILITY, 0 );
    return true;
}
attr_media_api OpenGLRenderContext* opengl_context_create( SurfaceHandle* surface ) {
    struct SDLSurface* surf = surface;
    return SDL_GL_CreateContext( surf->handle );
}
attr_media_api _Bool opengl_context_bind(
    SurfaceHandle* surface, OpenGLRenderContext* glrc
) {
    struct SDLOpenGLContext* ctx = glrc;
    struct SDLSurface* surf = surface;
    if( SDL_GL_MakeCurrent( surf->handle, ctx->ctx ) ) {
        ctx->window = surf->handle;
        return true;
    }
    return false;
}
attr_media_api void opengl_context_destroy( OpenGLRenderContext* glrc ) {
    struct SDLOpenGLContext* rc = glrc;
    SDL_GL_DestroyContext( rc->ctx );
    SDL_free( rc );
}
attr_media_api _Bool opengl_context_share(
    OpenGLRenderContext* src, OpenGLRenderContext* dst
) {
    struct SDLOpenGLContext* _src = src;
    struct SDLOpenGLContext* _dst = dst;

    _dst->window = _src->window;
    _dst->ctx    = SDL_GL_CreateContext( _src->window );

    if( !_dst->ctx ) {
        _dst->window = NULL;
        _dst->ctx    = NULL;
        return false;
    }
    return true;
}
attr_media_api void* opengl_load_proc( const char* function_name ) {
    return SDL_GL_GetProcAddress( function_name );
}
attr_media_api _Bool opengl_swap_buffers( SurfaceHandle* surface ) {
    struct SDLSurface* surf = surface;
    return SDL_GL_SwapWindow( surf->handle );
}
attr_media_api _Bool opengl_swap_interval(
    SurfaceHandle* surface, int interval
) {
    unused(surface);
    return SDL_GL_SetSwapInterval( interval );
}

#endif /* Platform Posix */

