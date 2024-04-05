/**
 * @file   opengl.c
 * @brief  Win32 opengl implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "impl/win32/common.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "core/prelude.h"
#include "core/sync.h"

#include "media/internal/logging.h"
#include "media/render.h"

#include "impl/win32/surface.h"

struct Win32OpenGLAttributes {
    DWORD dwFlags;
    int red, green, blue, alpha, depth, stencil;

    union {
        struct {
            int __profile_mask;
            int profile;
            int __major_mask;
            int major;
            int __minor_mask;
            int minor;
            int __context_mask;
            int context_flags;
            int null_terminator;
        };
        int attribs[9];
    };
};
static_assert(
    sizeof(MediaOpenGLAttributes) >= sizeof(struct Win32OpenGLAttributes),
    "MediaOpenGLAttributes is smaller than windows version!" );

#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB          0x20A9
#define ERROR_INVALID_VERSION_ARB                 0x2095
#define ERROR_INVALID_PROFILE_ARB                 0x2096

#define def( ret, fn, ... )\
typedef ret fn##FN( __VA_ARGS__ );\
attr_internal fn##FN* in_##fn = NULL

// NOTE(alicia): GDI32 declarations

def( int, DescribePixelFormat, HDC, int, UINT, LPPIXELFORMATDESCRIPTOR );
#define DescribePixelFormat in_DescribePixelFormat

def( int, ChoosePixelFormat, HDC, const PIXELFORMATDESCRIPTOR* );
#define ChoosePixelFormat in_ChoosePixelFormat

def( BOOL, SetPixelFormat, HDC, int, const PIXELFORMATDESCRIPTOR* );
#define SetPixelFormat in_SetPixelFormat

def( BOOL, SwapBuffers, HDC );
#define SwapBuffers in_SwapBuffers

// NOTE(alicia): OPENGL32 declarations

def( HGLRC, wglCreateContext, HDC );
#define wglCreateContext in_wglCreateContext

def( BOOL, wglDeleteContext, HGLRC );
#define wglDeleteContext in_wglDeleteContext

def( BOOL, wglMakeCurrent, HDC, HGLRC );
#define wglMakeCurrent in_wglMakeCurrent

def( PROC, wglGetProcAddress, LPCSTR );
#define wglGetProcAddress in_wglGetProcAddress

def( BOOL, wglShareLists, HGLRC, HGLRC );
#define wglShareLists in_wglShareLists

def( BOOL, wglCopyContext, HGLRC, HGLRC, UINT );
#define wglCopyContext in_wglCopyContext

def( HGLRC, wglCreateContextAttribsARB, HDC, HGLRC, const int* );
#define wglCreateContextAttribsARB in_wglCreateContextAttribsARB

def( BOOL, wglSwapIntervalEXT, int );
#define wglSwapIntervalEXT in_wglSwapIntervalEXT

#undef def

attr_media_api b32 media_render_gl_initialize(void) {
    if( global_win32_state->gl_initialized ) {
        return true;
    }

    #define open( name ) do {\
        if( !global_win32_state->name ) {\
            global_win32_state->name = LoadLibraryA( #name ".DLL" );\
            if( !global_win32_state->name ) {\
                win32_error( "failed to open library " #name "!" );\
                return false;\
            }\
        }\
    } while(0)

    #define load( mod, name ) do {\
        name = (name##FN*)GetProcAddress( global_win32_state->mod, #name );\
        if( !name ) {\
            win32_error( "win32: failed to load " #name " from library " #mod "!" );\
            return false;\
        }\
    } while(0)

    open( OPENGL32 );
    open( GDI32 );

    load( OPENGL32, wglCreateContext );
    load( OPENGL32, wglDeleteContext );
    load( OPENGL32, wglMakeCurrent );
    load( OPENGL32, wglGetProcAddress );
    load( OPENGL32, wglShareLists );
    load( OPENGL32, wglCopyContext );

    load( GDI32, DescribePixelFormat );
    load( GDI32, ChoosePixelFormat );
    load( GDI32, SetPixelFormat );
    load( GDI32, SwapBuffers );

    #undef open
    #undef load

    read_write_fence();
    interlocked_increment( &global_win32_state->gl_initialized );
    return true;
}

attr_internal void win32_gl_set_default_attr( struct Win32OpenGLAttributes* attr ) {
    attr->dwFlags        = PFD_DOUBLEBUFFER;
    attr->red            = 8;
    attr->green          = 8;
    attr->blue           = 8;
    attr->alpha          = 8;
    attr->depth          = 24;
    attr->stencil        = 0;
    attr->__profile_mask = WGL_CONTEXT_PROFILE_MASK_ARB;
    attr->profile        = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
    attr->__major_mask   = WGL_CONTEXT_MAJOR_VERSION_ARB;
    attr->major          = MEDIA_OPENGL_DEFAULT_MAJOR_VERSION;
    attr->__minor_mask   = WGL_CONTEXT_MINOR_VERSION_ARB;
    attr->minor          = MEDIA_OPENGL_DEFAULT_MINOR_VERSION;
    attr->__context_mask = WGL_CONTEXT_FLAGS_ARB;
    attr->context_flags  = 0;
}

attr_media_api MediaOpenGLAttributes media_render_gl_attr_create(void) {
    struct Win32OpenGLAttributes attr = {};
    win32_gl_set_default_attr( &attr );
    return rcast( MediaOpenGLAttributes, &attr );
}
attr_media_api b32 media_render_gl_attr_set(
    MediaOpenGLAttributes* in_attr, MediaOpenGLAttribute name, int value
) {
    struct Win32OpenGLAttributes* attr = (struct Win32OpenGLAttributes*)in_attr;
    switch( name ) {
        case MEDIA_OPENGL_ATTR_RED_SIZE: {
            attr->red = value;
        } break;
        case MEDIA_OPENGL_ATTR_GREEN_SIZE: {
            attr->green = value;
        } break;
        case MEDIA_OPENGL_ATTR_BLUE_SIZE: {
            attr->blue = value;
        } break;
        case MEDIA_OPENGL_ATTR_ALPHA_SIZE: {
            attr->alpha = value;
        } break;
        case MEDIA_OPENGL_ATTR_DEPTH_SIZE: {
            attr->depth = value;
        } break;
        case MEDIA_OPENGL_ATTR_STENCIL_SIZE: {
            attr->stencil = value;
        } break;
        case MEDIA_OPENGL_ATTR_PROFILE: {
            switch( (MediaOpenGLProfile)value ) {
                case MEDIA_OPENGL_PROFILE_CORE: {
                    attr->profile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
                } break;
                case MEDIA_OPENGL_PROFILE_COMPATIBILITY: {
                    attr->profile = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                } break;
                default: return false;
            }
        } break;
        case MEDIA_OPENGL_ATTR_MAJOR: {
            attr->major = value;
        } break;
        case MEDIA_OPENGL_ATTR_MINOR: {
            attr->minor = value;
        } break;
        case MEDIA_OPENGL_ATTR_DOUBLE_BUFFER: {
            attr->dwFlags = value ? PFD_DOUBLEBUFFER : 0;
        } break;
        case MEDIA_OPENGL_ATTR_DEBUG: {
            attr->context_flags = value ?
                bitfield_set( attr->context_flags, WGL_CONTEXT_DEBUG_BIT_ARB ) :
                bitfield_clear( attr->context_flags, WGL_CONTEXT_DEBUG_BIT_ARB );
        } break;
        case MEDIA_OPENGL_ATTR_FORWARD_COMPATIBILITY: {
            attr->context_flags = value ?
                bitfield_set( attr->context_flags, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB ) :
                bitfield_clear( attr->context_flags, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB );
        } break;

        default: return false;
    }
    return true;
}
attr_media_api int media_render_gl_attr_get(
    MediaOpenGLAttributes* in_attr, MediaOpenGLAttribute name
) {
    struct Win32OpenGLAttributes* attr = (struct Win32OpenGLAttributes*)in_attr;
    switch( name ) {
        case MEDIA_OPENGL_ATTR_RED_SIZE:     return attr->red;
        case MEDIA_OPENGL_ATTR_GREEN_SIZE:   return attr->green;
        case MEDIA_OPENGL_ATTR_BLUE_SIZE:    return attr->blue;
        case MEDIA_OPENGL_ATTR_ALPHA_SIZE:   return attr->alpha;
        case MEDIA_OPENGL_ATTR_DEPTH_SIZE:   return attr->depth;
        case MEDIA_OPENGL_ATTR_STENCIL_SIZE: return attr->stencil;
        case MEDIA_OPENGL_ATTR_PROFILE: {
            if( attr->profile == WGL_CONTEXT_CORE_PROFILE_BIT_ARB ) {
                return MEDIA_OPENGL_PROFILE_CORE;
            } else {
                return MEDIA_OPENGL_PROFILE_COMPATIBILITY;
            }
        } break;
        case MEDIA_OPENGL_ATTR_MAJOR:            return attr->major;
        case MEDIA_OPENGL_ATTR_MINOR:            return attr->minor;
        case MEDIA_OPENGL_ATTR_DOUBLE_BUFFER:
            return bitfield_check( attr->dwFlags, PFD_DOUBLEBUFFER );
        case MEDIA_OPENGL_ATTR_DEBUG:
            return bitfield_check( attr->context_flags, WGL_CONTEXT_DEBUG_BIT_ARB );
        case MEDIA_OPENGL_ATTR_FORWARD_COMPATIBILITY:
            return bitfield_check( attr->context_flags, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB );
        default: return I32_MAX;
    }
}

attr_media_api OpenGLRenderContext* media_render_gl_context_create(
    MediaSurface* in_surface, MediaOpenGLAttributes* opt_attr
) {
    surface_to_win32( in_surface );

    struct Win32OpenGLAttributes  _attr = {};
    struct Win32OpenGLAttributes* attr = &_attr;
    if( opt_attr ) {
        attr = (struct Win32OpenGLAttributes*)opt_attr;
    } else {
        win32_gl_set_default_attr( &_attr );
    }

    wglMakeCurrent( NULL, NULL );

    PIXELFORMATDESCRIPTOR dpf = {};
    dpf.nSize        = sizeof( dpf );
    dpf.iPixelType   = PFD_TYPE_RGBA;
    dpf.nVersion     = 1;
    dpf.dwFlags      = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | attr->dwFlags;
    dpf.cColorBits   = attr->red + attr->green + attr->blue + attr->alpha;

    dpf.cRedBits     = attr->red;
    dpf.cRedShift    = 0;

    dpf.cGreenBits   = attr->green;
    dpf.cGreenShift  = attr->red;

    dpf.cBlueBits    = attr->blue;
    dpf.cBlueShift   = attr->red + attr->green;

    dpf.cAlphaBits   = attr->alpha;
    dpf.cAlphaShift  = attr->red + attr->green + attr->blue;

    dpf.cDepthBits   = attr->depth;
    dpf.cStencilBits = attr->stencil;
    dpf.iLayerType   = PFD_MAIN_PLANE;

    i32 pf_index = ChoosePixelFormat( surface->hdc, &dpf );

    PIXELFORMATDESCRIPTOR spf = {};
    if( !DescribePixelFormat( surface->hdc, pf_index, sizeof(spf), &spf ) ) {
        win32_error( "failed to get pixel format!" );
        return false;
    }

    if( !SetPixelFormat( surface->hdc, pf_index, &spf ) ) {
        win32_error( "failed to set pixel format!" );
        return false;
    }

    HGLRC temp = wglCreateContext( surface->hdc );
    if( !temp ) {
        win32_error( "failed to create opengl temporary context!" );
        return false;
    }

    if( !wglMakeCurrent( surface->hdc, temp ) ) {
        win32_error( "failed to make dummy opengl context current!" );
        return false;
    }

    if( !wglCreateContextAttribsARB ) {
        #define wgl_load( name ) do {\
            name = (name##FN*)wglGetProcAddress( #name );\
            if( !name ) {\
                win32_error( "failed to load " #name " from wglGetProcAddress!" );\
                wglDeleteContext( temp );\
                return false;\
            }\
        } while(0)

        wgl_load( wglCreateContextAttribsARB );
        wgl_load( wglSwapIntervalEXT );

        #undef wgl_load
    }

    HGLRC rc    = wglCreateContextAttribsARB( surface->hdc, NULL, attr->attribs );
    DWORD error = GetLastError();
    wglDeleteContext( temp );

    if( !rc ) {
        switch( error ) {
            case ERROR_INVALID_VERSION_ARB: {
                media_error( "win32: failed to create opengl context because of invalid version!" );
            } break;
            case ERROR_INVALID_PROFILE_ARB: {
                media_error( "win32: failed to create opengl context because of invalid profile!" );
            } break;
            default: {
                media_error( "win32: [{u,X,f}] failed to create opengl context!", error );
            } break;
        }
        return NULL;
    }

    return (OpenGLRenderContext*)rc;
}
attr_media_api b32 media_render_gl_context_bind(
    MediaSurface* in_surface, OpenGLRenderContext* glrc
) {
    if( !in_surface ) {
        return wglMakeCurrent( NULL, NULL ) == TRUE;
    } else {
        surface_to_win32( in_surface );
        return wglMakeCurrent( surface->hdc, glrc ) == TRUE;
    }
}
attr_media_api void media_render_gl_context_destroy( OpenGLRenderContext* glrc ) {
    wglDeleteContext( glrc );
}
attr_media_api b32 media_render_gl_share_display_lists(
    OpenGLRenderContext* a, OpenGLRenderContext* b
) {
    return wglShareLists( a, b ) == TRUE;
}
attr_media_api void* media_render_gl_load_proc( const char* function_name ) {
    void* res = (void*)wglGetProcAddress( function_name );
    if( !res ) {
        HMODULE OPENGL32 = GetModuleHandleA( "OPENGL32.DLL" );
        res = (void*)GetProcAddress( OPENGL32, function_name );
    }

    return res;
}
attr_media_api b32 media_render_gl_swap_buffers( MediaSurface* in_surface ) {
    surface_to_win32( in_surface );
    return SwapBuffers( surface->hdc ) == TRUE;
}
attr_media_api b32 media_render_gl_swap_interval(
    MediaSurface* surface, int interval
) {
    unused(surface);
    return wglSwapIntervalEXT( interval ) == TRUE;
}

#endif /* Platform Windows */

