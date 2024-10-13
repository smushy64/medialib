/**
 * @file   prompt.c
 * @brief  Media Windows Prompts.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 11, 2024
*/
#include "media/defines.h"
#if defined(MEDIA_PLATFORM_WINDOWS)
#include "media/prompt.h"
#include "impl/win32/common.h"

#include <shobjidl.h>

attr_internal uint32_t win32_count_filters( uint32_t filter_len, const char* filter );
attr_internal void win32_clip_filter(
    uint32_t filter_len, const char* filter,
    uint32_t* out_filter_len );
attr_internal _Bool win32_clip_filter_name(
    uint32_t filter_len, const char* filter,
    uint32_t* out_filter_len );
attr_internal COMDLG_FILTERSPEC* win32_make_filters(
    uint32_t filter_len, const char* filter, uint32_t* out_count );
attr_internal void win32_free_filters( COMDLG_FILTERSPEC* filters );

attr_media_api PromptMessageResult prompt_message(
    uint32_t opt_title_len, const char* opt_title,
    uint32_t message_len, const char* message,
    PromptMessageType type, PromptMessageOptions options
) {
    if( !message || !message_len ) {
        win32_error( "prompt_message: did not provide a message!" );
        return PROMPT_MESSAGE_ERROR_UNKNOWN;
    }

    uint32_t wide_buffer_cap = 2;
    if( opt_title && opt_title_len ) {
        wide_buffer_cap +=
            MultiByteToWideChar( CP_UTF8, 0, opt_title, opt_title_len, 0, 0 );
    }
    wide_buffer_cap +=
        MultiByteToWideChar( CP_UTF8, 0, message, message_len, 0, 0 );

    uint32_t wide_buffer_size = sizeof(wchar_t) * wide_buffer_cap;

    void* wide_buffer =
        HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, wide_buffer_size );
    if( !wide_buffer ) {
        win32_error( "prompt_message: failed to allocate wide buffer!" );
        return PROMPT_MESSAGE_ERROR_UNKNOWN;
    }

    wchar_t* wtitle   = 0;
    wchar_t* wmessage = wide_buffer;
    if( opt_title && opt_title_len ) {
        wtitle  = wide_buffer;
        int len = MultiByteToWideChar(
            CP_UTF8, 0, opt_title, opt_title_len, wtitle, wide_buffer_cap );
        len++;
        
        wmessage         = (wchar_t*)wide_buffer + len;
        wide_buffer_cap -= len;
    }

    MultiByteToWideChar(
        CP_UTF8, 0,
        message, message_len,
        wmessage, wide_buffer_cap );

    UINT uType = 0;

    switch( type ) {
        case PROMPT_MESSAGE_TYPE_INFO: {
            if( !wtitle ) {
                wtitle = L"Info";
            }
            uType |= MB_ICONINFORMATION;
        } break;
        case PROMPT_MESSAGE_TYPE_WARN: {
            if( !wtitle ) {
                wtitle = L"Warning";
            }
            uType |= MB_ICONWARNING;
        } break;
        case PROMPT_MESSAGE_TYPE_ERROR: {
            if( !wtitle ) {
                wtitle = L"Error";
            }
            uType |= MB_ICONERROR;
        } break;
    }
    
    switch( options ) {
        case PROMPT_MESSAGE_OPTIONS_OK: {
            uType |= MB_OK;
        } break;
        case PROMPT_MESSAGE_OPTIONS_OK_CANCEL: {
            uType |= MB_OKCANCEL;
        } break;
        case PROMPT_MESSAGE_OPTIONS_YES_NO: {
            uType |= MB_YESNO;
        } break;
    }

    int result = MessageBoxW( NULL, wmessage, wtitle, uType );

    HeapFree( GetProcessHeap(), 0, wide_buffer );

    switch( result ) {
        case IDOK:     return PROMPT_MESSAGE_RESULT_OK_PRESSED;
        case IDCANCEL: return PROMPT_MESSAGE_RESULT_CANCEL_PRESSED;
        case IDYES:    return PROMPT_MESSAGE_RESULT_YES_PRESSED;
        case IDNO:     return PROMPT_MESSAGE_RESULT_NO_PRESSED;
        default: {
            win32_error( "prompt_message: unknown error occurred!" );
            return PROMPT_MESSAGE_ERROR_UNKNOWN;
        } break;
    }
}

