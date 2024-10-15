/**
 * @file   input.c
 * @brief  Posix input implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   October 14, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_POSIX)
#include "media/input.h"
#include <SDL3/SDL.h>

attr_media_api uintptr_t input_subsystem_query_memory_requirement(void) {
    // TODO(alicia): 
    return 1;
}
attr_media_api _Bool input_subsystem_initialize( void* buffer ) {
    unused(buffer);
    return SDL_InitSubSystem( SDL_INIT_GAMEPAD );
}
attr_media_api void input_subsystem_update(void) {
    // TODO(alicia): 
}
attr_media_api void input_subsystem_shutdown(void) {
    SDL_QuitSubSystem( SDL_INIT_GAMEPAD );
}
attr_media_api KeyboardMod input_keyboard_query_mod(void) {
    // TODO(alicia): 
    return 0;
}
attr_media_api _Bool input_keyboard_query_key( KeyboardCode keycode ) {
    // TODO(alicia):
    unused(keycode);
    return false;
}
attr_media_api void input_keyboard_copy_state( KeyboardState* out_state ) {
    // TODO(alicia):
    unused(out_state);
}
attr_media_api MouseButton input_mouse_query_buttons(void) {
    // TODO(alicia):
    return 0;
}
attr_media_api void input_mouse_query_position( int32_t* out_x, int32_t* out_y ) {
    // TODO(alicia):
    unused(out_x,out_y);
}
attr_media_api void input_mouse_position_to_client(
    SurfaceHandle* surface, int32_t* in_out_x, int32_t* in_out_y 
) {
    // TODO(alicia):
    unused(surface,in_out_x,in_out_y);
}
attr_media_api void input_mouse_query_delta( int32_t* out_x, int32_t* out_y ) {
    // TODO(alicia):
    unused(out_x,out_y);
}

#endif /* Platform Posix */

