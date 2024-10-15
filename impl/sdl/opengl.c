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

attr_media_api OpenGLAttributeList opengl_attr_create(void) {
}
attr_media_api _Bool opengl_attr_set(
    OpenGLAttributeList* attr, OpenGLAttribute name, int value ) {
}
attr_media_api int32_t opengl_attr_get(
    OpenGLAttributeList* attr, OpenGLAttribute name ) {
}
attr_media_api _Bool opengl_initialize(void) {
    return true;
}
attr_media_api OpenGLRenderContext* opengl_context_create(
    SurfaceHandle* surface, OpenGLAttributeList* opt_attributes
) {

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

