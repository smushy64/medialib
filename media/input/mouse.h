#if !defined(MEDIA_INPUT_MOUSE_H)
#define MEDIA_INPUT_MOUSE_H
/**
 * @file   mouse.h
 * @brief  Mouse input handling.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 27, 2024
*/
#include "core/types.h"
#include "media/attributes.h"

/// @brief Mouse Buttons bitfield.
typedef enum InputMouseButton : u8 {
    /// @brief Left mouse button.
    INPUT_MOUSE_BUTTON_LEFT    = (1 << 0),
    /// @brief Middle mouse button.
    INPUT_MOUSE_BUTTON_MIDDLE  = (1 << 1),
    /// @brief Right mouse button.
    INPUT_MOUSE_BUTTON_RIGHT   = (1 << 2),
    /// @brief Extra mouse button 1.
    INPUT_MOUSE_BUTTON_EXTRA_1 = (1 << 3),
    /// @brief Extra mouse button 2.
    INPUT_MOUSE_BUTTON_EXTRA_2 = (1 << 4),
} InputMouseButton;
/// @brief Query state of mouse buttons.
/// @return Mouse buttons bitfield.
attr_media_api InputMouseButton media_mouse_query_buttons(void);
/// @brief Query absolute mouse position.
/// @param[out] out_x, out_y Pointers to write absolute mouse positions to.
attr_media_api void media_mouse_query_absolute( i32* out_x, i32* out_y );
/// @brief Query mouse delta.
/// @param[out] out_x, out_y Pointers to write mouse delta positions to.
attr_media_api void media_mouse_query_delta( i32* out_x, i32* out_y );
/// @brief Format #InputMouseButton as a string.
/// @param button Button to format.
/// @param[out] opt_out_len (optional) Length of format string.
/// @return Mouse button as a string.
attr_header const char* input_mouse_button_to_string(
    InputMouseButton button, usize* opt_out_len
) {
    #define result( text ) do {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(text) - 1;\
        }\
        return text;\
    } while(0)

    switch( button ) {
        case INPUT_MOUSE_BUTTON_LEFT    : result( "Mouse Button Left" );
        case INPUT_MOUSE_BUTTON_MIDDLE  : result( "Mouse Button Middle" );
        case INPUT_MOUSE_BUTTON_RIGHT   : result( "Mouse Button Right" );
        case INPUT_MOUSE_BUTTON_EXTRA_1 : result( "Mouse Button Extra 1" );
        case INPUT_MOUSE_BUTTON_EXTRA_2 : result( "Mouse Button Extra 2" );
        default: {
            if(
                button > (INPUT_MOUSE_BUTTON_LEFT |
                INPUT_MOUSE_BUTTON_RIGHT   |
                INPUT_MOUSE_BUTTON_MIDDLE  |
                INPUT_MOUSE_BUTTON_EXTRA_1 |
                INPUT_MOUSE_BUTTON_EXTRA_2)
            ) {
                result( "Invalid Mouse Button" );
            } else {
                result( "Mouse Buttons" );
            }
        };
    }
    #undef result
}

#endif /* header guard */
