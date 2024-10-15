#if !defined(IMPL_SDL_SURFACE_H)
#define IMPL_SDL_SURFACE_H
/**
 * @file   surface.h
 * @brief  Posix surface.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 14, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_POSIX)
#include "media/surface.h"
#include "media/input/keyboard.h"
#include <SDL3/SDL.h>

struct SDLSurface {
    SDL_Window*        handle;
    SurfaceCallbackFN* callback;
    void*              callback_params;
    bool               is_focused;
    int x, y, w, h;
};

KeyboardCode sdl_to_key( SDL_Keycode kc );
SDL_Keycode key_to_sdl( KeyboardCode kc );

KeyboardMod sdl_to_mod( SDL_Keymod mod );
SDL_Keymod mod_to_sdl( KeyboardMod mod );

#endif /* Platform Posix */
#endif /* header guard */
