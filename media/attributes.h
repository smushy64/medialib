#if !defined(MEDIA_INTERNAL_ATTRIBUTES_H)
#define MEDIA_INTERNAL_ATTRIBUTES_H
/**
 * @file   attributes.h
 * @brief  Attributes used internally.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 13, 2024
*/
#include "core/attributes.h" // IWYU pragma: export

#if defined(MEDIA_ENABLE_STATIC_BUILD)
    /// @brief Attribute for Media functions.
    #define attr_media_api attr_clink
#else
    #if defined(MEDIA_ENABLE_EXPORT)
        /// @brief Attribute for Media functions.
        #define attr_media_api attr_export
    #else
        /// @brief Attribute for Media functions.
        #define attr_media_api attr_import
    #endif
#endif

#endif /* header guard */
