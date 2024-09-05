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
    void* attr_restrict dst, const void* attr_restrict src, m_uintptr size
) {
    for( m_uintptr i = 0; i < size; ++i ) {
        ((m_uint8*)dst)[i] = ((m_uint8*)src)[i];
    }
    return dst;
}
attr_clink
void* memset( void* dst, int val, m_uintptr size ) {
    for( m_uintptr i = 0; i < size; ++i ) {
        *((m_int8*)dst + i) = val;
    }
    return dst;
}
attr_clink
void* memmove( void* str1, const void* str2, m_uintptr n ) {
    if( !n ) {
        return str1;
    }
    if( str1 < str2 ) {
        return memcpy( str1, str2, n );
    }
    m_uint8* a = str1;
    const m_uint8* b = str2;

    for( m_uintptr i = n; i-- > 0; ) {
        a[i] = b[i];
    }

    return str1;
}
