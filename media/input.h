#if !defined(MEDIA_INPUT_H)
#define MEDIA_INPUT_H
/**
 * @file   input.h
 * @brief  Input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   April 20, 2024
*/
#include "media/defines.h"
// IWYU pragma: begin_exports
#include "media/input/gamepad.h"
#include "media/input/keyboard.h"
#include "media/input/mouse.h"
// IWYU pragma: end_exports

/// @brief Query how much memory is required for input subsystem.
/// @return Size of input subsystem in bytes.
attr_media_api uintptr_t input_subsystem_query_memory_requirement(void);

/// @brief Initialize input subsystem.
/// @note This function assumes that @c buffer has been zeroed out.
/// @warning MUST be called before querying for inputs.
/// @param[in] buffer Buffer for input subsystem, MUST be able to hold result of media_input_query_memory_requirement()!
/// @return
///     - true  : Input subsystem was successfully initialized.
///     - false : Failed to initialize subsystem, check logs for more info.
attr_media_api _Bool input_subsystem_initialize( void* buffer );
/// @brief Update input subsystem.
/// @warning Must be called from thread that initialized input subsystem.
/// @note Should be called before surface_pump_events().
attr_media_api void input_subsystem_update(void);
/// @brief Shutdown input subsystem.
/// @note Does not free input subsystem buffer.
attr_media_api void input_subsystem_shutdown(void);

/// @brief Query key modifiers.
/// @return Bitfield of key modifiers.
attr_media_api KeyboardMod input_keyboard_query_mod(void);
/// @brief Query state of given key.
/// @param keycode Code of key to query.
/// @return
///     - true  : Key is being pressed.
///     - false : Key is not being pressed.
attr_media_api _Bool input_keyboard_query_key( KeyboardCode keycode );
/// @brief Copy entire state of keyboard to buffer.
/// @param[out] out_keyboard Array to copy state of keyboard to.
attr_media_api void input_keyboard_copy_state( KeyboardState* out_state );

/// @brief Query state of mouse buttons.
/// @return Bitfield of mouse buttons.
attr_media_api MouseButton input_mouse_query_buttons(void);
/// @brief Query absolute position of mouse.
/// @details
/// X and Y are in total monitor space, use input_mouse_position_to_client()
/// to map them to a surface's client area.
/// @param[out] out_x Pointer to write out x position of mouse pointer.
/// @param[out] out_y Pointer to write out y position of mouse pointer.
attr_media_api void input_mouse_query_position( int32_t* out_x, int32_t* out_y );
/// @brief Map absolute position of mouse to surface client area.
/// @param[in]     surface  Handle to surface to map to.
/// @param[in,out] in_out_x Pointer to read x and write out mapped x position.
/// @param[in,out] in_out_y Pointer to read y and write out mapped y position.
attr_media_api void input_mouse_position_to_client(
    SurfaceHandle* surface, int32_t* in_out_x, int32_t* in_out_y );
/// @brief Query mouse delta.
/// @param[out] out_x Pointer to write out x delta.
/// @param[out] out_y Pointer to write out y delta.
attr_media_api void input_mouse_query_delta( int32_t* out_x, int32_t* out_y );

/// @brief Query state of gamepad at given index.
/// @param      index     Index of gamepad to query. Valid range is 0..#GAMEPAD_MAX_COUNT.
/// @param[out] out_state Pointer to write state of gamepad to.
/// @return
///     - true  : Gamepad is active and its state was written to @c out_state.
///     - false : Gamepad is inactive.
attr_media_api _Bool input_gamepad_query_state(
    uint32_t index, GamepadState* out_state );
/// @brief Set rumble state of gamepad at given index.
/// @param index       Index of gamepad to set rumble state of. Valid range is 0..#GAMEPAD_MAX_COUNT.
/// @param motor_left  Value to set left rumble motor to. Valid range is 0..UINT16_MAX.
/// @param motor_right Value to set right rumble motor to. Valid range is 0..UINT16_MAX.
/// @return 
///     - true  : Gamepad is active and its rumble values were set.
///     - false : Gamepad is inactive.
attr_media_api _Bool input_gamepad_rumble_set(
    uint32_t index, uint16_t motor_left, uint16_t motor_right );
/// @brief Clear rumble state of gamepad at given index.
/// @param index       Index of gamepad to set rumble state of. Valid range is 0..#GAMEPAD_MAX_COUNT.
/// @return 
///     - true  : Gamepad is active and its rumble values were cleared.
///     - false : Gamepad is inactive.
attr_header _Bool input_gamepad_rumble_clear( uint32_t index ) {
    return input_gamepad_rumble_set( index, 0, 0 );
}

/// @brief Get value of given key in keyboard state.
/// @param[in] state Pointer to keyboard state.
/// @param     code  Keycode to query.
/// @return
///     - true  : Key is pressed.
///     - false : Key is not pressed.
attr_header
attr_always_inline _Bool keyboard_state_get_key(
    const KeyboardState* state, KeyboardCode code
) {
    uint32_t index = (uint32_t)code;
    if( !index || index >= KB_COUNT ) {
        return false;
    }

    return (state->keys[index / 8] & (1 << (index % 8))) == (1 << (index % 8));
}
/// @brief Set value of given key in keyboard state.
/// @param[in] state Pointer to keyboard state.
/// @param     code  Keycode to set.
/// @param     value Whether to set key as pressed or key released.
attr_header
attr_always_inline void keyboard_state_set_key( 
    KeyboardState* state, KeyboardCode code, _Bool value
) {
    uint32_t index = (uint32_t)code;
    if( !index || index >= KB_COUNT ) {
        return;
    }

    if( value ) {
        state->keys[index / 8] |= (1 << (index % 8));
    } else {
        state->keys[index / 8] &= ~(1 << (index % 8));
    }
}

#endif /* header guard */
