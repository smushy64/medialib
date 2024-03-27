/**
 * @file   gamepad.c
 * @brief  media/gamepad win32 implementation.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "impl/win32/common.h"

#if defined(CORE_PLATFORM_WINDOWS)
#include "core/prelude.h"
#include "media/gamepad.h"

attr_media_api b32 media_gamepad_query_state(
    u32 index, InputGamepadState* out_state
) {
    if( !global_win32_state->active_gamepads[index] ) {
        return false;
    }

    XINPUT_STATE xinput;
    if( XInputGetState( index, &xinput ) != ERROR_SUCCESS ) {
        global_win32_state->active_gamepads[index] = false;
        global_win32_state->gamepad_rumble[index][0] = 0;
        global_win32_state->gamepad_rumble[index][1] = 0;

        return false;
    }
    XINPUT_GAMEPAD* pad = &xinput.Gamepad;

    out_state->buttons = bitfield_clear( pad->wButtons,
        INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_LEFT |
        INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_RIGHT );

    out_state->buttons |= pad->bLeftTrigger > (U8_MAX / 2) ?
        INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_LEFT : 0;
    out_state->buttons |= pad->bRightTrigger > (U8_MAX / 2) ?
        INPUT_GAMEPAD_EXT_BUTTON_TRIGGER_RIGHT : 0;

    out_state->trigger_left  = pad->bLeftTrigger;
    out_state->trigger_right = pad->bRightTrigger;

    out_state->stick_left_x  = pad->sThumbLX;
    out_state->stick_left_y  = pad->sThumbLY;
    out_state->stick_right_x = pad->sThumbRX;
    out_state->stick_right_y = pad->sThumbRY;

    return true;
}
attr_media_api void media_gamepad_set_rumble(
    u32 index, u16 motor_left, u16 motor_right
) {
    if( global_win32_state->active_gamepads[index] ) {
        global_win32_state->gamepad_rumble[index][0] = motor_left;
        global_win32_state->gamepad_rumble[index][1] = motor_right;

        XINPUT_VIBRATION vib = { motor_left, motor_right };
        XInputSetState( index, &vib );
    }
}
attr_media_api void media_gamepad_query_rumble(
    u32 index, u16* out_motor_left, u16* out_motor_right
) {
    *out_motor_left  = global_win32_state->gamepad_rumble[index][0];
    *out_motor_right = global_win32_state->gamepad_rumble[index][1];
}

#endif /* Platform Windows */

