#if !defined(MEDIA_SURFACE_H)
#define MEDIA_SURFACE_H
/**
 * @file   surface.h
 * @brief  Create and manipulate a surface (window).
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "media/defines.h"
#include "media/types.h"

#define SURFACE_MAX_TITLE_LEN (255)

/// @brief Flags for creating a surface.
typedef enum SurfaceCreateFlags {
    /// @brief Created surface should be hidden upon creation.
    SURFACE_CREATE_FLAG_HIDDEN     = (1 << 0),
    /// @brief Surface should be resizeable.
    SURFACE_CREATE_FLAG_RESIZEABLE = (1 << 1),
    /// @brief Surface should be fullscreen upon creation.
    /// @note Some platforms don't have a notion of fullscreen/windowed surfaces.
    SURFACE_CREATE_FLAG_FULLSCREEN = (1 << 2),
    /// @brief Enable surface border dark mode.
    /// @note Only works on platforms that have customizeable surface border.
    SURFACE_CREATE_FLAG_DARK_MODE   = (1 << 3),
    /// @brief Disable minimize button.
    /// @note Not all platforms have surfaces with minimize button.
    SURFACE_CREATE_FLAG_NO_MINIMIZE = (1 << 4),
    /// @brief Disable maximize button.
    /// @note Not all platforms have surfaces with maximize button.
    SURFACE_CREATE_FLAG_NO_MAXIMIZE = (1 << 5),
    /// @brief Center surface on the X axis.
    /// @details
    /// Ignores @c x parameter when this flag is used.
    /// @note Not all platforms have a notion of surface position.
    SURFACE_CREATE_FLAG_X_CENTERED  = (1 << 6),
    /// @brief Center surface on the Y axis.
    /// @details
    /// Ignores @c y parameter when this flag is used.
    /// @note Not all platforms have a notion of surface position.
    SURFACE_CREATE_FLAG_Y_CENTERED  = (1 << 7),


    /// @brief Surface should be created with OpenGL support.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    SURFACE_CREATE_FLAG_OPENGL = (1 << 16),
    /// @brief Surface should be created with Vulkan support.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    SURFACE_CREATE_FLAG_VULKAN = (1 << 17),
#if defined(MEDIA_PLATFORM_WINDOWS)
    /// @brief Surface should be created with DirectX support.
    /// @note Only valid on Windows.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    SURFACE_CREATE_FLAG_DIRECTX = (1 << 18),
#endif
#if defined(MEDIA_PLATFORM_MACOS) || defined(MEDIA_PLATFORM_IOS)
    /// @brief Surface should be created with Metal support.
    /// @note Only valid on MacOS and iOS.
    /// @warning Cannot be combined with any other
    /// graphics backend surface create flags.
    SURFACE_CREATE_FLAG_METAL = (1 << 19),
#endif
} SurfaceCreateFlags;

/// @brief Surface state flags.
typedef enum SurfaceStateFlags {
    /// @brief Surface is hidden.
    SURFACE_STATE_IS_HIDDEN  = (1 << 0),
    /// @brief Surface is focused.
    SURFACE_STATE_IS_FOCUSED = (1 << 1),
    /// @brief Surface is fullscreen.
    SURFACE_STATE_FULLSCREEN = (1 << 2),
} SurfaceStateFlags;

/// @brief Types of media surface callbacks.
typedef enum SurfaceCallbackType {
    /// @brief User is trying to close surface.
    /// @note This type does not have any associated data.
    SURFACE_CALLBACK_TYPE_CLOSE,
    /// @brief Surface was focused or unfocused.
    /// @see SurfaceCallbackData::focus
    SURFACE_CALLBACK_TYPE_FOCUS,
    /// @brief Surface's dimensions were updated.
    /// @see SurfaceCallbackData::resize
    SURFACE_CALLBACK_TYPE_RESIZE,
    /// @brief Surface's position has been updated.
    /// @note Not all platforms support moving surfaces.
    /// @see SurfaceCallbackData::position
    SURFACE_CALLBACK_TYPE_POSITION,
    /// @brief Mouse buttons were pressed/release.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see SurfaceCallbackData::mouse_button
    SURFACE_CALLBACK_TYPE_MOUSE_BUTTON,
    /// @brief Mouse has moved inside surface.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see SurfaceCallbackData::mouse_move
    SURFACE_CALLBACK_TYPE_MOUSE_MOVE,
    /// @brief Mouse has moved inside surface, delta from last position.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see SurfaceCallbackData::mouse_move_delta
    SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA,
    /// @brief Mouse wheel was moved.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support mouse input.
    /// @see SurfaceCallbackData::mouse_wheel
    SURFACE_CALLBACK_TYPE_MOUSE_WHEEL,
    /// @brief Key was pressed/released.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support keyboard input.
    /// @see SurfaceCallbackData::key
    SURFACE_CALLBACK_TYPE_KEY,
    /// @brief Text was typed.
    /// @details Only the active surface receives this callback.
    /// @note Not all platforms support keyboard input.
    /// @see SurfaceCallbackData::text
    SURFACE_CALLBACK_TYPE_TEXT,
} SurfaceCallbackType;

/// @brief Discriminated union of surface callback data.
typedef struct SurfaceCallbackData {
    /// @brief What type of data is being sent.
    SurfaceCallbackType type;
    /// @brief Union of callback data.
    union {
        /// @brief Surface focus callback data.
        /// @details Valid when surface focus state changes.
        /// @see #SURFACE_CALLBACK_TYPE_FOCUS
        struct {
            /// @brief If focus was gained or lost.
            m_bool32 gained;
        } focus;
        /// @brief Surface resize callback data.
        /// @details Valid when surface is resized by user or through an API call.
        /// @see #SURFACE_CALLBACK_TYPE_RESIZE
        struct {
            /// @brief Old surface client area dimensions.
            m_int32 old_w, old_h;
            /// @brief New surface client area dimensions.
            m_int32 w, h;
        } resize;
        /// @brief Surface position callback data.
        /// @details Valid when surface is repositioned by user or through an API call.
        /// @see #SURFACE_CALLBACK_TYPE_POSITION
        struct {
            /// @brief Old surface screen position.
            m_int32 old_x, old_y;
            /// @brief New surface screen position.
            m_int32 x, y;
        } position;
        /// @brief Mouse button callback data.
        /// @details Mouse button press/release.
        /// @see #SURFACE_CALLBACK_TYPE_MOUSE_BUTTON
        struct {
            /// @brief Bitfield of current mouse button state.
            enum MouseButton state;
            /// @brief Bitfield of mouse buttons that changed from last frame.
            enum MouseButton delta;
        } mouse_button;
        /// @brief Mouse move callback data.
        /// @details Mouse move absolute.
        /// @see #SURFACE_CALLBACK_TYPE_MOUSE_MOVE
        struct {
            /// @brief Absolute mouse position.
            /// @details
            /// - x range: (0 .. surface width)
            /// - y range: (0 .. surface height), bottom to top of surface.
            ///
            /// if position is outside valid range, that means mouse pointer
            /// is outside of surface.
            m_int32 x, y;
        } mouse_move;
        /// @brief Mouse move callback data.
        /// @details Mouse move delta.
        /// @see #SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA
        struct {
            /// @brief Delta mouse position.
            /// @details
            /// - negative x is movement towards left side of screen.
            /// - positive x is movement towards right side of screen.
            /// - negative y is movement towards bottom side of screen.
            /// - positive y is movement towards top side of screen.
            m_int32 x, y;
        } mouse_move_delta;
        /// @brief Mouse wheel callback data.
        /// @details Mouse wheel delta.
        /// @see #SURFACE_CALLBACK_TYPE_MOUSE_WHEEL
        struct {
            /// @brief Direction that wheel is scrolled (-1 or 1).
            /// @details
            /// if @c is_horizontal is true:
            ///     - negative : scrolled left.
            ///     - positive : scrolled right.
            ///
            /// if @c is_horizontal is false:
            ///     - negative : scrolled down (away from screen).
            ///     - positive : scrolled up (towards screen).
            m_int32  delta;
            /// @brief Whether scroll direction is vertical or horizontal.
            m_bool32 is_horizontal;
        } mouse_wheel;
        /// @brief Key callback data.
        /// @details Key press/release.
        /// @see #SURFACE_CALLBACK_TYPE_KEY
        struct {
            /// @brief Code of key that was pressed/released.
            enum KeyboardCode code;
            /// @brief Bitfield of current modifier keys.
            /// @note Only valid when input subsystem is initialized.
            enum KeyboardMod  mod;
            /// @brief Whether key was pressed or released.
            m_bool32 is_down;
        } key;
        /// @brief Text callback data.
        /// @details Text from keyboard.
        /// @see #SURFACE_CALLBACK_TYPE_TEXT
        struct {
            /// @brief UTF-8 codepoints.
            char utf8[16];
        } text;
        /// @brief Raw callback data bytes.
        m_uint8 raw[sizeof(m_uint64) * 2];
    };
} SurfaceCallbackData;

/// @brief Function prototype for surface callback functions.
/// @param[in] surface Surface handle.
/// @param[in] data    Callback data.
/// @param[in] params  (optional) User parameters.
typedef void SurfaceCallbackFN(
    const SurfaceHandle* surface, const SurfaceCallbackData* data, void* params );
/// @brief Query how much memory is required to create a new surface.
/// @return Size of media surface.
attr_media_api m_uintptr surface_query_memory_requirement(void);
/// @brief Create a media surface.
/// @param      title_len           Length of title of surface.
/// @param[in]  title               Title of surface (this is the title of surface on supported platforms).
/// @param      x, y                Position of surface.
/// @param      w, h                Width and height of surface.
/// @param      flags               Surface creation flags.
/// @param      opt_callback        (optional) Surface callback function.
/// @param[in]  opt_callback_params (optional) User parameters for surface callback function.
/// @param[in]  opt_parent          (optional) Surface parent.
/// @param[out] out_surface         Pointer to memory to store surface in. Must be able to hold the result of surface_query_memory_requirement().
/// @return
///     - true  : Surface created successfully.
///     - false : Failed to create surface.
/// @note @c x and @c y are ignored if #SURFACE_CREATE_FLAG_FULLSCREEN is set.
/// @note @c w and @c h are ignored if #SURFACE_CREATE_FLAG_FULLSCREEN is set
/// (uses screen width and height instead).
/// @note @c x and @c y are in terms of surface's top left corner.
/// @note @c w and @c h are dimensions of surface's client area.
attr_media_api m_bool32 surface_create(
    m_uint32 title_len, const char* title, m_int32 x, m_int32 y, m_int32 w, m_int32 h,
    SurfaceCreateFlags flags, SurfaceCallbackFN* opt_callback,
    void* opt_callback_params, SurfaceHandle* opt_parent, SurfaceHandle* out_surface );
/// @brief Destroy a media surface.
/// @param[in] surface Handle to surface to destroy.
attr_media_api void surface_destroy( SurfaceHandle* surface );
/// @brief Process surface events.
/// @warning Only the thread that created the surface should call this function.
/// @param[in] surface Surface whose events should be processed.
attr_media_api void surface_pump_events( SurfaceHandle* surface );
/// @brief Set surface callback function.
/// @param[in] surface             Surface to set callback for.
/// @param     callback            Surface callback function.
/// @param[in] opt_callback_params (optional) Parameters for callback function.
/// @warning Only the thread that created the surface should use this function!
attr_media_api void surface_set_callback(
    SurfaceHandle* surface, SurfaceCallbackFN* callback, void* opt_callback_params );
/// @brief Clear surface callback functions.
/// @param[in] surface Surface to clear callbacks for.
/// @warning Only the thread that created the surface should use this function!
attr_media_api void surface_clear_callback( SurfaceHandle* surface );
/// @brief Get platform handle for surface.
/// @details
/// On Windows, returned value is an HWND.
/// @param[in] surface Surface to get handle for.
/// @return Platform handle.
attr_media_api void* surface_get_platform_handle( SurfaceHandle* surface );
/// @brief Query title of surface.
/// @param[in]  surface     Surface to query title of.
/// @param[out] opt_out_len (optional) Pointer to write length of title to.
/// @return Read-only pointer to surface title (UTF-8 encoded).
attr_media_api const char* surface_query_title(
    const SurfaceHandle* surface, m_uint32* opt_out_len );
/// @brief Set title of surface.
/// @param[in] surface Surface to set title of.
/// @param     len     Length of new title.
/// @param[in] title   Pointer to start of new title (UTF-8 encoded).
attr_media_api void surface_set_title(
    SurfaceHandle* surface, m_uint32 len, const char* title );
/// @brief Query position of surface.
/// @param[in]  surface      Surface to get position of.
/// @param[out] out_x, out_y Pointer to integers to write position to.
/// @note Position is in terms of surface's top left corner.
attr_media_api void surface_query_position(
    const SurfaceHandle* surface, m_int32* out_x, m_int32* out_y );
/// @brief Set position of surface.
/// @param[in] surface Surface to set position of.
/// @param     x, y    New position of surface.
/// @note Not all platforms support moving surface.
/// @note Position is in terms of surface's top left corner.
attr_media_api void surface_set_position(
    SurfaceHandle* surface, m_int32 x, m_int32 y );
/// @brief Query dimensions of surface.
/// @param[in]  surface      Surface to query dimensions of.
/// @param[out] out_w, out_h Pointer to integers to write dimensions to.
/// @note Width and height are always in terms of client area, not total window size.
attr_media_api void surface_query_dimensions(
    const SurfaceHandle* surface, m_int32* out_w, m_int32* out_h );
/// @brief Set dimensions of surface.
/// @param[in] surface Surface to set dimensions of.
/// @param     w, h    New dimensions of surface.
/// @note Width and height are always in terms of client area, not total window size.
/// @note Does nothing if surface is fullscreen.
attr_media_api void surface_set_dimensions(
    SurfaceHandle* surface, m_int32 w, m_int32 h );
/// @brief Query surface state.
/// @param[in] surface Surface to query state of.
/// @return State flags.
attr_media_api SurfaceStateFlags surface_query_state( const SurfaceHandle* surface );
/// @brief Set surface fullscreen mode.
/// @param[in] surface       Surface to set state.
/// @param     is_fullscreen If surface should be fullscreen or not.
/// @note Some platforms don't have a notion of fullscreen/windowed surfaces.
attr_media_api void surface_set_fullscreen(
    SurfaceHandle* surface, m_bool32 is_fullscreen );
/// @brief Hide/show surface.
/// @param[in] surface   Surface to hide/show.
/// @param     is_hidden If surface should be hidden or shown.
attr_media_api void surface_set_hidden( SurfaceHandle* surface, m_bool32 is_hidden );

/// @brief Format surface callback type as a string.
/// @param      type        Callback type to format.
/// @param[out] opt_out_len (optional) Length of formatted string.
/// @return Pointer to string.
attr_header const char* surface_callback_type_to_string(
    SurfaceCallbackType type, m_uintptr* opt_out_len
) {
    #define result( str ) do {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(str) - 1;\
        }\
        return str;\
    } while(0)

    switch( type ) {
        case SURFACE_CALLBACK_TYPE_CLOSE:            result( "Surface Close" );
        case SURFACE_CALLBACK_TYPE_FOCUS:            result( "Surface Focused/Unfocused" );
        case SURFACE_CALLBACK_TYPE_RESIZE:           result( "Surface Resized" );
        case SURFACE_CALLBACK_TYPE_POSITION:         result( "Surface Position Changed" );
        case SURFACE_CALLBACK_TYPE_MOUSE_BUTTON:     result( "Mouse Button Clicked" );
        case SURFACE_CALLBACK_TYPE_MOUSE_MOVE:       result( "Mouse Moved" );
        case SURFACE_CALLBACK_TYPE_MOUSE_MOVE_DELTA: result( "Mouse Moved Delta" );
        case SURFACE_CALLBACK_TYPE_MOUSE_WHEEL:      result( "Mouse Wheel Scrolled" );
        case SURFACE_CALLBACK_TYPE_KEY:              result( "Key Press/Release" );
        case SURFACE_CALLBACK_TYPE_TEXT:             result( "Text Input" );
    }
    result( "Unknown" );
    #undef result
}

#endif /* header guard */
