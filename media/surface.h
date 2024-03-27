#if !defined(MEDIA_SURFACE_H)
#define MEDIA_SURFACE_H
/**
 * @file   surface.h
 * @brief  Create and manipulate a surface (window).
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "core/defines.h"
#include "core/string.h"
#include "media/types.h"
#include "media/attributes.h"

enum InputKeymod  : u16;
enum InputKeycode : u16;
enum InputMouseButton : u8;

/// @brief Flags for creating a surface.
typedef enum MediaSurfaceCreateFlags {
    /// @brief Created surface should be hidden upon creation.
    MEDIA_SURFACE_CREATE_FLAG_HIDDEN     = (1 << 0),
    /// @brief Surface should be resizeable.
    MEDIA_SURFACE_CREATE_FLAG_RESIZEABLE = (1 << 1),
    /// @brief Surface should be fullscreen upon creation.
    /// @note Some platforms don't have a notion of fullscreen/windowed surfaces.
    MEDIA_SURFACE_CREATE_FLAG_FULLSCREEN = (1 << 2),
    /// @brief Enable surface border dark mode.
    /// @note Only works on platforms that have customizeable surface border.
    MEDIA_SURFACE_CREATE_FLAG_DARK_MODE   = (1 << 3),
    /// @brief Disable minimize button.
    /// @note Not all platforms have surfaces with minimize button.
    MEDIA_SURFACE_CREATE_FLAG_NO_MINIMIZE = (1 << 4),
    /// @brief Disable maximize button.
    /// @note Not all platforms have surfaces with maximize button.
    MEDIA_SURFACE_CREATE_FLAG_NO_MAXIMIZE = (1 << 5),
    /// @brief Surface should be created with OpenGL support.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    MEDIA_SURFACE_CREATE_FLAG_OPENGL = (1 << 8),
    /// @brief Surface should be created with Vulkan support.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    MEDIA_SURFACE_CREATE_FLAG_VULKAN = (1 << 9),
#if defined(CORE_PLATFORM_WINDOWS)
    /// @brief Surface should be created with DirectX support.
    /// @note Only valid on Windows.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    MEDIA_SURFACE_CREATE_FLAG_DIRECTX = (1 << 10),
#endif
#if defined(CORE_PLATFORM_MACOS) || defined(CORE_PLATFORM_IOS)
    /// @brief Surface should be created with Metal support.
    /// @note Only valid on MacOS and iOS.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    MEDIA_SURFACE_CREATE_FLAG_METAL = (1 << 11),
#endif
} MediaSurfaceCreateFlags;

/// @brief Window modes for media surface.
/// @remarks Surface is always fullscreen on platforms that don't have 'windowed' mode.
typedef enum MediaSurfaceMode {
    /// @brief Windowed surface.
    MEDIA_SURFACE_MODE_WINDOWED   = 0,
    /// @brief Fullscreen surface.
    MEDIA_SURFACE_MODE_FULLSCREEN = 1,
} MediaSurfaceMode;

/// @brief Types of media surface callbacks.
typedef enum MediaSurfaceCallbackType {
    /// @brief User is trying to close surface.
    /// @note This type does not have any associated data.
    MEDIA_SURFACE_CALLBACK_TYPE_CLOSE,
    /// @brief Surface was focused or unfocused.
    /// @see MediaSurfaceCallbackData::focus
    MEDIA_SURFACE_CALLBACK_TYPE_FOCUS,
    /// @brief Surface's dimensions were updated.
    /// @see MediaSurfaceCallbackData::resize
    MEDIA_SURFACE_CALLBACK_TYPE_RESIZE,
    /// @brief Surface's position has been updated.
    /// @note Not all platforms support moving surfaces.
    /// @see MediaSurfaceCallbackData::pos
    MEDIA_SURFACE_CALLBACK_TYPE_POSITION,
    /// @brief Mouse buttons were pressed/release.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see MediaSurfaceCallbackData::mouse_button
    MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_BUTTON,
    /// @brief Mouse has moved inside surface.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see MediaSurfaceCallbackData::mouse_move
    MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE,
    /// @brief Mouse has moved inside surface, delta from last position.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see MediaSurfaceCallbackData::mouse_delta
    MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA,
    /// @brief Mouse wheel was moved.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see MediaSurfaceCallbackData::mouse_wheel
    MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_WHEEL,
    /// @brief Key was pressed/released.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support keyboard input.
    /// @see MediaSurfaceCallbackData::key
    MEDIA_SURFACE_CALLBACK_TYPE_KEY,
    /// @brief Text was typed.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support keyboard input.
    /// @see MediaSurfaceCallbackData::text
    MEDIA_SURFACE_CALLBACK_TYPE_TEXT,
} MediaSurfaceCallbackType;

/// @brief Discriminated union of surface callback data.
typedef struct MediaSurfaceCallbackData {
    /// @brief What type of data is being sent.
    MediaSurfaceCallbackType type;
    /// @brief Union of callback data.
    union {
        /// @brief Surface focus update callback data.
        /// @details
        /// - @c gained - (b32) True if surface gained focus, false if it lost focus.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_FOCUS
        struct {
            b32 gained;
        } focus;
        /// @brief Surface dimensions update callback data.
        /// @details
        /// - @c old_w, @c old_h - (i32) Old surface dimensions.
        /// - @c new_w, @c new_h - (i32) New surface dimensions.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_RESIZE
        struct {
            i32 old_w, old_h, new_w, new_h;
        } resize;
        /// @brief Surface position update callback data.
        /// @details
        /// - @c old_x, @c old_y - (i32) Old surface position coordinates.
        /// - @c new_x, @c new_y - (i32) New surface position coordinates.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_POSITION
        struct {
            i32 old_x, old_y, new_x, new_y;
        } pos;
        /// @brief Mouse button callback data.
        /// @details
        /// - @c buttons_state - (InputMouseButton) Bitfield of mouse state.
        /// - @c buttons_changed - (InputMouseButton) Bitfield of mouse buttons that changed state.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_BUTTON
        struct {
            enum InputMouseButton buttons_state;
            enum InputMouseButton buttons_changed;
        } mouse_button;
        /// @brief Mouse move callback data.
        /// @details
        /// - @c x - (i32) Absolute X position.
        /// - @c y - (i32) Absolute Y position.
        ///
        /// X position is in range (0 .. surface width), min being left side of surface and max being right side.
        ///
        /// Y position is in range (0 .. surface height), min being bottom of surface and max being top of surface.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE
        struct {
            i32 x, y;
        } mouse_move;
        /// @brief Mouse move relative callback data.
        /// @details
        /// - @c x - (i32) Delta between last X absolute position and current position.
        /// - @c y - (i32) Delta between last Y absolute position and current position.
        ///
        /// A negative X value indicates movement to the left and positive indicates movement to the right.
        ///
        /// A negative Y value indicates movement down and positive indicates movement up.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA
        struct {
            i32 x, y;
        } mouse_delta;
        /// @brief Mouse wheel callback data.
        /// @details
        /// - @c delta - (i32) Direction that scroll wheel moved, range (-1 .. 1).
        /// - @c is_horizontal - (b32) Whether scroll is vertical or horizontal.
        ///
        /// If is_horizontal is @c true:
        /// - @c -1 - Scrolled to the left.
        /// - @c 1 - Scrolled to the right.
        ///
        /// If is_horizontal is @c false:
        /// - @c -1 - Scrolled away from the screen (down).
        /// - @c 1 - Scrolled towards the screen (up).
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_WHEEL
        struct {
            i32 delta;
            b32 is_horizontal;
        } mouse_wheel;
        /// @brief Key callback data.
        /// @details
        /// - @c code - (InputKeycode) Key code of key that was pressed/released.
        /// - @c mod - (InputKeymod) Bitfield of key modifiers.
        /// - @c is_down (b32) Whether key was pressed or released.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_KEY
        struct {
            enum InputKeycode code;
            enum InputKeymod  mod;
            b32 is_down;
        } key;
        /// @brief Text callback data.
        /// - @c utf8 - (char[16]) UTF-8 codepoints.
        ///
        /// @note @c utf8 is not necessarily null-terminated.
        /// Use surface_callback_data_text_to_string() to convert to a String.
        ///
        /// @see MEDIA_SURFACE_CALLBACK_TYPE_TEXT
        struct {
            char utf8[16];
        } text;
        /// @brief Raw callback data bytes.
        u8 raw[sizeof(u64) * 2];
    };
} MediaSurfaceCallbackData;
/// @brief Convert callback data text to a String.
/// @param[in] data Pointer to callback data.
/// @return String containing typed text.
attr_header String surface_callback_data_text_to_string(
    const MediaSurfaceCallbackData* data
) {
    String result;
    result.cc  = data->text.utf8;
    result.len = sizeof( data->text.utf8 );
    usize null = 0;
    if( string_find( result, 0, &null ) ) {
        result.len = null;
    }
    return result;
}
/// @brief Function prototype for surface callback functions.
/// @param[in] surface Surface handle.
/// @param[in] data    Callback data.
/// @param[in] params  (optional) User parameters.
typedef void MediaSurfaceCallbackFN(
    const MediaSurface* surface, const MediaSurfaceCallbackData* data, void* params );
/// @brief Create a media surface.
/// @param name Name of surface (this is the title of surface on supported platforms).
/// @param x, y Position of surface.
/// @param w, h Width and height of surface.
/// @param flags Surface creation flags.
/// @param opt_callback (optional) Surface callback function.
/// @param[in] opt_callback_params (optional) User parameters for
/// surface callback function.
/// @param[in] opt_parent (optional) Surface parent.
/// @return Surface handle, NULL if function failed.
/// @note @c x and @c y are ignored if #MEDIA_SURFACE_CREATE_FLAG_FULLSCREEN is set.
/// @note @c w and @c h are ignored if #MEDIA_SURFACE_CREATE_FLAG_FULLSCREEN is set
/// (uses screen width and height instead).
/// @note @c x and @c y are in terms of surface's top left corner.
/// @note @c w and @c h are dimensions of surface's client area.
attr_media_api MediaSurface* media_surface_create(
    String name, i32 x, i32 y, i32 w, i32 h,
    MediaSurfaceCreateFlags flags, MediaSurfaceCallbackFN* opt_callback,
    void* opt_callback_params, MediaSurface* opt_parent );
/// @brief Destroy a media surface.
/// @param[in] surface Handle to surface to destroy.
attr_media_api void media_surface_destroy( MediaSurface* surface );
/// @brief Process surface events.
/// @warning Only the thread that created the surface should call this function.
/// @param[in] surface Surface whose events should be processed.
attr_media_api void media_surface_pump_events( MediaSurface* surface );
/// @brief Set surface callback function.
/// @param[in] surface Surface to set callback for.
/// @param callback Surface callback function.
/// @param[in] opt_callback_params (optional) Parameters for callback function.
/// @warning Only the thread that created the surface should use this function!
attr_media_api void media_surface_set_callback(
    MediaSurface* surface, MediaSurfaceCallbackFN* callback, void* opt_callback_params );
/// @brief Clear surface callback functions.
/// @param[in] surface Surface to clear callbacks for.
/// @warning Only the thread that created the surface should use this function!
attr_media_api void media_surface_clear_callback( MediaSurface* surface );
/// @brief Query name of surface.
/// @param[in] surface Surface to query name of.
/// @return String containing surface name (constant string).
attr_media_api String media_surface_query_name( const MediaSurface* surface );
/// @brief Set name of surface.
/// @param[in] surface Surface to set name of.
/// @param name New name of surface.
attr_media_api void media_surface_set_name( MediaSurface* surface, String name );
/// @brief Query position of surface.
/// @param[in] surface Surface to get position of.
/// @param[out] out_x, out_y Pointer to integers to write position to.
/// @note Position is in terms of surface's top left corner.
attr_media_api void media_surface_query_position(
    const MediaSurface* surface, i32* out_x, i32* out_y );
/// @brief Set position of surface.
/// @param[in] surface Surface to set position of.
/// @param x, y New position of surface.
/// @note Not all platforms support moving surface.
/// @note Position is in terms of surface's top left corner.
attr_media_api void media_surface_set_position(
    MediaSurface* surface, i32 x, i32 y );
/// @brief Query dimensions of surface.
/// @param[in] surface Surface to query dimensions of.
/// @param[out] out_w, out_h Pointer to integers to write dimensions to.
/// @note Width and height are always in terms of client area, not total window size.
attr_media_api void media_surface_query_dimensions(
    const MediaSurface* surface, i32* out_w, i32* out_h );
/// @brief Set dimensions of surface.
/// @param[in] surface Surface to set dimensions of.
/// @param w, h New dimensions of surface.
/// @note Width and height are always in terms of client area, not total window size.
/// @note Does nothing if surface is fullscreen.
attr_media_api void media_surface_set_dimensions(
    MediaSurface* surface, i32 w, i32 h );
/// @brief Query surface mode.
/// @param[in] surface Surface to get mode of.
/// @return Window mode.
/// @note Some platforms don't have a notion of fullscreen/windowed surfaces.
attr_media_api MediaSurfaceMode media_surface_query_mode(
    const MediaSurface* surface );
/// @brief Set surface mode.
/// @param[in] surface Surface to set mode of.
/// @param mode Window mode.
/// @note Some platforms don't have a notion of fullscreen/windowed surfaces.
attr_media_api void media_surface_set_mode(
    MediaSurface* surface, MediaSurfaceMode mode );
/// @brief Query if surface is hidden.
/// @param[in] surface Surface to query.
/// @return True if surface is hidden, false if it isn't.
attr_media_api b32 media_surface_query_hidden( const MediaSurface* surface );
/// @brief Hide/show surface.
/// @param[in] surface Surface to hide/show.
/// @param is_hidden If surface should be hidden or shown.
attr_media_api void media_surface_set_hidden( MediaSurface* surface, b32 is_hidden );

/// @brief Format surface mode as a string.
/// @param mode Mode to format.
/// @param[out] opt_out_len (optional) Length of formatted string.
/// @return Pointer to string.
attr_header const char* media_surface_mode_to_string(
    MediaSurfaceMode mode, usize* opt_out_len
) {
    #define result( str ) do {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(str) - 1;\
        }\
        return str;\
    } while(0)

    switch( mode ) {
        case MEDIA_SURFACE_MODE_WINDOWED:   result( "Windowed" );
        case MEDIA_SURFACE_MODE_FULLSCREEN: result( "Fullscreen" );
    }
    result( "Unknown" );
    #undef result
}
/// @brief Format surface callback type as a string.
/// @param type Callback type to format.
/// @param[out] opt_out_len (optional) Length of formatted string.
/// @return Pointer to string.
attr_header const char* media_surface_callback_type_to_string(
    MediaSurfaceCallbackType type, usize* opt_out_len
) {
    #define result( str ) do {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(str) - 1;\
        }\
        return str;\
    } while(0)

    switch( type ) {
        case MEDIA_SURFACE_CALLBACK_TYPE_CLOSE:            result( "Surface Close Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_FOCUS:            result( "Surface Focused/Unfocused Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_RESIZE:           result( "Surface Resized Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_POSITION:         result( "Surface Position Changed Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_BUTTON:     result( "Mouse Button Clicked Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE:       result( "Mouse Moved Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA: result( "Mouse Moved Delta Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_MOUSE_WHEEL:      result( "Mouse Wheel Scrolled Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_KEY:              result( "Key Press/Release Callback" );
        case MEDIA_SURFACE_CALLBACK_TYPE_TEXT:             result( "Text Input Callback" );
    }
    result( "Unknown Callback" );
    #undef result
}

#endif /* header guard */
