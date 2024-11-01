/**
 * @file   opengl.c
 * @brief  X11 OpenGL implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 23, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_LINUX)
#include "media/opengl.h"
#include "media/internal/logging.h"
#include "impl/x11/surface.h"
#include <GL/glx.h>
#include <stdlib.h>

struct X11GLRC {
    struct X11Surface* surf;
    GLXContext         ctx;
};

int global_x11_gl_visual[] = {
    GLX_X_RENDERABLE,  1,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE,   GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE,      8,
    GLX_GREEN_SIZE,    8,
    GLX_BLUE_SIZE,     8,
    GLX_ALPHA_SIZE,    8,
    GLX_DEPTH_SIZE,    24,
    GLX_STENCIL_SIZE,  0,
    GLX_DOUBLEBUFFER,  1,
    0
};

#define GLX11_RED_IDX       9
#define GLX11_GREEN_IDX     11
#define GLX11_BLUE_IDX      13
#define GLX11_ALPHA_IDX     15
#define GLX11_DEPTH_IDX     17
#define GLX11_STENCIL_IDX   19
#define GLX11_DOUBLEBUF_IDX 21

attr_global int global_x11_gl_attr[] = {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    GLX_CONTEXT_FLAGS_ARB,         0,
    0
};
#define GLX11_MAJOR_IDX   1
#define GLX11_MINOR_IDX   3
#define GLX11_PROFILE_IDX 5
#define GLX11_FLAGS_IDX   7

typedef void glXSwapIntervalEXTFN( Display* dpy, GLXDrawable drawable, int interval );
typedef void* glXGetProcAddressARBFN( const GLubyte* procName );
typedef GLXContext glXCreateContextAttribsARBFN(
    Display* display, GLXFBConfig config,
    GLXContext share, Bool direct, const int* attribs );

attr_internal void glXSwapIntervalEXT_stub(
    Display* dpy, GLXDrawable drawable, int interval
) {
    unused(dpy,drawable,interval);
}

attr_global glXSwapIntervalEXTFN*
    __in_glXSwapIntervalEXT = glXSwapIntervalEXT_stub;
attr_global glXGetProcAddressARBFN*
    __in_glXGetProcAddressARB = NULL;
attr_global glXCreateContextAttribsARBFN*
    __in_glXCreateContextAttribsARB = NULL;

#define glXSwapIntervalEXT   __in_glXSwapIntervalEXT
#define glXGetProcAddressARB __in_glXGetProcAddressARB
#define glXCreateContextAttribsARB __in_glXCreateContextAttribsARB

attr_media_api _Bool opengl_initialize(void) {
    if( !glXGetProcAddressARB ) {
        glXGetProcAddressARB =
            (glXGetProcAddressARBFN*)glXGetProcAddress(
                (const GLubyte*)"glXGetProcAddressARB" );
        if( !glXGetProcAddressARB ) {
            media_error( "X11: GL: glXGetProcAddressARB is not available!" );
            return false;
        }
    }

    if( !glXCreateContextAttribsARB ) {
        glXCreateContextAttribsARB =
            (glXCreateContextAttribsARBFN*)glXGetProcAddressARB(
                (const GLubyte*)"glXCreateContextAttribsARB" );
        if( !glXCreateContextAttribsARB ) {
            media_error( "X11: GL: glXCreateContextAttribsARB is not available!" );
            return false;
        }
    }

    glXSwapIntervalEXTFN* swap_interval =
        (glXSwapIntervalEXTFN*)glXGetProcAddress( (const GLubyte*)"glXSwapIntervalEXT" );
    if( swap_interval ) {
        glXSwapIntervalEXT = swap_interval;
    } else {
        media_warn( "X11: GL: glXSwapIntervalEXT is not available!" );
    }

    return true;
}
attr_media_api OpenGLRenderContext* opengl_context_create(
    SurfaceHandle* surface
) {
    struct X11GLRC* glrc = calloc( 1, sizeof(*glrc) );
    if( !glrc ) {
        media_error( "X11: GL: Failed to allocate GLRC!" );
        return NULL;
    }

    struct X11Surface* surf = surface;
    GLXContext context = glXCreateContextAttribsARB( 
        surf->display, surf->fb_config, NULL, true, global_x11_gl_attr );

    if( !context ) {
        media_error( "X11: GL: Failed to create context!" );
        free( glrc );
        return NULL;
    }

    glrc->ctx  = context;
    glrc->surf = surf;

    return glrc;
}
attr_media_api void opengl_context_destroy( OpenGLRenderContext* glrc ) {
    struct X11GLRC* rc = glrc;
    glXDestroyContext( rc->surf->display, rc->ctx );
    free( rc );
}
attr_media_api _Bool opengl_context_bind(
    SurfaceHandle* surface, OpenGLRenderContext* glrc
) {
    struct X11GLRC*    rc   = glrc;
    struct X11Surface* surf = surface;

    if( !glrc ) {
        return glXMakeCurrent( surf->display, None, NULL ) != False;
    } else {
        return glXMakeCurrent( surf->display, surf->window, rc->ctx ) != False;
    }
}
attr_media_api _Bool opengl_attr_set( OpenGLAttribute name, int value ) {
    switch( name ) {
        case OPENGL_ATTR_RED_SIZE: {
            global_x11_gl_visual[GLX11_RED_IDX] = value;
        } return true;
        case OPENGL_ATTR_GREEN_SIZE: {
            global_x11_gl_visual[GLX11_GREEN_IDX] = value;
        } return true;
        case OPENGL_ATTR_BLUE_SIZE: {
            global_x11_gl_visual[GLX11_BLUE_IDX] = value;
        } return true;
        case OPENGL_ATTR_ALPHA_SIZE: {
            global_x11_gl_visual[GLX11_ALPHA_IDX] = value;
        } return true;
        case OPENGL_ATTR_DEPTH_SIZE: {
            global_x11_gl_visual[GLX11_DEPTH_IDX] = value;
        } return true;
        case OPENGL_ATTR_STENCIL_SIZE: {
            global_x11_gl_visual[GLX11_STENCIL_IDX] = value;
        } return true;
        case OPENGL_ATTR_PROFILE: {
            switch( (OpenGLProfile)value ) {
                case OPENGL_PROFILE_CORE: {
                    global_x11_gl_attr[GLX11_PROFILE_IDX] =
                        GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
                } return true;
                case OPENGL_PROFILE_COMPATIBILITY: {
                    global_x11_gl_attr[GLX11_PROFILE_IDX] =
                        GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                } return true;
            }
        } break;
        case OPENGL_ATTR_MAJOR: {
            global_x11_gl_attr[GLX11_MAJOR_IDX] = value;
        } return true;
        case OPENGL_ATTR_MINOR: {
            global_x11_gl_attr[GLX11_MINOR_IDX] = value;
        } return true;
        case OPENGL_ATTR_DOUBLE_BUFFER: {
            global_x11_gl_visual[GLX11_DOUBLEBUF_IDX] = value ? 1 : 0;
        } return true;
        case OPENGL_ATTR_DEBUG: {
            if( value ) {
                global_x11_gl_attr[GLX11_FLAGS_IDX] |= GLX_CONTEXT_DEBUG_BIT_ARB;
            } else {
                global_x11_gl_attr[GLX11_FLAGS_IDX] &= ~GLX_CONTEXT_DEBUG_BIT_ARB;
            }
        } return true;
        case OPENGL_ATTR_FORWARD_COMPATIBILITY: {
            if( value ) {
                global_x11_gl_attr[GLX11_FLAGS_IDX] |=
                    GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            } else {
                global_x11_gl_attr[GLX11_FLAGS_IDX] &=
                    ~GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            }
        } return true;
    }

    return false;
}
attr_media_api int32_t opengl_attr_get( OpenGLAttribute name ) {
    switch( name ) {
        case OPENGL_ATTR_RED_SIZE: {
            return global_x11_gl_visual[GLX11_RED_IDX];
        } break;
        case OPENGL_ATTR_GREEN_SIZE: {
            return global_x11_gl_visual[GLX11_GREEN_IDX];
        } break;
        case OPENGL_ATTR_BLUE_SIZE: {
            return global_x11_gl_visual[GLX11_BLUE_IDX];
        } break;
        case OPENGL_ATTR_ALPHA_SIZE: {
            return global_x11_gl_visual[GLX11_ALPHA_IDX];
        } break;
        case OPENGL_ATTR_DEPTH_SIZE: {
            return global_x11_gl_visual[GLX11_DEPTH_IDX];
        } break;
        case OPENGL_ATTR_STENCIL_SIZE: {
            return global_x11_gl_visual[GLX11_STENCIL_IDX];
        } break;
        case OPENGL_ATTR_PROFILE: {
            int val = global_x11_gl_attr[GLX11_PROFILE_IDX];
            if( val == GLX_CONTEXT_CORE_PROFILE_BIT_ARB ) {
                return OPENGL_PROFILE_CORE;
            }
            if( val == GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB ) {
                return OPENGL_PROFILE_COMPATIBILITY;
            }
            return -1;
        } break;
        case OPENGL_ATTR_MAJOR: {
            return global_x11_gl_attr[GLX11_MAJOR_IDX];
        } break;
        case OPENGL_ATTR_MINOR: {
            return global_x11_gl_attr[GLX11_MINOR_IDX];
        } break;
        case OPENGL_ATTR_DOUBLE_BUFFER: {
            return global_x11_gl_visual[GLX11_DOUBLEBUF_IDX];
        } break;
        case OPENGL_ATTR_DEBUG: {
            return global_x11_gl_attr[GLX11_FLAGS_IDX] & GLX_CONTEXT_DEBUG_BIT_ARB;
        } break;
        case OPENGL_ATTR_FORWARD_COMPATIBILITY: {
            return global_x11_gl_attr[GLX11_FLAGS_IDX] &
                GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
        } break;
    }

    return -1;
}
attr_media_api _Bool opengl_context_share(
    OpenGLRenderContext* src, OpenGLRenderContext* dst
) {
    // TODO(alicia): 
    unused(src,dst);
    return false;
}
attr_media_api void* opengl_load_proc( const char* function_name ) {
    return glXGetProcAddressARB( (const GLubyte*)function_name );
}
attr_media_api _Bool opengl_swap_buffers( SurfaceHandle* surface ) {
    struct X11Surface* surf = surface;
    glXSwapBuffers( surf->display, surf->window );
    return true;
}
attr_media_api _Bool opengl_swap_interval(
    SurfaceHandle* surface, int interval
) {
    struct X11Surface* surf = surface;
    glXSwapIntervalEXT( surf->display, surf->window, interval );
    return true;
}

#endif /* Platform Linux */

