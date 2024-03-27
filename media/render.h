#if !defined(MEDIA_RENDER_H)
#define MEDIA_RENDER_H
/**
 * @file   render.h
 * @brief  Rendering related functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 18, 2024
*/
#include "media/types.h"
#include "media/attributes.h"

/// @brief Opaque handle to an OpenGL render context.
typedef void OpenGLRenderContext;
/// @brief Opaque pointer to OpenGL attributes.
typedef void OpenGLAttributes;

/// @brief OpenGL profile types.
typedef enum MediaOpenGLProfile {
    /// @brief OpenGL Core profile.
    MEDIA_OPENGL_PROFILE_CORE,
    /// @brief OpenGL Compatibility profile.
    MEDIA_OPENGL_PROFILE_COMPATIBILITY,
} MediaOpenGLProfile;
/// @brief OpenGL attribute names.
typedef enum MediaOpenGLAttribute {
    /// @brief Bit-size of red channel in color buffer.
    /// @details Default value is 8.
    MEDIA_OPENGL_ATTR_RED_SIZE,
    /// @brief Bit-size of green channel in color buffer.
    /// @details Default value is 8.
    MEDIA_OPENGL_ATTR_GREEN_SIZE,
    /// @brief Bit-size of blue channel in color buffer.
    /// @details Default value is 8.
    MEDIA_OPENGL_ATTR_BLUE_SIZE,
    /// @brief Bit-size of alpha channel in color buffer.
    /// @details Default value is 8.
    MEDIA_OPENGL_ATTR_ALPHA_SIZE,
    /// @brief Bit-size of depth buffer.
    /// @details Default value is 24.
    MEDIA_OPENGL_ATTR_DEPTH_SIZE,
    /// @brief Bit-size of stencil buffer.
    /// @details Default value is 0.
    MEDIA_OPENGL_ATTR_STENCIL_SIZE,
    /// @brief Change requested OpenGL profile.
    /// @details Default value is #MEDIA_OPENGL_PROFILE_CORE 
    /// @see MediaOpenGLProfile
    MEDIA_OPENGL_ATTR_PROFILE,
    /// @brief Set OpenGL major version.
    /// @details Default value is 3.
    MEDIA_OPENGL_ATTR_MAJOR,
    /// @brief Set OpenGL minor version.
    /// @details Default value is 0.
    MEDIA_OPENGL_ATTR_MINOR,
    /// @brief Request double buffering.
    /// @details Default value is @c true
    MEDIA_OPENGL_ATTR_DOUBLE_BUFFER,
    /// @brief Request debug context.
    /// @details Default value is @c false
    MEDIA_OPENGL_ATTR_DEBUG,
    /// @brief Forward compatible context.
    /// @details Default value is @c false
    MEDIA_OPENGL_ATTR_FORWARD_COMPATIBILITY,
} MediaOpenGLAttribute;

/// @brief Create default OpenGL attributes array.
///
/// Must be destroyed with media_render_gl_attr_destroy() to prevent memory leak.
/// @return Pointer to attributes.
attr_media_api OpenGLAttributes* media_render_gl_attr_create(void);
/// @brief Set value of an OpenGL attribute.
/// @param[in] attr Attributes array.
/// @param name Name of attribute to set.
/// @param value Value to set attribute.
/// @return True if value was valid.
/// @see MediaOpenGLAttribute
attr_media_api b32 media_render_gl_attr_set(
    OpenGLAttributes* attr, MediaOpenGLAttribute name, int value );
/// @brief Get value of an OpenGL attribute.
/// @param[in] attr Attributes array.
/// @param name Name of attribute to get.
/// @return Value of requested attribute.
///
/// Returns I32_MAX if name is unrecognized!
attr_media_api int media_render_gl_attr_get(
    OpenGLAttributes* attr, MediaOpenGLAttribute name );
/// @brief Free OpenGL attributes array.
/// @param[in] attr Attributes array to destroy.
attr_media_api void media_render_gl_attr_destroy( OpenGLAttributes* attr );

/// @brief Initialize OpenGL.
///
/// @warning MUST be called before any other OpenGL-related functions!
/// @return True if successful.
attr_media_api b32 media_render_gl_initialize(void);
/// @brief Create an OpenGL render context for surface.
/// @param[in] surface Surface to create OpenGL render context for.
/// @param[in] opt_attributes (optional) Attributes. If NULL, uses default attributes.
/// @return OpenGL render context for provided surface.
/// Returns NULL if failed to create context.
attr_media_api OpenGLRenderContext* media_render_gl_context_create(
    MediaSurface* surface, OpenGLAttributes* opt_attributes );
/// @brief Bind the calling thread's render context to surface.
///
/// Use this function to render to multiple OpenGL surfaces within the same thread.
/// @param[in] surface Surface to set context to.
/// @param[in] glrc OpenGL render context to modify.
/// @return True if successful.
attr_media_api b32 media_render_gl_context_bind(
    MediaSurface* surface, OpenGLRenderContext* glrc );
/// @brief Unbind calling thread's render context.
attr_header
attr_always_inline void media_render_gl_context_unbind(void) {
    (void)media_render_gl_context_bind( NULL, NULL );
}
/// @brief Delete an OpenGL render context.
///
/// MUST be called before deleting surface bound to this render context!
/// @param[in] glrc OpenGL render context to delete.
/// @warning Calling thread should unbind the OpenGL render context before deleting it!
/// @see media_render_gl_unbind_context()
attr_media_api void media_render_gl_context_destroy( OpenGLRenderContext* glrc );
/// @brief Share display lists between OpenGL contexts.
///
/// Display lists include shaders, vertex arrays, textures and buffers.
/// @param[in] a, b OpenGL contexts to share display lists between.
/// @return
/// - @c true if both contexts were created with the same attributes.
/// - @c false if contexts were created with different attributes.
attr_media_api b32 media_render_gl_context_share(
    OpenGLRenderContext* a, OpenGLRenderContext* b );
/// @brief OpenGL function loading procedure.
/// @param[in] function_name Name of function to load.
/// @return Pointer to loaded function.
attr_media_api void* media_render_gl_load_proc( const char* function_name );
/// @brief Swap back/front buffers after drawing finished.
/// @param[in] surface Surface to swap buffers for.
/// @return True if successful.
attr_media_api b32 media_render_gl_swap_buffers( MediaSurface* surface );
/// @brief Set swap interval (Vsync).
///
/// - @c 0 V-sync off.
/// - @c 1 V-sync on.
/// - @c 2 V-sync double buffered on.
/// @param[in] surface Surface to set swap interval for.
/// @param interval Interval to set.
/// @return True if successful.
attr_media_api b32 media_render_gl_swap_interval( MediaSurface* surface, int interval );

#endif /* header guard */
