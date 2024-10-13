/**
 * @file   cstdlib.c
 * @brief  C Standard Library replacement functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   March 18, 2024
*/
#include "media/defines.h"
#include "media/types.h"

attr_clink
void* memcpy(
    void* attr_restrict dst, const void* attr_restrict src, uintptr_t size
) {
    for( uintptr_t i = 0; i < size; ++i ) {
        ((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
    }
    return dst;
}
attr_clink
void* memset( void* dst, int val, uintptr_t size ) {
    for( uintptr_t i = 0; i < size; ++i ) {
        *((int8_t*)dst + i) = val;
    }
    return dst;
}
attr_clink
void* memmove( void* str1, const void* str2, uintptr_t n ) {
    if( !n ) {
        return str1;
    }
    if( str1 < str2 ) {
        return memcpy( str1, str2, n );
    }
    uint8_t* a = str1;
    const uint8_t* b = str2;

    for( uintptr_t i = n; i-- > 0; ) {
        a[i] = b[i];
    }

    return str1;
}
