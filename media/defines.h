#if !defined(MEDIA_DEFINES_H)
#define MEDIA_DEFINES_H
/**
 * @file   defines.h
 * @brief  Media library defines.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   July 25, 2024
*/

#if defined(__cplusplus)
    #define MEDIA_CPLUSPLUS
#endif

// platform defines
#if defined(_WIN32) || defined(_WIN64)
    #undef MEDIA_PLATFORM_WINDOWS
    /// @brief Platform is Windows.
    #define MEDIA_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__gnu_linux__)
    #undef MEDIA_PLATFORM_LINUX
    /// @brief Platform is GNU/Linux.
    #define MEDIA_PLATFORM_LINUX
#elif defined(__ANDROID__)
    #undef MEDIA_PLATFORM_ANDROID
    /// @brief Platform is Android.
    #define MEDIA_PLATFORM_ANDROID
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #undef MEDIA_PLATFORM_IOS
        /// @brief Platform is iOS.
        #define MEDIA_PLATFORM_IOS
    #elif TARGET_OS_MAC
        #undef MEDIA_PLATFORM_MACOS
        /// @brief Platform is MacOS.
        #define MEDIA_PLATFORM_MACOS
    #endif
#else
    #undef MEDIA_PLATFORM_UNKNOWN
    /// @brief Platform is unknown.
    #define MEDIA_PLATFORM_UNKNOWN
#endif

#if defined(__i386__) || defined(_M_IX86)
    #undef MEDIA_ARCH_X86
    /// @brief Architecture is x86.
    #define MEDIA_ARCH_X86
    #undef MEDIA_ARCH_32_BIT
    /// @brief Architecture is 32-bit.
    #define MEDIA_ARCH_32_BIT

    #if !defined(MEDIA_ARCH_LITTLE_ENDIAN)
        #undef MEDIA_ARCH_LITTLE_ENDIAN
        /// @brief Little endian.
        #define MEDIA_ARCH_LITTLE_ENDIAN
    #endif
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #undef MEDIA_ARCH_X86
    /// @brief Architecture is x86.
    #define MEDIA_ARCH_X86

    #undef MEDIA_ARCH_32_BIT
    #undef MEDIA_ARCH_64_BIT
    /// @brief Architecture is 64-bit.
    #define MEDIA_ARCH_64_BIT

    #undef MEDIA_ARCH_LITTLE_ENDIAN
    /// @brief Little endian.
    #define MEDIA_ARCH_LITTLE_ENDIAN
#endif

#if defined(__arm__) || defined(_M_ARM)
    #undef MEDIA_ARCH_ARM
    /// @brief Architecture is ARM.
    #define MEDIA_ARCH_ARM
    #undef MEDIA_ARCH_32_BIT
    /// @brief Architecture is 32-bit.
    #define MEDIA_ARCH_32_BIT
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
    #undef MEDIA_ARCH_ARM
    /// @brief Architecture is ARM.
    #define MEDIA_ARCH_ARM
    #undef MEDIA_ARCH_32_BIT
    #undef MEDIA_ARCH_64_BIT
    /// @brief Architecture is 64-bit.
    #define MEDIA_ARCH_64_BIT
#endif

#if !defined(attr_restrict)
    #if defined( MEDIA_CPLUSPLUS )
        /// @brief C/C++ restrict keyword
        #define attr_restrict __restrict
    #else
        /// @brief C/C++ restrict keyword
        #define attr_restrict restrict
    #endif
#endif

#if !defined(attr_header)
    #if defined( MEDIA_CPLUSPLUS )
        /// @brief Attribute for header-only functions.
        #define attr_header inline
    #else
        /// @brief Attribute for header-only functions.
        #define attr_header static inline
    #endif
#endif

#if !defined(attr_clink)
    #if defined( MEDIA_CPLUSPLUS )
        /// @brief Attribute for functions with C-linkage.
        #define attr_clink extern "C"
    #else
        /// @brief Attribute for functions with C-linkage.
        #define attr_clink extern
    #endif
#endif

#if !defined(attr_export)
    #if defined(MEDIA_PLATFORM_WINDOWS) || defined(MEDIA_COMPILER_MSVC)
        /// @brief Attribute for functions to be exported by dll.
        #define attr_export __declspec(dllexport)
    #else
        /// @brief Attribute for functions to be exported by dll.
        #define attr_export __attribute__((visibility("default")))
    #endif
#endif

#if !defined(attr_import)
    #if defined(MEDIA_PLATFORM_WINDOWS) || defined(MEDIA_COMPILER_MSVC)
        /// @brief Attribute for functions to be imported from dll.
        #define attr_import __declspec(dllimport) attr_clink
    #else
        /// @brief Attribute for functions to be imported from dll.
        #define attr_import attr_clink
    #endif
#endif

#if defined(MEDIA_ENABLE_STATIC_BUILD)
    /// @brief Attribute for Core functions.
    #define attr_media_api attr_clink
#else
    #if defined(MEDIA_ENABLE_EXPORT)
        /// @brief Attribute for Core functions.
        #define attr_media_api attr_export
    #else
        /// @brief Attribute for Core functions.
        #define attr_media_api attr_import
    #endif
#endif

/// @brief Attribute for forcing optimizations for function.
#define attr_optimized __attribute__((__hot__))
/// @brief Attribute for forcing optimizations off for function.
#define attr_optnone __attribute__((__optnone__))

/// @brief Attribute for forcing function to be inlined.
#define attr_always_inline __attribute__((__always_inline__))
/// @brief Attribute for preventing inlining.
#define attr_no_inline __attribute__((__noinline__))

/// @brief Attribute for marking functions as deprecated.
#define attr_deprecate  __attribute__((__deprecated__))

/// @brief Attribute for marking variables/functions as unused.
/// @note Does not do anything on MSVC if C version is < C23.
#define attr_unused __attribute__((__unused__))

/// @brief Attribute for functions that are internal to a translation unit.
#define attr_internal static
/// @brief Attribute for global variables.
#define attr_global static
/// @brief Attribute for static variables.
#define attr_local static

#if !defined(unused)
    attr_header attr_always_inline attr_optimized
    void _unused(int a, ...) { (void)(a); }

    #define unused(...) _unused( 0, __VA_ARGS__ )
#endif

#if !defined(MEDIA_CPLUSPLUS)
    #if !defined(true)
        #define true 1
    #endif
    #if !defined(false)
        #define false 0
    #endif
#endif

#if !defined(NULL)
    #define NULL ((void*)0)
#endif

#if !defined(rcast)
    #define rcast( type, ptr ) ( *(type *)(ptr) )
#endif
#if !defined(rcast_ref)
    #define rcast_ref( type, ptr ) ( (type *)(ptr) )
#endif

#endif /* header guard */
