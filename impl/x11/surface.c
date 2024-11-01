/**
 * @file   surface.c
 * @brief  Surface implementation for X11.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 21, 2024
*/
#include "media/defines.h"

#if defined(MEDIA_PLATFORM_LINUX)
#include "media/surface.h"
#include "media/internal/logging.h"
#include "media/input.h"
#include "impl/x11/surface.h"
#include "impl/x11/input.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/XKBlib.h>
#include <GL/glx.h>
#include <string.h>
#include <stdio.h>

#define MOUSE_BUTTON_BACK    8
#define MOUSE_BUTTON_FORWARD 9

extern int global_x11_gl_visual[];

attr_media_api uintptr_t surface_query_memory_requirement(void) {
    return sizeof(struct X11Surface);
}
attr_media_api _Bool surface_create(
    uint32_t title_len, const char* title, int32_t x, int32_t y, int32_t w, int32_t h,
    SurfaceCreateFlags flags, SurfaceCallbackFN* opt_callback,
    void* opt_callback_params, SurfaceHandle* opt_parent, SurfaceHandle* out_surface
) {
    // TODO(alicia): parent!
    unused(opt_parent);

    struct X11Surface* surf = out_surface;
    char buf[255] = {};
    if( title_len >= sizeof(buf) ) {
        memcpy( buf, title, sizeof(buf) - 1 );
    } else {
        memcpy( buf, title, title_len );
    }

    surf->display = XOpenDisplay( NULL );
    if( !surf->display ) {
        media_error( "X11: failed to open display!" );
        return false;
    }
    surf->WM_DELETE_WINDOW = XInternAtom( surf->display, "WM_DELETE_WINDOW", False );

    surf->x = x;
    surf->y = y;
    surf->w = w ? w : 800;
    surf->h = h ? h : 600;

    int depth      = CopyFromParent;
    int class      = CopyFromParent;
    Visual* visual = CopyFromParent;
    XSetWindowAttributes attr = {};

    if( flags & SURFACE_CREATE_FLAG_OPENGL ) {
        int fb_count = 0;
        GLXFBConfig* fbc = glXChooseFBConfig(
            surf->display, DefaultScreen(surf->display),
            global_x11_gl_visual, &fb_count );

        if( !fbc ) {
            media_error( "X11: GL: Failed to get framebuffer config!" );
            XCloseDisplay( surf->display );
            return false;
        }
        
        // TODO(alicia): find best config
        GLXFBConfig config = fbc[0];
        surf->fb_config    = config;

        XVisualInfo* vi = glXGetVisualFromFBConfig( surf->display, config );

        visual = vi->visual;
        attr.colormap = XCreateColormap( 
            surf->display, XDefaultRootWindow( surf->display ),
            visual, AllocNone );

        XFree( vi );
        XFree( fbc );
    }

    int attr_value_mask = CWColormap | CWBackPixel | CWEventMask;
    attr.background_pixel = 0;
    attr.event_mask =
        StructureNotifyMask | // surface closed
        PointerMotionMask   | // mouse moved
        ButtonPressMask     | // mouse button press
        ButtonReleaseMask   | // mouse button release
        KeyPressMask        | // key press
        KeyReleaseMask      | // key release
        FocusChangeMask     | // when surface is focused
        ExposureMask        ; // when surface is focused?

    surf->window = XCreateWindow(
        surf->display, XDefaultRootWindow( surf->display ),
        x, y, surf->w, surf->h, 0, depth, class,
        visual, attr_value_mask, &attr );

    XStoreName( surf->display, surf->window, buf );
    XSetWMProtocols( surf->display, surf->window, &surf->WM_DELETE_WINDOW, 1 );

    if( flags & SURFACE_CREATE_FLAG_HIDDEN ) {
        surf->flags |= SURFACE_STATE_IS_HIDDEN;
    } else {
        XMapWindow( surf->display, surf->window );
    }
    XFlush( surf->display );

    surf->callback        = opt_callback;
    surf->callback_params = opt_callback_params;

    int supported = 0;
    XkbSetDetectableAutoRepeat( surf->display, True, &supported );
    if( !supported ) {
        media_warn( "X11: disabling key repeat events is not supported!" );
    }

    if( flags & SURFACE_CREATE_FLAG_FULLSCREEN ) {
        surface_set_fullscreen( surf, true );
    }

    return true;
}
attr_media_api void surface_destroy( SurfaceHandle* surface ) {
    struct X11Surface* surf = surface;
    XCloseDisplay( surf->display );
    memset( surf, 0, sizeof(*surf) );
}
attr_media_api void surface_pump_events( SurfaceHandle* surface ) {
    struct X11Surface* surf = surface;

    while( XPending( surf->display ) ) {
        XEvent event = {};
        XNextEvent( surf->display, &event );

        if( !surf->callback ) {
            continue;
        }

        SurfaceCallbackData data;
        memset( &data, 0, sizeof(data) );

        #define cb()\
            surf->callback( surface, &data, surf->callback_params )

        switch( event.type ) {
            case ClientMessage: {
                XClientMessageEvent* val = (void*)&event;
                if( (Atom)val->data.l[0] == surf->WM_DELETE_WINDOW ) {

                    data.type = SURFACE_CALLBACK_TYPE_CLOSE;
                    cb();
                }
            } break;
            case FocusOut:
            case FocusIn: {
                XFocusChangeEvent* val = (void*)&event;

                data.type = SURFACE_CALLBACK_TYPE_FOCUS;
                data.focus.gained = val->type == FocusIn;

                if( data.focus.gained ) {
                    surf->flags |= SURFACE_STATE_IS_FOCUSED;
                } else {
                    surf->flags &= ~SURFACE_STATE_IS_FOCUSED;
                }

                cb();
            } break;
            case ConfigureNotify: {
                XConfigureEvent* val = (void*)&event;

                if(
                    val->x != surf->x ||
                    val->y != surf->y
                ) {
                    data.type = SURFACE_CALLBACK_TYPE_POSITION;
                    data.position.old_x = surf->x;
                    data.position.old_y = surf->y;
                    data.position.x     = val->x;
                    data.position.y     = val->y;

                    cb();

                }
                surf->x = val->x;
                surf->y = val->y;

                if(
                    val->width  != surf->w ||
                    val->height != surf->h
                ) {
                    memset( &data, 0, sizeof(data) );

                    data.type = SURFACE_CALLBACK_TYPE_RESIZE;
                    data.resize.old_w = surf->w;
                    data.resize.old_h = surf->h;
                    data.resize.w     = val->width;
                    data.resize.h     = val->height;

                    cb();

                    memset( &data, 0, sizeof(data) );
                }
                surf->w = val->width;
                surf->h = val->height;
            } break;
            case KeyPress: {
                XKeyEvent* val = (void*)&event;

                // NOTE(alicia): handle text input

                data.type = SURFACE_CALLBACK_TYPE_TEXT;
                KeySym _unused_;
                XLookupString( val, data.text.utf8, 15, &_unused_, NULL );

                cb();
                memset( &data, 0, sizeof(data) );

            }
            case KeyRelease: {
                XKeyEvent* val = (void*)&event;
                KeySym sym = XLookupKeysym( val, 0 );
                if( sym == NoSymbol ) {
                    break;
                }
                KeyboardCode kc  = x11_keysym_to_key( sym );
                bool is_down     = event.type == KeyPress;

                if( is_down ) {
                    if( keyboard_state_get_key( &surf->keys, kc ) ) {
                        break;
                    }
                }

                keyboard_state_set_key( &surf->keys, kc, is_down );

                data.type        = SURFACE_CALLBACK_TYPE_KEY;
                data.key.code    = kc;
                data.key.is_down = event.type == KeyPress;

                if( val->state & ShiftMask ) {
                    data.key.mod |= KBMOD_SHIFT;
                }
                if( val->state & ControlMask ) {
                    data.key.mod |= KBMOD_CTRL;
                }
                if( val->state & LockMask ) {
                    data.key.mod |= KBMOD_CAPSLK;
                }
                if( val->state & Mod1Mask ) {
                    data.key.mod |= KBMOD_ALT;
                }
                if( val->state & Mod2Mask ) {
                    data.key.mod |= KBMOD_NUMLK;
                }
                // TODO(alicia): scroll lock modifier? not really that useful tho

                /*
                if( val->state & Mod3Mask ) {
                    data.key.mod |= KBMOD_SCRLK;
                }
                */

                cb();
            } break;
            case ButtonRelease:
            case ButtonPress:   {
                XButtonEvent* val = (void*)&event;

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_BUTTON;

                switch( val->button ) {
                    case Button1: {
                        data.mouse_button.delta = MB_LEFT;
                    } break;
                    case Button2: {
                        data.mouse_button.delta = MB_MIDDLE;
                    } break;
                    case Button3: {
                        data.mouse_button.delta = MB_RIGHT;
                    } break;
                    case 8: {
                        data.mouse_button.delta = MB_EXTRA_1;
                    } break;
                    case 9: {
                        data.mouse_button.delta = MB_EXTRA_2;
                    } break;
                }

                bool is_down = val->type == ButtonPress;

                if( is_down ) {
                    surf->mb_state |= data.mouse_button.delta;
                } else {
                    surf->mb_state &= ~data.mouse_button.delta;
                }

                data.mouse_button.state = surf->mb_state;

                cb();

                int wx = 0;
                int wy = 0;

                switch( val->button ) {
                    case 4: {
                        wy = 1;
                    } break; // scroll up
                    case 5: {
                        wy = -1;
                    } break; // scroll down
                    case 6: {
                        wx = -1;
                    } break; // scroll left
                    case 7: {
                        wx = 1;
                    } break; // scroll right 
                }

                if( !(wx || wy) ) {
                    break;
                }

                memset( &data, 0, sizeof(data) );

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_WHEEL;

                if( wx ) {
                    data.mouse_wheel.delta         = wx;
                    data.mouse_wheel.is_horizontal = true;

                    cb();
                }

                if( wy ) {
                    data.mouse_wheel.delta         = wy;
                    data.mouse_wheel.is_horizontal = false;

                    cb();
                }
            } break;
            case MotionNotify: {
                XMotionEvent* val = (void*)&event;

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE;
                data.mouse_move.x = val->x;
                data.mouse_move.y = val->y;

                cb();

                data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA;

                data.mouse_move_delta.x = val->x   - surf->mx;
                data.mouse_move_delta.y = surf->my - val->y;

                cb();

                surf->mx = val->x;
                surf->my = val->y;
            } break;
        }

        #undef cb
    }
}
attr_media_api void surface_set_callback(
    SurfaceHandle* surface, SurfaceCallbackFN* callback, void* opt_callback_params
) {
    struct X11Surface* surf = surface;
    surf->callback          = callback;
    surf->callback_params   = opt_callback_params;
}
attr_media_api void surface_clear_callback( SurfaceHandle* surface ) {
    struct X11Surface* surf = surface;
    surf->callback          = 0;
    surf->callback_params   = 0;
}
attr_media_api void* surface_get_platform_handle( SurfaceHandle* surface ) {
    return surface;
}
attr_media_api const char* surface_query_title(
    const SurfaceHandle* surface, uint32_t* opt_out_len
) {
    const struct X11Surface* surf = surface;
    XTextProperty prop;
    XGetWMName( surf->display, surf->window, &prop );

    if( opt_out_len ) {
        *opt_out_len = prop.nitems;
    }
    return (const char*)prop.value;
}
attr_media_api void surface_set_title(
    SurfaceHandle* surface, uint32_t len, const char* title
) {
    struct X11Surface* surf = surface;

    char buf[255] = {};
    if( len >= sizeof(buf) ) {
        memcpy( buf, title, sizeof(buf) - 1 );
    } else {
        memcpy( buf, title, len );
    }

    XStoreName( surf->display, surf->window, buf );
}
attr_media_api void surface_query_position(
    const SurfaceHandle* surface, int32_t* out_x, int32_t* out_y
) {
    const struct X11Surface* surf = surface;

    *out_x = surf->x;
    *out_y = surf->y;
}
attr_media_api void surface_set_position(
    SurfaceHandle* surface, int32_t x, int32_t y
) {
    struct X11Surface* surf = surface;
    unused(surf,x,y);

    // TODO(alicia): this doesn't work for whatever reason.

    /* XMoveWindow( surf->display, surf->window, x, y ); */
}
attr_media_api void surface_query_dimensions(
    const SurfaceHandle* surface, int32_t* out_w, int32_t* out_h
) {
    const struct X11Surface* surf = surface;

    *out_w = surf->w;
    *out_h = surf->h;
}
attr_media_api void surface_set_dimensions(
    SurfaceHandle* surface, int32_t w, int32_t h 
) {
    struct X11Surface* surf = surface;
    XResizeWindow( surf->display, surf->window, w, h );
}
attr_media_api SurfaceStateFlags surface_query_state( const SurfaceHandle* surface ) {
    const struct X11Surface* surf = surface;
    return surf->flags;
}
attr_media_api void surface_set_fullscreen(
    SurfaceHandle* surface, _Bool is_fullscreen
) {
    struct X11Surface* surf = surface;

    if( ((surf->flags & SURFACE_STATE_FULLSCREEN) != 0) == is_fullscreen ) {
        return;
    }

    Atom WM_STATE            = XInternAtom( surf->display, "_NET_WM_STATE", False );
    Atom WM_STATE_FULLSCREEN = XInternAtom(
        surf->display, "_NET_WM_STATE_FULLSCREEN", False );

    XEvent ev = {0};

    ev.type                 = ClientMessage;
    ev.xclient.window       = surf->window;
    ev.xclient.message_type = WM_STATE;
    ev.xclient.format       = 32;
    ev.xclient.data.l[0]    = is_fullscreen ? 1 : 0;
    ev.xclient.data.l[1]    = WM_STATE_FULLSCREEN;
    ev.xclient.data.l[2]    = 0;

    XSendEvent(
        surf->display, DefaultRootWindow(surf->display),
        False, SubstructureNotifyMask, &ev );

    if( is_fullscreen ) {
        surf->flags |= SURFACE_STATE_FULLSCREEN;
    } else {
        surf->flags &= ~SURFACE_STATE_FULLSCREEN;
    }
}
attr_media_api void surface_set_hidden( SurfaceHandle* surface, _Bool is_hidden ) {
    struct X11Surface* surf = surface;

    if( ((surf->flags & SURFACE_STATE_IS_HIDDEN) != 0) == is_hidden ) {
        return;
    }

    if( is_hidden ) {
        surf->flags |= SURFACE_STATE_IS_HIDDEN;
        XUnmapWindow( surf->display, surf->window );
    } else {
        surf->flags &= ~SURFACE_STATE_IS_HIDDEN;
        XMapWindow( surf->display, surf->window );
    }
}

#undef MOUSE_BUTTON_BACK   
#undef MOUSE_BUTTON_FORWARD

#endif /* Platform Linux */

