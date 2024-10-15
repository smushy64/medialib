/**
 * @file   surface.c
 * @brief  Posix surface implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 14, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_POSIX)
#include "media/surface.h"
#include "media/input/keyboard.h"
#include "media/input/mouse.h"
#include "impl/sdl/surface.h"
#include "media/internal/logging.h"
#include <SDL3/SDL.h>
#include <float.h>

attr_media_api uintptr_t surface_query_memory_requirement(void) {
    return sizeof( struct SDLSurface );
}

bool SDLCALL sdl_event_filter( void* userparams, SDL_Event* event ) {
    // NOTE(alicia): this is an evil function designed to pass
    // struct SDLSurface pointer to pump events.
    // Do not try at home, kids!

    struct SDLSurface* surf = userparams;

    switch( event->type ) {
        case SDL_EVENT_WINDOW_SHOWN:             
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MOVED:            
        case SDL_EVENT_WINDOW_RESIZED:          
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
        case SDL_EVENT_WINDOW_MINIMIZED:        
        case SDL_EVENT_WINDOW_MAXIMIZED:         
        case SDL_EVENT_WINDOW_RESTORED:           
        case SDL_EVENT_WINDOW_MOUSE_ENTER:       
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:      
        case SDL_EVENT_WINDOW_FOCUS_GAINED:     
        case SDL_EVENT_WINDOW_FOCUS_LOST:       
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:  
        case SDL_EVENT_WINDOW_HIT_TEST:         
        case SDL_EVENT_WINDOW_ICCPROF_CHANGED:  
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:  
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
        case SDL_EVENT_WINDOW_OCCLUDED:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN: 
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN: 
        case SDL_EVENT_WINDOW_DESTROYED:        
        case SDL_EVENT_WINDOW_HDR_STATE_CHANGED: {
            SDL_Window* win = SDL_GetWindowFromID( event->window.windowID );
            if( win ) {
                if( surf->handle == win ) {
                    event->window.timestamp = (Uint64)surf;
                }
            } else {
                event->window.timestamp = 0;
            }
        } break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            SDL_Window* win = SDL_GetWindowFromID( event->window.windowID );
            if( win ) {
                if( surf->handle == win ) {
                    event->key.timestamp = (Uint64)surf;
                }
            } else {
                event->key.timestamp = 0;
            }
        } break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Window* win = SDL_GetWindowFromID( event->window.windowID );
            if( win ) {
                if( surf->handle == win ) {
                    event->button.timestamp = (Uint64)surf;
                }
            } else {
                event->button.timestamp = 0;
            }
        } break;
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Window* win = SDL_GetWindowFromID( event->window.windowID );
            if( win ) {
                if( surf->handle == win ) {
                    event->motion.timestamp = (Uint64)surf;
                }
            } else {
                event->motion.timestamp = 0;
            }
        } break;
        case SDL_EVENT_MOUSE_WHEEL: {
            SDL_Window* win = SDL_GetWindowFromID( event->window.windowID );
            if( win ) {
                if( surf->handle == win ) {
                    event->wheel.timestamp = (Uint64)surf;
                }
            } else {
                event->wheel.timestamp = 0;
            }
        } break;
        case SDL_EVENT_TEXT_INPUT: {
            SDL_Window* win = SDL_GetWindowFromID( event->window.windowID );
            if( win ) {
                if( surf->handle == win ) {
                    event->text.timestamp = (Uint64)surf;
                }
            } else {
                event->text.timestamp = 0;
            }
        } break;
        default: break;
    }
    return true;
}

attr_media_api _Bool surface_create(
    uint32_t title_len, const char* title, int32_t x, int32_t y, int32_t w, int32_t h,
    SurfaceCreateFlags flags, SurfaceCallbackFN* opt_callback,
    void* opt_callback_params, SurfaceHandle* opt_parent, SurfaceHandle* out_surface
) {
    struct SDLSurface* surface = out_surface;
    // TODO(alicia): set parent!
    unused(opt_parent);

    char title_buf[SURFACE_MAX_TITLE_LEN];
    SDL_memset( title_buf, 0, sizeof(title_buf) );
    uint32_t max_copy = title_len;
    if( max_copy >= SURFACE_MAX_TITLE_LEN ) {
        max_copy = SURFACE_MAX_TITLE_LEN - 1;
    }
    SDL_memcpy( title_buf, title, max_copy );

    SDL_WindowFlags sdl_flags = 0;
    if( flags & SURFACE_CREATE_FLAG_HIDDEN ) {
        sdl_flags |= SDL_WINDOW_HIDDEN;
    }
    if( flags & SURFACE_CREATE_FLAG_RESIZEABLE ) {
        sdl_flags |= SDL_WINDOW_RESIZABLE;
    }
    if( flags & SURFACE_CREATE_FLAG_FULLSCREEN ) {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }
    // TODO(alicia): validate flags!
    if( flags & SURFACE_CREATE_FLAG_OPENGL ) {
        sdl_flags |= SDL_WINDOW_OPENGL;
    }
    if( flags & SURFACE_CREATE_FLAG_VULKAN ) {
        sdl_flags |= SDL_WINDOW_VULKAN;
    }

    int _x = x;
    int _y = y;

    if( flags & SURFACE_CREATE_FLAG_X_CENTERED ) {
        _x = SDL_WINDOWPOS_CENTERED;
    }
    if( flags & SURFACE_CREATE_FLAG_Y_CENTERED ) {
        _y = SDL_WINDOWPOS_CENTERED;
    }

    SDL_Window* win = SDL_CreateWindow( title_buf, w, h, sdl_flags );
    if( !win ) {
        media_error( "sdl: failed to create window!" );
        return false;
    }

    surface->handle          = win;
    surface->callback        = opt_callback;
    surface->callback_params = opt_callback_params;
    surface->w = w;
    surface->h = h;
    if( !(flags & SURFACE_CREATE_FLAG_HIDDEN) ) {
        surface->is_focused = true;
    }

    SDL_SetEventFilter( sdl_event_filter, surface );
    SDL_StartTextInput( win );
    SDL_SetWindowPosition( win, _x, _y );
    SDL_GetWindowPosition( win, &surface->x, &surface->y );

    return true;
}
attr_media_api void surface_destroy( SurfaceHandle* surface ) {
    struct SDLSurface* surf = surface;
    SDL_StopTextInput( surf->handle );
    SDL_DestroyWindow( surf->handle );
    SDL_memset( surf, 0, sizeof(*surf) );
}
attr_media_api void surface_pump_events(void) {
    SDL_PumpEvents();
    SDL_Event event;
    SurfaceCallbackData data;
    while( SDL_PollEvent( &event ) ) {
        SDL_memset( &data, 0, sizeof(data) );
        switch( event.type ) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }
                if( surf->callback ) {
                    data.type = SURFACE_CALLBACK_TYPE_CLOSE;
                    surf->callback( surf, &data, surf->callback_params );
                }
            } break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }
                surf->is_focused = true;

                if( surf->callback ) {
                    data.type = SURFACE_CALLBACK_TYPE_FOCUS;
                    data.focus.gained = true;
                    surf->callback( surf, &data, surf->callback_params );
                }
            } break;
            case SDL_EVENT_WINDOW_FOCUS_LOST: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }
                surf->is_focused = false;

                if( surf->callback ) {
                    data.type = SURFACE_CALLBACK_TYPE_FOCUS;
                    data.focus.gained = false;
                    surf->callback( surf, &data, surf->callback_params );
                }
            } break;
            case SDL_EVENT_WINDOW_MOVED: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if(
                    surf->callback &&
                    (surf->x != event.window.data1 || surf->y != event.window.data2)
                ) {
                    data.type = SURFACE_CALLBACK_TYPE_POSITION;
                    data.position.old_x = surf->x;
                    data.position.old_y = surf->y;
                    data.position.x     = event.window.data1;
                    data.position.y     = event.window.data2;
                    surf->callback( surf, &data, surf->callback_params );
                }

                surf->x = event.window.data1;
                surf->y = event.window.data2;
            } break;
            case SDL_EVENT_WINDOW_RESIZED: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if(
                    surf->callback &&
                    (surf->w != event.window.data1 || surf->h != event.window.data2)
                ) {
                    data.type = SURFACE_CALLBACK_TYPE_RESIZE;
                    data.resize.old_w = surf->w;
                    data.resize.old_h = surf->h;
                    data.resize.w     = event.window.data1;
                    data.resize.h     = event.window.data2;
                    surf->callback( surf, &data, surf->callback_params );
                }

                surf->w = event.window.data1;
                surf->h = event.window.data2;
            } break;
            case SDL_EVENT_MOUSE_MOTION: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if( surf->callback ) {
                    data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE;
                    data.mouse_move.x = event.motion.x;
                    data.mouse_move.y = event.motion.y;
                    surf->callback( surf, &data, surf->callback_params );

                    data.type = SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA;
                    data.mouse_move_delta.x = event.motion.xrel < 0.0f ? -1 : 1;
                    data.mouse_move_delta.y = event.motion.yrel > 0.0f ? -1 : 1;
                    surf->callback( surf, &data, surf->callback_params );
                }
            } break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if( surf->callback ) {
                    data.type = SURFACE_CALLBACK_TYPE_MOUSE_BUTTON;
                    MouseButton btn_mask = 0;
                    switch( event.button.button ) {
                        case SDL_BUTTON_LEFT: {
                            btn_mask |= MB_LEFT;
                        } break;
                        case SDL_BUTTON_MIDDLE: {
                            btn_mask |= MB_MIDDLE;
                        } break;
                        case SDL_BUTTON_RIGHT: {
                            btn_mask |= MB_RIGHT;
                        } break;
                        case SDL_BUTTON_X1: {
                            btn_mask |= MB_EXTRA_1;
                        } break;
                        case SDL_BUTTON_X2: {
                            btn_mask |= MB_EXTRA_2;
                        } break;
                    }

                    if( event.button.down ) {
                        data.mouse_button.state = btn_mask;
                    } else {
                        data.mouse_button.state = 0;
                    }
                    data.mouse_button.delta = btn_mask;
                    surf->callback( surf, &data, surf->callback_params );
                }
            } break;
            case SDL_EVENT_MOUSE_WHEEL: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if( surf->callback ) {
                    if(
                        (event.wheel.y < 0.0f ? -event.wheel.y : event.wheel.y) >
                        FLT_EPSILON
                    ) {
                        data.type = SURFACE_CALLBACK_TYPE_MOUSE_WHEEL;
                        data.mouse_wheel.delta         = event.wheel.y < 0.0f ? -1 : 1;
                        data.mouse_wheel.is_horizontal = false;

                        surf->callback( surf, &data, surf->callback_params );
                    }

                    if(
                        (event.wheel.x < 0.0f ? -event.wheel.x : event.wheel.x) >
                        FLT_EPSILON
                    ) {
                        data.type = SURFACE_CALLBACK_TYPE_MOUSE_WHEEL;
                        data.mouse_wheel.delta         = event.wheel.x < 0.0f ? -1 : 1;
                        data.mouse_wheel.is_horizontal = true;

                        surf->callback( surf, &data, surf->callback_params );
                    }
                }
            } break;
            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_KEY_DOWN: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if( surf->callback ) {
                    data.type = SURFACE_CALLBACK_TYPE_KEY;
                    data.key.is_down = event.key.down;
                    data.key.mod     = sdl_to_mod( event.key.mod );
                    data.key.code    = sdl_to_key( event.key.key );

                    surf->callback( surf, &data, surf->callback_params );
                }
            } break;
            case SDL_EVENT_TEXT_INPUT: {
                struct SDLSurface* surf = (void*)event.window.timestamp;
                if( !surf ) {
                    continue;
                }

                if( surf->callback ) {
                    data.type     = SURFACE_CALLBACK_TYPE_TEXT;
                    uintptr_t len = SDL_strlen( event.text.text );
                    uintptr_t off = 0;

                    while( len ) {
                        uintptr_t max_copy = len;
                        if( len >= sizeof( data.text.utf8 ) ) {
                            len = sizeof( data.text.utf8 ) - 1;
                        }
                        SDL_memcpy( data.text.utf8, event.text.text + off, max_copy );
                        data.text.utf8[max_copy] = 0;

                        surf->callback( surf, &data, surf->callback_params );

                        off += max_copy;
                        len -= max_copy;
                    }

                }
            } break;
        }
    }
}
attr_media_api void surface_set_callback(
    SurfaceHandle* surface, SurfaceCallbackFN* callback, void* opt_callback_params
) {
    struct SDLSurface* surf = surface;
    surf->callback          = callback;
    surf->callback_params   = opt_callback_params;
}
attr_media_api void surface_clear_callback( SurfaceHandle* surface ) {
    struct SDLSurface* surf = surface;
    surf->callback        = NULL;
    surf->callback_params = NULL;
}
attr_media_api void* surface_get_platform_handle( SurfaceHandle* surface ) {
    struct SDLSurface* surf = surface;
    return surf->handle;
}
attr_media_api const char* surface_query_title(
    const SurfaceHandle* surface, uint32_t* opt_out_len
) {
    const struct SDLSurface* surf = surface;
    const char* title = SDL_GetWindowTitle( surf->handle );
    if( opt_out_len ) {
        *opt_out_len = SDL_strlen( title );
    }
    return title;
}
attr_media_api void surface_set_title(
    SurfaceHandle* surface, uint32_t len, const char* title
) {
    struct SDLSurface* surf = surface;
    char buf[SURFACE_MAX_TITLE_LEN];
    SDL_memset( buf, 0, sizeof(buf));

    uint32_t max_copy = len;
    if( max_copy >= SURFACE_MAX_TITLE_LEN ) {
        max_copy = SURFACE_MAX_TITLE_LEN - 1;
    }
    SDL_memcpy( buf, title, max_copy );

    SDL_SetWindowTitle( surf->handle, buf );
}
attr_media_api void surface_query_position(
    const SurfaceHandle* surface, int32_t* out_x, int32_t* out_y 
) {
    const struct SDLSurface* surf = surface;
    SDL_GetWindowPosition( surf->handle, out_x, out_y );
}
attr_media_api void surface_set_position(
    SurfaceHandle* surface, int32_t x, int32_t y
) {
    struct SDLSurface* surf = surface;
    SDL_SetWindowPosition( surf->handle, x, y );
}
attr_media_api void surface_query_dimensions(
    const SurfaceHandle* surface, int32_t* out_w, int32_t* out_h
) {
    const struct SDLSurface* surf = surface;
    SDL_GetWindowSize( surf->handle, out_w, out_h );
}
attr_media_api void surface_set_dimensions(
    SurfaceHandle* surface, int32_t w, int32_t h
) {
    struct SDLSurface* surf = surface;
    SDL_SetWindowSize( surf->handle, w, h );
}
attr_media_api SurfaceStateFlags surface_query_state( const SurfaceHandle* surface ) {
    const struct SDLSurface* surf = surface;
    SDL_WindowFlags sdl_flags = SDL_GetWindowFlags( surf->handle );
    SurfaceStateFlags flags = 0;
    if( sdl_flags & SDL_WINDOW_FULLSCREEN ) {
        flags |= SURFACE_STATE_FULLSCREEN;
    }
    if( sdl_flags & SDL_WINDOW_HIDDEN ) {
        flags |= SURFACE_STATE_IS_HIDDEN;
    }
    if( surf->is_focused ) {
        flags |= SURFACE_STATE_IS_FOCUSED;
    }
    return flags;
}
attr_media_api void surface_set_fullscreen(
    SurfaceHandle* surface, _Bool is_fullscreen
) {
    struct SDLSurface* surf = surface;
    SDL_SetWindowFullscreen( surf->handle, is_fullscreen );
}
attr_media_api void surface_set_hidden( SurfaceHandle* surface, _Bool is_hidden ) {
    struct SDLSurface* surf = surface;
    if( is_hidden ) {
        SDL_HideWindow( surf->handle );
    } else {
        SDL_ShowWindow( surf->handle );
    }
}

KeyboardMod sdl_to_mod( SDL_Keymod mod ) {
    KeyboardMod res = 0;
    if( mod & SDL_KMOD_SHIFT ) {
        res |= KBMOD_SHIFT;
    }
    if( mod & SDL_KMOD_CTRL ) {
        res |= KBMOD_CTRL;
    }
    if( mod & SDL_KMOD_ALT ) {
        res |= KBMOD_ALT;
    }
    if( mod & SDL_KMOD_CAPS ) {
        res |= KBMOD_CAPSLK;
    }
    if( mod & SDL_KMOD_SCROLL ) {
        res |= KBMOD_SCRLK;
    }
    if( mod & SDL_KMOD_NUM ) {
        res |= KBMOD_NUMLK;
    }
    return res;
}
SDL_Keymod mod_to_sdl( KeyboardMod mod ) {
    SDL_Keymod res = 0;
    if( mod & KBMOD_SHIFT ) {
        res |= SDL_KMOD_SHIFT;
    }
    if( mod & KBMOD_CTRL ) {
        res |= SDL_KMOD_CTRL;
    }
    if( mod & KBMOD_ALT ) {
        res |= SDL_KMOD_ALT;
    }
    if( mod & KBMOD_CAPSLK ) {
        res |= SDL_KMOD_CAPS;
    }
    if( mod & KBMOD_SCRLK ) {
        res |= SDL_KMOD_SCROLL;
    }
    if( mod & KBMOD_NUMLK ) {
        res |= SDL_KMOD_NUM;
    }
    return res;
}

KeyboardCode sdl_to_key( SDL_Keycode kc ) {
    switch( kc ) {
        case SDLK_BACKSPACE     : return KB_BACKSPACE;
        case SDLK_TAB           : return KB_TAB;
        case SDLK_RETURN        : return KB_ENTER;
        case SDLK_LSHIFT        : return KB_SHIFT_LEFT;
        case SDLK_LCTRL         : return KB_CONTROL_LEFT;
        case SDLK_LALT          : return KB_ALT_LEFT;
        case SDLK_PAUSE         : return KB_PAUSE;
        case SDLK_CAPSLOCK      : return KB_CAPSLOCK;
        case SDLK_ESCAPE        : return KB_ESCAPE;
        case SDLK_SPACE         : return KB_SPACE;
        case SDLK_PAGEUP        : return KB_PAGE_UP;
        case SDLK_PAGEDOWN      : return KB_PAGE_DOWN;
        case SDLK_END           : return KB_END;
        case SDLK_HOME          : return KB_HOME;
        case SDLK_LEFT          : return KB_ARROW_LEFT;
        case SDLK_UP            : return KB_ARROW_UP;
        case SDLK_RIGHT         : return KB_ARROW_RIGHT;
        case SDLK_DOWN          : return KB_ARROW_DOWN;
        case SDLK_PRINTSCREEN   : return KB_PRINT_SCREEN;
        case SDLK_INSERT        : return KB_INSERT;
        case SDLK_DELETE        : return KB_DELETE;
        case SDLK_0             : return KB_0;
        case SDLK_1             : return KB_1;
        case SDLK_2             : return KB_2;
        case SDLK_3             : return KB_3;
        case SDLK_4             : return KB_4;
        case SDLK_5             : return KB_5;
        case SDLK_6             : return KB_6;
        case SDLK_7             : return KB_7;
        case SDLK_8             : return KB_8;
        case SDLK_9             : return KB_9;
        case SDLK_A             : return KB_A;
        case SDLK_B             : return KB_B;
        case SDLK_C             : return KB_C;
        case SDLK_D             : return KB_D;
        case SDLK_E             : return KB_E;
        case SDLK_F             : return KB_F;
        case SDLK_G             : return KB_G;
        case SDLK_H             : return KB_H;
        case SDLK_I             : return KB_I;
        case SDLK_J             : return KB_J;
        case SDLK_K             : return KB_K;
        case SDLK_L             : return KB_L;
        case SDLK_M             : return KB_M;
        case SDLK_N             : return KB_N;
        case SDLK_O             : return KB_O;
        case SDLK_P             : return KB_P;
        case SDLK_Q             : return KB_Q;
        case SDLK_R             : return KB_R;
        case SDLK_S             : return KB_S;
        case SDLK_T             : return KB_T;
        case SDLK_U             : return KB_U;
        case SDLK_V             : return KB_V;
        case SDLK_W             : return KB_W;
        case SDLK_X             : return KB_X;
        case SDLK_Y             : return KB_Y;
        case SDLK_Z             : return KB_Z;
        case SDLK_LGUI          : return KB_SUPER_LEFT;
        case SDLK_RGUI          : return KB_SUPER_RIGHT;
        case SDLK_KP_0          : return KB_PAD_0;
        case SDLK_KP_1          : return KB_PAD_1;
        case SDLK_KP_2          : return KB_PAD_2;
        case SDLK_KP_3          : return KB_PAD_3;
        case SDLK_KP_4          : return KB_PAD_4;
        case SDLK_KP_5          : return KB_PAD_5;
        case SDLK_KP_6          : return KB_PAD_6;
        case SDLK_KP_7          : return KB_PAD_7;
        case SDLK_KP_8          : return KB_PAD_8;
        case SDLK_KP_9          : return KB_PAD_9;
        case SDLK_KP_PLUS       : return KB_PAD_ADD;
        case SDLK_KP_MULTIPLY   : return KB_PAD_MULTIPLY;
        case SDLK_KP_MINUS      : return KB_PAD_SUBTRACT;
        case SDLK_KP_DIVIDE     : return KB_PAD_DIVIDE;
        case SDLK_KP_PERIOD     : return KB_PAD_DOT;
        case SDLK_F1            : return KB_F1;
        case SDLK_F2            : return KB_F2;
        case SDLK_F3            : return KB_F3;
        case SDLK_F4            : return KB_F4;
        case SDLK_F5            : return KB_F5;
        case SDLK_F6            : return KB_F6;
        case SDLK_F7            : return KB_F7;
        case SDLK_F8            : return KB_F8;
        case SDLK_F9            : return KB_F9;
        case SDLK_F10           : return KB_F10;
        case SDLK_F11           : return KB_F11;
        case SDLK_F12           : return KB_F12;
        case SDLK_F13           : return KB_F13;
        case SDLK_F14           : return KB_F14;
        case SDLK_F15           : return KB_F15;
        case SDLK_F16           : return KB_F16;
        case SDLK_F17           : return KB_F17;
        case SDLK_F18           : return KB_F18;
        case SDLK_F19           : return KB_F19;
        case SDLK_F20           : return KB_F20;
        case SDLK_F21           : return KB_F21;
        case SDLK_F22           : return KB_F22;
        case SDLK_F23           : return KB_F23;
        case SDLK_F24           : return KB_F24;
        case SDLK_NUMLOCKCLEAR  : return KB_NUM_LOCK;
        case SDLK_SCROLLLOCK    : return KB_SCROLL_LOCK;
        case SDLK_SEMICOLON     : return KB_SEMICOLON;
        case SDLK_EQUALS        : return KB_EQUALS;
        case SDLK_COMMA         : return KB_COMMA;
        case SDLK_MINUS         : return KB_MINUS;
        case SDLK_PERIOD        : return KB_PERIOD;
        case SDLK_SLASH         : return KB_SLASH;
        case SDLK_GRAVE         : return KB_BACKTICK;
        case SDLK_LEFTBRACKET   : return KB_BRACKET_LEFT;
        case SDLK_BACKSLASH     : return KB_BACKSLASH;
        case SDLK_RIGHTBRACKET  : return KB_BRACKET_RIGHT;
        case SDLK_DBLAPOSTROPHE : return KB_QUOTE;
        case SDLK_RSHIFT        : return KB_SHIFT_RIGHT;
        case SDLK_RALT          : return KB_ALT_RIGHT;
        case SDLK_RCTRL         : return KB_CONTROL_RIGHT;

        default: return KB_UNKNOWN;
    }

}
SDL_Keycode key_to_sdl( KeyboardCode kc ) {
    switch( kc ) {
        case KB_BACKSPACE        : return SDLK_BACKSPACE;
        case KB_TAB              : return SDLK_TAB;
        case KB_ENTER            : return SDLK_RETURN;
        case KB_SHIFT_LEFT       : return SDLK_LSHIFT;
        case KB_CONTROL_LEFT     : return SDLK_LCTRL;
        case KB_ALT_LEFT         : return SDLK_LALT;
        case KB_PAUSE            : return SDLK_PAUSE;
        case KB_CAPSLOCK         : return SDLK_CAPSLOCK;
        case KB_ESCAPE           : return SDLK_ESCAPE;
        case KB_SPACE            : return SDLK_SPACE;
        case KB_PAGE_UP          : return SDLK_PAGEUP;
        case KB_PAGE_DOWN        : return SDLK_PAGEDOWN;
        case KB_END              : return SDLK_END;
        case KB_HOME             : return SDLK_HOME;
        case KB_ARROW_LEFT       : return SDLK_LEFT;
        case KB_ARROW_UP         : return SDLK_UP;
        case KB_ARROW_RIGHT      : return SDLK_RIGHT;
        case KB_ARROW_DOWN       : return SDLK_DOWN;
        case KB_PRINT_SCREEN     : return SDLK_PRINTSCREEN;
        case KB_INSERT           : return SDLK_INSERT;
        case KB_DELETE           : return SDLK_DELETE;
        case KB_0                : return SDLK_0;
        case KB_1                : return SDLK_1;
        case KB_2                : return SDLK_2;
        case KB_3                : return SDLK_3;
        case KB_4                : return SDLK_4;
        case KB_5                : return SDLK_5;
        case KB_6                : return SDLK_6;
        case KB_7                : return SDLK_7;
        case KB_8                : return SDLK_8;
        case KB_9                : return SDLK_9;
        case KB_A                : return SDLK_A;
        case KB_B                : return SDLK_B;
        case KB_C                : return SDLK_C;
        case KB_D                : return SDLK_D;
        case KB_E                : return SDLK_E;
        case KB_F                : return SDLK_F;
        case KB_G                : return SDLK_G;
        case KB_H                : return SDLK_H;
        case KB_I                : return SDLK_I;
        case KB_J                : return SDLK_J;
        case KB_K                : return SDLK_K;
        case KB_L                : return SDLK_L;
        case KB_M                : return SDLK_M;
        case KB_N                : return SDLK_N;
        case KB_O                : return SDLK_O;
        case KB_P                : return SDLK_P;
        case KB_Q                : return SDLK_Q;
        case KB_R                : return SDLK_R;
        case KB_S                : return SDLK_S;
        case KB_T                : return SDLK_T;
        case KB_U                : return SDLK_U;
        case KB_V                : return SDLK_V;
        case KB_W                : return SDLK_W;
        case KB_X                : return SDLK_X;
        case KB_Y                : return SDLK_Y;
        case KB_Z                : return SDLK_Z;
        case KB_SUPER_LEFT       : return SDLK_LGUI;
        case KB_SUPER_RIGHT      : return SDLK_RGUI;
        case KB_PAD_0            : return SDLK_KP_0;
        case KB_PAD_1            : return SDLK_KP_1;
        case KB_PAD_2            : return SDLK_KP_2;
        case KB_PAD_3            : return SDLK_KP_3;
        case KB_PAD_4            : return SDLK_KP_4;
        case KB_PAD_5            : return SDLK_KP_5;
        case KB_PAD_6            : return SDLK_KP_6;
        case KB_PAD_7            : return SDLK_KP_7;
        case KB_PAD_8            : return SDLK_KP_8;
        case KB_PAD_9            : return SDLK_KP_9;
        case KB_PAD_ADD          : return SDLK_KP_PLUS;
        case KB_PAD_MULTIPLY     : return SDLK_KP_MULTIPLY;
        case KB_PAD_SUBTRACT     : return SDLK_KP_MINUS;
        case KB_PAD_DIVIDE       : return SDLK_KP_DIVIDE;
        case KB_PAD_DOT          : return SDLK_KP_PERIOD;
        case KB_F1               : return SDLK_F1;
        case KB_F2               : return SDLK_F2;
        case KB_F3               : return SDLK_F3;
        case KB_F4               : return SDLK_F4;
        case KB_F5               : return SDLK_F5;
        case KB_F6               : return SDLK_F6;
        case KB_F7               : return SDLK_F7;
        case KB_F8               : return SDLK_F8;
        case KB_F9               : return SDLK_F9;
        case KB_F10              : return SDLK_F10;
        case KB_F11              : return SDLK_F11;
        case KB_F12              : return SDLK_F12;
        case KB_F13              : return SDLK_F13;
        case KB_F14              : return SDLK_F14;
        case KB_F15              : return SDLK_F15;
        case KB_F16              : return SDLK_F16;
        case KB_F17              : return SDLK_F17;
        case KB_F18              : return SDLK_F18;
        case KB_F19              : return SDLK_F19;
        case KB_F20              : return SDLK_F20;
        case KB_F21              : return SDLK_F21;
        case KB_F22              : return SDLK_F22;
        case KB_F23              : return SDLK_F23;
        case KB_F24              : return SDLK_F24;
        case KB_NUM_LOCK         : return SDLK_NUMLOCKCLEAR;
        case KB_SCROLL_LOCK      : return SDLK_SCROLLLOCK;
        case KB_SEMICOLON        : return SDLK_SEMICOLON;
        case KB_EQUALS           : return SDLK_EQUALS;
        case KB_COMMA            : return SDLK_COMMA;
        case KB_MINUS            : return SDLK_MINUS;
        case KB_PERIOD           : return SDLK_PERIOD;
        case KB_SLASH            : return SDLK_SLASH;
        case KB_BACKTICK         : return SDLK_GRAVE;
        case KB_BRACKET_LEFT     : return SDLK_LEFTBRACKET;
        case KB_BACKSLASH        : return SDLK_BACKSLASH;
        case KB_BRACKET_RIGHT    : return SDLK_RIGHTBRACKET;
        case KB_QUOTE            : return SDLK_DBLAPOSTROPHE;
        case KB_SHIFT_RIGHT      : return SDLK_RSHIFT;
        case KB_ALT_RIGHT        : return SDLK_RALT;
        case KB_CONTROL_RIGHT    : return SDLK_RCTRL;

        case KB_RIGHT_CLICK_MENU:
        case KB_UNKNOWN:
        case KB_COUNT:   return SDLK_UNKNOWN;
    }
}

#endif /* Platform Posix */