attr_media_api PromptFileOpenResult prompt_file_open(
    uint32_t opt_title_len,
    const char* opt_title,
    uint32_t opt_ext_filters_len,
    const char* opt_ext_filters,
    uint32_t result_buffer_cap,
    uint32_t* out_result_len,
    char* out_result_buffer
) {
    wchar_t* title = NULL;
    if( opt_title ) {
        if( opt_title_len ) {
            title = win32_utf8_to_ucs2_alloc( opt_title_len, opt_title, 0 );
            if( !title ) {
                win32_error(
                    "prompt_file_open: failed to allocate title wide buffer!" );
                return PROMPT_FILE_OPEN_ERROR_OUT_OF_MEMORY; 
            }
        } else {
            win32_warn( "prompt_file_open: did not provide length for title!" );
        }
    }
    #define free_title() do {\
        if(title) {\
            HeapFree( GetProcessHeap(), 0, title );\
        }\
    } while(0)

    uint32_t filter_count = 0;
    COMDLG_FILTERSPEC* filter = NULL;
    if( opt_ext_filters ) {
        filter = win32_make_filters(
            opt_ext_filters_len, opt_ext_filters, &filter_count );
        if( !filter && filter_count ) {
            // NOTE(alicia): logging occurs in win32_make_filters
            free_title();
            return PROMPT_FILE_OPEN_ERROR_PARSE_FILTERS;
        }
    }

    IFileOpenDialog* pFileOpen;
    HRESULT hr = CoCreateInstance(
        &CLSID_FileOpenDialog,
        NULL, CLSCTX_ALL, &IID_IFileOpenDialog, (void**)(&pFileOpen) );

    if( !CoCheck(hr) ) {
        win32_error( "prompt_file_open: failed to create file open dialog" );
        free_title();
        return PROMPT_FILE_OPEN_ERROR_UNKNOWN;
    }

    if( filter && filter_count ) {
        hr = pFileOpen->lpVtbl->SetFileTypes( pFileOpen, filter_count, filter );
        if( !CoCheck( hr ) ) {
            win32_error( "prompt_file_open: failed to set extension filters!" );
            pFileOpen->lpVtbl->Release( pFileOpen );
            win32_free_filters( filter );
            free_title();
            return PROMPT_FILE_OPEN_ERROR_UNKNOWN;
        }
    }

    if( title ) {
        hr = pFileOpen->lpVtbl->SetTitle( pFileOpen, title );
        if( !CoCheck( hr ) ) {
            win32_error( "prompt_file_open: failed to set prompt title!" );
            pFileOpen->lpVtbl->Release( pFileOpen );
            win32_free_filters( filter );
            free_title();
            return PROMPT_FILE_OPEN_ERROR_UNKNOWN;
        }
    }

    hr = pFileOpen->lpVtbl->Show(pFileOpen, NULL);
    switch( hr ) {
        case S_OK: break;
        default: {
            PromptFileOpenResult result = PROMPT_FILE_OPEN_ERROR_UNKNOWN;
            pFileOpen->lpVtbl->Release( pFileOpen );
            win32_free_filters( filter );
            free_title();

            if( HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr ) {
                result = PROMPT_FILE_OPEN_RESULT_CANCELED;
            } else {
                win32_error( "prompt_file_open: failed to show file open dialog!" );
            }
            return result;
        } break;
    }

    IShellItem* pItem = NULL;
    hr = pFileOpen->lpVtbl->GetResult( pFileOpen, &pItem );

    if( !CoCheck(hr) ) {
        win32_error( "prompt_file_open: failed to get result from file open dialog!" );
        pFileOpen->lpVtbl->Release( pFileOpen );
        win32_free_filters( filter );
        free_title();
        return PROMPT_FILE_OPEN_ERROR_UNKNOWN;
    }

    PWSTR pszFilePath;
    hr = pItem->lpVtbl->GetDisplayName( pItem, SIGDN_FILESYSPATH, &pszFilePath );

    if( !CoCheck( hr ) ) {
        win32_error( "prompt_file_open: failed to get path!" );
        pItem->lpVtbl->Release( pItem );
        pFileOpen->lpVtbl->Release( pFileOpen );
        win32_free_filters( filter );
        free_title();
        return PROMPT_FILE_OPEN_ERROR_UNKNOWN;
    }

    int len = WideCharToMultiByte(
        CP_UTF8, 0,
        pszFilePath, -1,
        out_result_buffer, result_buffer_cap,
        0, 0 ) - 1;

    PromptFileOpenResult result = PROMPT_FILE_OPEN_RESULT_SUCCESS;
    if( len < 1 ) {
        win32_error( "prompt_file_open: failed to convert path from UCS-2 to UTF-8!" );
        result = PROMPT_FILE_OPEN_ERROR_UNKNOWN;
    } else {
        *out_result_len = len;
    }

    free_title();
    CoTaskMemFree( pszFilePath );
    pItem->lpVtbl->Release( pItem );
    pFileOpen->lpVtbl->Release( pFileOpen );

    win32_free_filters( filter );

    #undef free_title
    return result;
}
attr_internal uint32_t win32_count_filters( uint32_t filter_len, const char* filter ) {
    if( !filter_len ) {
        return 0;
    }
    uint32_t res = 0;
    for( uint32_t i = 0; i < filter_len; ++i ) {
        if( filter[i] == ';' ) {
            res++;
        }
    }
    return res ? res : 1;
}
attr_internal void win32_clip_filter(
    uint32_t filter_len, const char* filter,
    uint32_t* out_filter_len
) {
    for( uint32_t i = 0; i < filter_len; ++i ) {
        if( filter[i] == ';' ) {
            *out_filter_len = i;
            return;
        }
    }

    *out_filter_len = filter_len;
}
attr_internal _Bool win32_clip_filter_name(
    uint32_t filter_len, const char* filter,
    uint32_t* out_filter_len
) {
    for( uint32_t i = 0; i < filter_len; ++i ) {
        if( filter[i] == ':' ) {
            *out_filter_len = i;
            return true;
        }
    }

    return false;
}

