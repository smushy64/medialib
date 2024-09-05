#if !defined(MEDIA_OPENGL_H)
#define MEDIA_OPENGL_H
/**
 * @file   opengl.h
 * @brief  OpenGL related functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 18, 2024
*/
#include "media/defines.h"
#include "media/types.h"

/// @brief Opaque handle to an OpenGL render context.
typedef void OpenGLRenderContext;

/// @brief Default major version of OpenGL.
#define OPENGL_DEFAULT_MAJOR_VERSION (3)
/// @brief Default minor version of OpenGL.
#define OPENGL_DEFAULT_MINOR_VERSION (2)

/// @brief OpenGL profile types.
typedef enum OpenGLProfile {
    /// @brief OpenGL Core profile.
    OPENGL_PROFILE_CORE,
    /// @brief OpenGL Compatibility profile.
    OPENGL_PROFILE_COMPATIBILITY,
} OpenGLProfile;
/// @brief OpenGL attribute names.
typedef enum OpenGLAttribute {
    /// @brief Bit-size of red channel in color buffer.
    /// @details Default value is 8.
    OPENGL_ATTR_RED_SIZE,
    /// @brief Bit-size of green channel in color buffer.
    /// @details Default value is 8.
    OPENGL_ATTR_GREEN_SIZE,
    /// @brief Bit-size of blue channel in color buffer.
    /// @details Default value is 8.
    OPENGL_ATTR_BLUE_SIZE,
    /// @brief Bit-size of alpha channel in color buffer.
    /// @details Default value is 8.
    OPENGL_ATTR_ALPHA_SIZE,
    /// @brief Bit-size of depth buffer.
    /// @details Default value is 24.
    OPENGL_ATTR_DEPTH_SIZE,
    /// @brief Bit-size of stencil buffer.
    /// @details Default value is 0.
    OPENGL_ATTR_STENCIL_SIZE,
    /// @brief Change requested OpenGL profile.
    /// @details Default value is #OPENGL_PROFILE_CORE 
    /// @see OpenGLProfile
    OPENGL_ATTR_PROFILE,
    /// @brief Set OpenGL major version.
    /// @details Default value is 3.
    OPENGL_ATTR_MAJOR,
    /// @brief Set OpenGL minor version.
    /// @details Default value is 2.
    OPENGL_ATTR_MINOR,
    /// @brief Request double buffering.
    /// @details Default value is @c true
    OPENGL_ATTR_DOUBLE_BUFFER,
    /// @brief Request debug context.
    /// @details Default value is @c false
    OPENGL_ATTR_DEBUG,
    /// @brief Forward compatible context.
    /// @details Default value is @c false
    OPENGL_ATTR_FORWARD_COMPATIBILITY,
} OpenGLAttribute;
/// @brief OpenGL attributes.
typedef struct { m_uint8 raw[sizeof(int) * 16]; } OpenGLAttributeList;
/// @brief Create default OpenGL attributes array.
///
/// Must be destroyed with opengl_attr_destroy() to prevent memory leak.
/// @return Pointer to attributes.
attr_media_api OpenGLAttributeList opengl_attr_create(void);
/// @brief Set value of an OpenGL attribute.
/// @param[in] attr Attributes array.
/// @param name Name of attribute to set.
/// @param value Value to set attribute.
/// @return True if value was valid.
/// @see OpenGLAttribute
attr_media_api m_bool32 opengl_attr_set(
    OpenGLAttributeList* attr, OpenGLAttribute name, int value );
/// @brief Get value of an OpenGL attribute.
/// @param[in] attr Attributes array.
/// @param     name Name of attribute to get.
/// @return 
///     - positive: Value of requested attribute.
///     - negative: @c name is not recognized.
attr_media_api m_int32 opengl_attr_get(
    OpenGLAttributeList* attr, OpenGLAttribute name );

/// @brief Initialize OpenGL.
///
/// @warning MUST be called before any other OpenGL-related functions!
/// @return True if successful.
attr_media_api m_bool32 opengl_initialize(void);
/// @brief Create an OpenGL render context for surface.
/// @param[in] surface Surface to create OpenGL render context for.
/// @param[in] opt_attributes (optional) Attributes. If NULL, uses default attributes.
/// @return OpenGL render context for provided surface.
/// Returns NULL if failed to create context.
attr_media_api OpenGLRenderContext* opengl_context_create(
    SurfaceHandle* surface, OpenGLAttributeList* opt_attributes );
/// @brief Bind the calling thread's render context to surface.
///
/// Use this function to render to multiple OpenGL surfaces within the same thread.
/// @param[in] surface Surface to set context to.
/// @param[in] glrc OpenGL render context to modify.
/// @return True if successful.
attr_media_api m_bool32 opengl_context_bind(
    SurfaceHandle* surface, OpenGLRenderContext* glrc );
/// @brief Unbind calling thread's render context.
attr_header
attr_always_inline void opengl_context_unbind(void) {
    (void)opengl_context_bind( NULL, NULL );
}
/// @brief Delete an OpenGL render context.
///
/// MUST be called before deleting surface bound to this render context!
/// @param[in] glrc OpenGL render context to delete.
/// @warning Calling thread should unbind the OpenGL render context before deleting it!
/// @see opengl_unbind_context()
attr_media_api void opengl_context_destroy( OpenGLRenderContext* glrc );
/// @brief Share display lists between OpenGL contexts.
///
/// Display lists include shaders, vertex arrays, textures and buffers.
/// @param[in] a, b OpenGL contexts to share display lists between.
/// @return
/// - @c true if both contexts were created with the same attributes.
/// - @c false if contexts were created with different attributes.
attr_media_api m_bool32 opengl_context_share(
    OpenGLRenderContext* a, OpenGLRenderContext* b );
/// @brief OpenGL function loading procedure.
/// @param[in] function_name Name of function to load.
/// @return Pointer to loaded function.
attr_media_api void* opengl_load_proc( const char* function_name );
/// @brief Swap back/front buffers after drawing finished.
/// @param[in] surface Surface to swap buffers for.
/// @return True if successful.
attr_media_api m_bool32 opengl_swap_buffers( SurfaceHandle* surface );
/// @brief Set swap interval (Vsync).
///
/// - @c 0 V-sync off.
/// - @c 1 V-sync on.
/// - @c 2 V-sync double buffered on.
/// @param[in] surface Surface to set swap interval for.
/// @param interval Interval to set.
/// @return True if successful.
attr_media_api m_bool32 opengl_swap_interval(
    SurfaceHandle* surface, int interval );

#endif /* header guard */
