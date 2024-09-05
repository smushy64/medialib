/**
 * @file   opengl.c
 * @brief  Windows OpenGL.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 23, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/opengl.h"
#include "impl/win32/common.h"
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
    fn##FN* in_##fn = NULL

// NOTE(alicia): GDI32

def( int, DescribePixelFormat,
    HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd );
#define DescribePixelFormat in_DescribePixelFormat

def( int, ChoosePixelFormat, HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd );
#define ChoosePixelFormat in_ChoosePixelFormat

def( BOOL, SetPixelFormat, HDC hdc, int format, const PIXELFORMATDESCRIPTOR* ppfd );
#define SetPixelFormat in_SetPixelFormat

def( BOOL, SwapBuffers, HDC unnamedParam1 );
#define SwapBuffers in_SwapBuffers

// NOTE(alicia): OPENGL32

def( HGLRC, wglCreateContext, HDC hdc );
#define wglCreateContext in_wglCreateContext

def( BOOL, wglDeleteContext, HGLRC hglrc );
#define wglDeleteContext in_wglDeleteContext

def( BOOL, wglMakeCurrent, HDC hdc, HGLRC hglrc );
#define wglMakeCurrent in_wglMakeCurrent

def( PROC, wglGetProcAddress, LPCSTR name );
#define wglGetProcAddress in_wglGetProcAddress

def( BOOL, wglShareLists, HGLRC rc1, HGLRC rc2 );
#define wglShareLists in_wglShareLists

def( BOOL, wglCopyContext, HGLRC dst, HGLRC src, UINT uint );
#define wglCopyContext in_wglCopyContext

def( HGLRC, wglCreateContextAttribsARB, HDC, HGLRC, const int* );
#define wglCreateContextAttribsARB in_wglCreateContextAttribsARB

def( BOOL, wglSwapIntervalEXT, int );
#define wglSwapIntervalEXT in_wglSwapIntervalEXT

attr_media_api m_bool32 opengl_initialize(void) {
    #define load( lib, fn ) do {\
        fn = (fn##FN*)GetProcAddress( global_win32_state->modules.lib, #fn );\
        if( !fn ) {\
            win32_error( "opengl_initialize: failed to load " #fn " from " #lib "!");\
            return false;\
        }\
    } while(0)

    global_win32_state->modules.OPENGL32 = LoadLibraryA( "OPENGL32.DLL" );
    if( !global_win32_state->modules.OPENGL32 ) {
        win32_error( "opengl_initialize: failed to open library OPENGL32.DLL!" );
        return false;
    }

    load( GDI32, DescribePixelFormat );
    load( GDI32, ChoosePixelFormat );
    load( GDI32, SetPixelFormat );
    load( GDI32, SwapBuffers );

    load( OPENGL32, wglCreateContext );
    load( OPENGL32, wglDeleteContext );
    load( OPENGL32, wglMakeCurrent );
    load( OPENGL32, wglGetProcAddress );
    load( OPENGL32, wglShareLists );
    load( OPENGL32, wglCopyContext );

    #undef load
    return true;
}

struct Win32OpenGLAttributes win32_opengl_default_attrib(void) {
    struct Win32OpenGLAttributes attrib;
    attrib.dwFlags         = PFD_DOUBLEBUFFER;
    attrib.red             = 8;
    attrib.green           = 8;
    attrib.blue            = 8;
    attrib.alpha           = 8;
    attrib.depth           = 24;
    attrib.stencil         = 0;
    attrib.__profile_mask  = WGL_CONTEXT_PROFILE_MASK_ARB;
    attrib.profile         = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
    attrib.__major_mask    = WGL_CONTEXT_MAJOR_VERSION_ARB;
    attrib.major           = 3;
    attrib.__minor_mask    = WGL_CONTEXT_MINOR_VERSION_ARB;
    attrib.minor           = 3;
    attrib.__context_mask  = WGL_CONTEXT_FLAGS_ARB;
    attrib.context_flags   = 0;
    attrib.null_terminator = 0;

    return attrib;
}
attr_media_api OpenGLAttributeList opengl_attr_create(void) {
    struct Win32OpenGLAttributes attrib = win32_opengl_default_attrib();
    return *(OpenGLAttributeList*)&attrib;
}
attr_media_api m_bool32 opengl_attr_set(
    OpenGLAttributeList* attr, OpenGLAttribute name, int value 
) {
    struct Win32OpenGLAttributes* attrib = (struct Win32OpenGLAttributes*)attr;
    switch( name ) {
        case OPENGL_ATTR_RED_SIZE: {
            attrib->red = value;
        } break;
        case OPENGL_ATTR_GREEN_SIZE: {
            attrib->green = value;
        } break;
        case OPENGL_ATTR_BLUE_SIZE: {
            attrib->blue = value;
        } break;
        case OPENGL_ATTR_ALPHA_SIZE: {
            attrib->alpha = value;
        } break;
        case OPENGL_ATTR_DEPTH_SIZE: {
            attrib->depth = value;
        } break;
        case OPENGL_ATTR_STENCIL_SIZE: {
            attrib->stencil = value;
        } break;
        case OPENGL_ATTR_PROFILE: {
            switch( (OpenGLProfile)value ) {
                case OPENGL_PROFILE_CORE: {
                    attrib->profile = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
                } break;
                case OPENGL_PROFILE_COMPATIBILITY: {
                    attrib->profile = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
                } break;
                default: {
                    win32_error(
                        "opengl_attr_set: invalid value for OPENGL_ATTR_PROFILE!");
                } return false;
            }
        } break;
        case OPENGL_ATTR_MAJOR: {
            attrib->major = value;
        } break;
        case OPENGL_ATTR_MINOR: {
            attrib->minor = value;
        } break;
        case OPENGL_ATTR_DOUBLE_BUFFER: {
            attrib->dwFlags = value ? PFD_DOUBLEBUFFER : 0;
        } break;
        case OPENGL_ATTR_DEBUG: {
            if( value ) {
                attrib->context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
            } else {
                attrib->context_flags &= ~WGL_CONTEXT_DEBUG_BIT_ARB;
            }
        } break;
        case OPENGL_ATTR_FORWARD_COMPATIBILITY: {
            if( value ) {
                attrib->context_flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            } else {
                attrib->context_flags &= ~WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            }
        } break;
    }

    return true;
}
attr_media_api m_int32 opengl_attr_get(
    OpenGLAttributeList* attr, OpenGLAttribute name 
) {
    struct Win32OpenGLAttributes* attrib = (struct Win32OpenGLAttributes*)attr;
    switch( name ) {
        case OPENGL_ATTR_RED_SIZE              : return attrib->red;
        case OPENGL_ATTR_GREEN_SIZE            : return attrib->green;
        case OPENGL_ATTR_BLUE_SIZE             : return attrib->blue;
        case OPENGL_ATTR_ALPHA_SIZE            : return attrib->alpha;
        case OPENGL_ATTR_DEPTH_SIZE            : return attrib->depth;
        case OPENGL_ATTR_STENCIL_SIZE          : return attrib->stencil;
        case OPENGL_ATTR_PROFILE               : {
            if( attrib->profile == WGL_CONTEXT_CORE_PROFILE_BIT_ARB ) {
                return OPENGL_PROFILE_CORE;
            } else {
                return OPENGL_PROFILE_COMPATIBILITY;
            }
        } break;
        case OPENGL_ATTR_MAJOR                 : return attrib->major;
        case OPENGL_ATTR_MINOR                 : return attrib->minor;
        case OPENGL_ATTR_DOUBLE_BUFFER         :
            return (attrib->dwFlags & PFD_DOUBLEBUFFER) != 0;
        case OPENGL_ATTR_DEBUG                 :
            return (attrib->context_flags & WGL_CONTEXT_DEBUG_BIT_ARB) != 0;
        case OPENGL_ATTR_FORWARD_COMPATIBILITY :
            return (attrib->context_flags & WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) != 0;
    }
    return -1;
}

attr_media_api OpenGLRenderContext* opengl_context_create(
    SurfaceHandle* in_surface, OpenGLAttributeList* opt_attributes 
) {
    struct Win32Surface* surface = in_surface;

    struct Win32OpenGLAttributes attr;
    struct Win32OpenGLAttributes* attrib = &attr;
    if( opt_attributes ) {
        attrib  = (struct Win32OpenGLAttributes*)opt_attributes;
    } else {
        *attrib = win32_opengl_default_attrib();
    }

    wglMakeCurrent( 0, 0 );

    PIXELFORMATDESCRIPTOR desired_pfd;
    memset( &desired_pfd, 0, sizeof(desired_pfd) );

    desired_pfd.nSize      = sizeof( desired_pfd );
    desired_pfd.iPixelType = PFD_TYPE_RGBA;
    desired_pfd.nVersion   = 1;
    desired_pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | attrib->dwFlags;
    desired_pfd.cColorBits = attrib->red + attrib->green + attrib->blue + attrib->alpha;

    desired_pfd.cRedBits  = attrib->red;
    desired_pfd.cRedShift = 0;

    desired_pfd.cGreenBits  = attrib->green;
    desired_pfd.cGreenShift = attrib->red;

    desired_pfd.cBlueBits  = attrib->blue;
    desired_pfd.cBlueShift = attrib->red + attrib->green;

    desired_pfd.cAlphaBits  = attrib->alpha;
    desired_pfd.cAlphaShift = attrib->red + attrib->green + attrib->blue;

    desired_pfd.cDepthBits   = attrib->depth;
    desired_pfd.cStencilBits = attrib->stencil;
    desired_pfd.iLayerType   = PFD_MAIN_PLANE;

    int pfd_index = ChoosePixelFormat( surface->hdc, &desired_pfd );

    PIXELFORMATDESCRIPTOR pfd;
    memset( &pfd, 0, sizeof(pfd) );
    if( !DescribePixelFormat( surface->hdc, pfd_index, sizeof(pfd), &pfd ) ) {
        win32_error( "failed to get pixel format!" );
        return NULL;
    }

    if( !SetPixelFormat( surface->hdc, pfd_index, &pfd ) ) {
        win32_error( "failed to set pixel format!" );
        return NULL;
    }

    HGLRC temp = wglCreateContext( surface->hdc );
    if( !temp ) {
        win32_error( "failed to create temporary OpenGL context!" );
        return NULL;
    }

    if( !wglMakeCurrent( surface->hdc, temp ) ) {
        wglDeleteContext( temp );
        win32_error( "failed to make dummy opengl context current!" );
        return NULL;
    }

    if( !wglCreateContextAttribsARB ) {
        #define load( name ) do {\
            name = (name##FN*)wglGetProcAddress( #name );\
            if( !name ) {\
                wglMakeCurrent( 0, 0 );\
                wglDeleteContext( temp );\
                win32_error( "failed to load " #name " from wglGetProcAddress!" );\
                return NULL;\
            }\
        } while(0)

        load( wglCreateContextAttribsARB );
        load( wglSwapIntervalEXT );

        #undef load
    } 

    HGLRC rc  = wglCreateContextAttribsARB( surface->hdc, NULL, attrib->attribs );
    DWORD err = GetLastError();

    wglDeleteContext( temp );

    if( !rc ) {
        switch( err ) {
            case ERROR_INVALID_VERSION_ARB: {
                win32_error(
                    "failed to create opengl context because of invalid version!");
            } break;
            case ERROR_INVALID_PROFILE_ARB: {
                win32_error(
                    "failed to create opengl context because of invalid profile!" );
            } break;
            default: {
                win32_error(
                    "failed to create OpenGL context for unknown reason!" );
            } break;
        }
        return NULL;
    }

    return rc;
}
attr_media_api m_bool32 opengl_context_bind(
    SurfaceHandle* in_surface, OpenGLRenderContext* glrc 
) {
    if( !in_surface || !glrc ) {
        return wglMakeCurrent( 0, 0 ) != FALSE;
    }
    struct Win32Surface* surface = in_surface;
    return wglMakeCurrent( surface->hdc, glrc ) != FALSE;
}
attr_media_api void opengl_context_destroy( OpenGLRenderContext* glrc ) {
    wglDeleteContext( glrc );
}
attr_media_api m_bool32 opengl_context_share(
    OpenGLRenderContext* a, OpenGLRenderContext* b 
) {
    return wglShareLists( a, b ) != FALSE;
}
attr_media_api void* opengl_load_proc( const char* function_name ) {
    void* res = (void*)wglGetProcAddress( function_name );
    if( !res ) {
        res = (void*)GetProcAddress(
            global_win32_state->modules.OPENGL32, function_name );
    }
    return res;
}
attr_media_api m_bool32 opengl_swap_buffers( SurfaceHandle* in_surface ) {
    struct Win32Surface* surface = in_surface;
    return SwapBuffers( surface->hdc ) != FALSE;
}
attr_media_api m_bool32 opengl_swap_interval(
    SurfaceHandle* in_surface, int interval 
) {
    unused( in_surface );
    return wglSwapIntervalEXT( interval ) != FALSE;
}

#undef def
#endif