attr_internal COMDLG_FILTERSPEC* win32_make_filters(
    uint32_t filter_len, const char* filter, uint32_t* out_count
) {
    uint32_t filter_count = win32_count_filters( filter_len, filter );
    if( !filter_count ) {
        win32_warn( "prompt_file_open: couldn't count filters!" );
        return NULL;
    }

    uint32_t wide_filter_len =
        MultiByteToWideChar( CP_UTF8, 0, filter, filter_len, 0, 0 );

    uint32_t wide_buf_cap = wide_filter_len + 8;
    uint32_t buffer_size =
        (sizeof(wchar_t) * wide_buf_cap) +
        (sizeof(COMDLG_FILTERSPEC) * filter_count);
    void* buffer = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, buffer_size );
    if( !buffer ) {
        *out_count = filter_count;
        win32_error( "prompt_file_open: couldn't allocate file filter buffer!" );
        return NULL;
    }

    COMDLG_FILTERSPEC* filters  = buffer;
    wchar_t* filter_text_buffer = (wchar_t*)(filters + filter_count);

    uint32_t    filter_slice_len = filter_len;
    const char* filter_slice     = filter;

    uint32_t final_count = 0;
    for( uint32_t i = 0; i < filter_count; ++i ) {
        if( !(filter_slice_len && wide_buf_cap) ) {
            break;
        }

        uint32_t current_slice_len = 0;
        const char* current_filter_start = 0;
        uint32_t current_filter_name_len = 0;

        win32_clip_filter( 
            filter_slice_len, filter_slice,
            &current_slice_len );

        if( win32_clip_filter_name(
            current_slice_len, filter_slice,
            &current_filter_name_len
        ) ) {
            if(
                !current_filter_name_len ||
                !( current_slice_len - current_filter_name_len )
            ) {
                goto win32_make_filters_skip_filter;
            }

            current_filter_start = 
                filter_slice + current_filter_name_len + 1;
            uint32_t current_filter_len = 
                current_slice_len - (current_filter_name_len + 1);
            if( !current_filter_len ) {
                goto win32_make_filters_skip_filter;
            }

            int name_len = MultiByteToWideChar(
                CP_UTF8, 0,
                filter_slice, current_filter_name_len,
                filter_text_buffer, wide_buf_cap );

            filter_text_buffer[name_len++] = 0;
            filters[i].pszName = filter_text_buffer;

            filter_text_buffer += name_len;
            wide_buf_cap       += name_len;

            if( !wide_buf_cap ) {
                break;
            }

            int len = MultiByteToWideChar(
                CP_UTF8, 0,
                current_filter_start, current_filter_len,
                filter_text_buffer, wide_buf_cap );
            filter_text_buffer[len++] = 0;
            filters[i].pszSpec = filter_text_buffer;

            filter_text_buffer += len;
            wide_buf_cap       += len;
        } else {
            int len = MultiByteToWideChar(
                CP_UTF8, 0,
                filter_slice, current_slice_len,
                filter_text_buffer, wide_buf_cap );

            filter_text_buffer[len++] = 0;
            filters[i].pszName = filter_text_buffer;
            filters[i].pszSpec = filter_text_buffer;

            filter_text_buffer += len;
            wide_buf_cap       -= len;
        }

        final_count++;

win32_make_filters_skip_filter:
        filter_slice_len -= current_slice_len + 1;
        filter_slice     += current_slice_len + 1;
    }

    if( final_count != filter_count ) {
        win32_warn(
            "promp_file_open:"
            "failed to include every file filter requested! "
            "possibly malformed filter?" );
    }

    *out_count = final_count;
    return filters;
}
attr_internal void win32_free_filters( COMDLG_FILTERSPEC* filters ) {
    if( filters ) {
        HeapFree( GetProcessHeap(), 0, filters );
    }
}

#endif /* Platform Windows */

