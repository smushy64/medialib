/**
 * @file   cbuild.c
 * @brief  Media library build system.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   September 10, 2024
*/
#include "cbuild.h"
#include <unistd.h>

#define MEDIA_LIB_VERSION_MAJOR 0
#define MEDIA_LIB_VERSION_MINOR 1
#define MEDIA_LIB_VERSION_PATCH 1

#define ARGS_OPT    "-O2"
#define ARGS_NO_OPT "-O0"

#define ARGS_WARN "-Wall", "-Wextra", "-Werror=vla", "-Werror"

#if defined(PLATFORM_WINDOWS)
    #define SO_EXT  ".dll"
    #define EXE_EXT ".exe"

    #define ARGS_WITH_SYMBOLS_STATIC "-g"
    #define ARGS_WITH_SYMBOLS        "-g", "-gcodeview", "-fuse-ld=lld", "-Wl,/debug"

    #define ARGS_LINK "-lkernel32", "-nostdlib"

    #define ARGS_LD "-shared"
#else
    #define SO_EXT  ".so"
    #define EXE_EXT ""

    #define ARGS_WITH_SYMBOLS_STATIC "-ggdb"
    #define ARGS_WITH_SYMBOLS        "-ggdb"
    
    #define ARGS_LINK "-lSDL3"

    #define ARGS_LD   "-fPIC", "-shared"
#endif

#define STATIC_EXT ".o"

#define TEST_PATH "./build/libmedia-test" EXE_EXT

typedef enum Mode {
    M_HELP,
    M_BUILD,
    M_TEST,
    M_DOCS,
    M_LSP,

    M_COUNT
} Mode;
String mode_to_str( Mode mode );
bool mode_from_str( String str, Mode* out_mode );
String mode_description( Mode mode );

typedef enum Target {
    T_NATIVE,

    T_COUNT
} Target;
String target_to_str( Target target );
bool target_from_str( String str, Target* out_target );

typedef enum Platform {
    P_WINDOWS,
    P_LINUX,
    P_MACOS,

    P_UNKNOWN
} Platform;
Platform platform_current(void);
String platform_friendly_name( Platform platform );
String platform_name( Platform platform );

typedef struct ParsedArgs {
    Mode mode;
    union {
        struct HelpArgs {
            Mode mode;
        } help;
        struct BuildArgs {
            const char* name;
            const char* output;
            Target      target;
            bool        release;
            bool        strip_symbols;
            bool        is_static;
            bool        dry;
        } build;
        struct TestArgs {
            struct BuildArgs build;
            int          start;
            int          argc;
            const char** argv;
        } test;
        struct DocsArgs {
            struct BuildArgs build;
            bool             launch_browser;
        } docs;
        struct LspArgs {
            struct BuildArgs build;
        } lsp;
    };
} ParsedArgs;

int mode_help( ParsedArgs* args );
int mode_build( struct BuildArgs* args, CommandBuilder* opt_out_builder );
int mode_test( struct TestArgs* args );
int mode_docs( struct DocsArgs* args );
int mode_lsp( struct LspArgs* args );

int main( int argc, const char** argv ) {
    init( LOGGER_LEVEL_INFO );

    if( argc <= 1 ) {
        mode_help( NULL );
        return 0;
    }

    ParsedArgs parsed_args;
    memory_zero( &parsed_args, sizeof(parsed_args) );

    String arg = string_from_cstr( argv[1] );
    if( !mode_from_str( arg, &parsed_args.mode ) ) {
        cb_error( "unrecognized mode '%s'", arg.cc );
        mode_help( NULL );
        return 1;
    }

    if( parsed_args.mode == M_HELP && argc >= 2 ) {
        arg = string_from_cstr( argv[2] );
        mode_from_str( arg, &parsed_args.help.mode );

        mode_help( &parsed_args );
        return 0;
    }

    bool break_loop = false;
    for( int i = 2; i < argc && !break_loop; ++i ) {
        arg = string_from_cstr( argv[i] );

        switch( parsed_args.mode ) {
            case M_BUILD:
            case M_TEST:
            case M_DOCS:
            case M_LSP: {
                if( string_cmp( string_text("-t"), arg ) ) {
                    i++;
                    if( i >= argc ) {
                        cb_error( "argument '-t' requires a target name after it!" );
                        mode_help( &parsed_args );
                        return 1;
                    }
                    arg = string_from_cstr( argv[i] );
                    if( !target_from_str( arg, &parsed_args.build.target ) ) {
                        cb_error( "unrecognized target '%s'", arg.cc );
                        mode_help( &parsed_args );
                        return 1;
                    }
                    continue;
                }
                if( string_cmp( string_text("-static"), arg ) ) {
                    parsed_args.build.is_static = true;
                    continue;
                }
            } break;

            case M_HELP:
            case M_COUNT: break;
        }

        switch( parsed_args.mode ) {
            case M_BUILD:
            case M_TEST: {
                if( string_cmp( string_text("-o"), arg ) ) {
                    i++;
                    if( i >= argc ) {
                        cb_error( "argument '-o' requires a directory after it!" );
                        mode_help( &parsed_args );
                        return 1;
                    }
                    parsed_args.build.output = argv[i];
                    continue;
                }
                if( string_cmp( string_text("-release"), arg ) ) {
                    parsed_args.build.release = true;
                    continue;
                }
                if( string_cmp( string_text("-no-symbols"), arg ) ) {
                    parsed_args.build.strip_symbols = true;
                    continue;
                }
                if( string_cmp( string_text("-dry"), arg ) ) {
                    parsed_args.build.dry = true;
                    continue;
                }
            } break;

            case M_DOCS:
            case M_LSP:
            case M_HELP:
            case M_COUNT: break;
        }

        switch( parsed_args.mode ) {
            case M_DOCS: {
                if( string_cmp( string_text("-browser"), arg ) ) {
                    parsed_args.docs.launch_browser = true;
                    continue;
                }
            } break;

            case M_TEST: {
                if( string_cmp( string_text( "--" ), arg ) ) {
                    parsed_args.test.start = i + 1;
                    parsed_args.test.argc  = argc;
                    parsed_args.test.argv  = argv;
                    break_loop = true;
                    continue;
                }
            } break;

            case M_BUILD: {
                if( string_cmp( string_text("-n"), arg ) ) {
                    i++;
                    if( i >= argc ) {
                        cb_error( "argument '-n' requires a string after it!" );
                        mode_help( &parsed_args );
                        return 1;
                    }
                    parsed_args.build.name = argv[i];
                    continue;
                }
            } break;

            case M_HELP:
            case M_LSP:
            case M_COUNT:break;
        }

        cb_error( "unrecognized argument '%s'", arg.cc );
        mode_help( &parsed_args );
        return 1;
    }

    switch( parsed_args.mode ) {
        case M_BUILD : return mode_build( &parsed_args.build, NULL );
        case M_TEST  : return mode_test( &parsed_args.test );
        case M_DOCS  : return mode_docs( &parsed_args.docs );
        case M_LSP   : return mode_lsp( &parsed_args.lsp );

        case M_HELP  :
        case M_COUNT : unreachable(); break;
    }
}
DString* path_join( const char* a, const char* b ) {
    const char* path1 = a;
    if( !path1 ) {
        path1 = "";
    }
    const char* path2 = b;
    if( !path2 ) {
        path2 = "";
    }
    usize path1_len = strlen( path1 );
    usize path2_len = strlen( path2 );

    usize cap = path1_len + path2_len + 16;
    DString* result = dstring_empty( cap );
    expect( result, "failed to create path!");

    dstring_append( result, string_new( path1_len, path1 ) );
    if( !path1_len || path1[path1_len - 1] != '/' ) {
        dstring_push( result, '/' );
    }
    return dstring_append( result, string_new( path2_len, path2 ) );
}
bool mode_build_generate_command_line( const char* cmd_line ) {
    cb_info( "build: generating command line . . ." );
    if( !path_exists( "./generated") ) {
        if(!dir_create( "./generated")) {
            cb_error( "build: failed to create ./generated dir!" );
            return false;
        }
    }

    const char* path = "./generated/medialib_command_line.c";

    FileOpenFlags flags = FOPEN_WRITE;
    if( path_exists( path ) ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    FD fd;
    if( !fd_open( path, flags, &fd ) ) {
        cb_error( "build: failed to open %s!", path );
        return false;
    }

    #define write( args... )\
        fd_write_fmt( &fd, args )

    write( "/* generated command line for media lib. */\n" );
    write( "const char external_media_library_command_line[] =\"" );
    write( "%s", cmd_line );
    write( "\";\n" );

    write( "unsigned int external_media_library_command_line_len " );
    write( "= sizeof(external_media_library_command_line) - 1;\n");

    fd_close( &fd );
    cb_info( "build: generated command line at '%s'", path );
    #undef write
    return true;
}
int mode_build( struct BuildArgs* args, CommandBuilder* opt_out_builder ) {
    f64 start = timer_milliseconds();

    // NOTE(alicia): finalized output path is generated here.
    args->output = path_join(
        args->output ? args->output : "./build",
        args->name ? args->name : "libmedia" );

    args->output = dstring_append(
        (DString*)args->output,
        args->is_static ? string_text(STATIC_EXT) : string_text(SO_EXT) );

    CommandBuilder builder;
    expect(
        command_builder_new( "clang", &builder ),
        "failed to create command builder!" );

    command_builder_append(
        &builder, "-std=c11", "-include",
        "generated/medialib_command_line.c",
        "-xc", "impl/sources.h",
        ARGS_WARN );

    if( args->is_static ) {
        command_builder_append( &builder, "-c", "-o", args->output );
    } else {
        command_builder_append( &builder, "-o", args->output );
    }

    if( args->release ) {
        command_builder_append( &builder, ARGS_OPT );
    } else {
        command_builder_append( &builder, ARGS_NO_OPT, "-DMEDIA_ENABLE_LOGGING" );
    }

    if( args->strip_symbols ) {
    } else {
        if( args->is_static ) {
            command_builder_append( &builder, ARGS_WITH_SYMBOLS_STATIC );
        } else {
            command_builder_append( &builder, ARGS_WITH_SYMBOLS );
        }
    }

    if( args->is_static ) {
        command_builder_append( &builder, "-DMEDIA_ENABLE_STATIC_BUILD" );
    } else {
        command_builder_append( &builder, "-DMEDIA_ENABLE_EXPORT", ARGS_LINK );
        command_builder_append( &builder, ARGS_LD );
    }

    command_builder_append(
        &builder,
        "-I.",
        "-DMEDIA_LIB_VERSION_MAJOR=" macro_value_to_string(MEDIA_LIB_VERSION_MAJOR),
        "-DMEDIA_LIB_VERSION_MINOR=" macro_value_to_string(MEDIA_LIB_VERSION_MINOR),
        "-DMEDIA_LIB_VERSION_PATCH=" macro_value_to_string(MEDIA_LIB_VERSION_PATCH) );

    // TODO(alicia): use target flags.

    Command cmd = command_builder_cmd( &builder );

    DString* flat = command_flatten_dstring( &cmd );
    if( args->dry ) {
        cb_info( "build: %s", flat );
        dstring_free( flat );

        if( opt_out_builder ) {
            *opt_out_builder = builder;
        } else {
            command_builder_free( &builder );
        }
        return 0;
    }

    if( !mode_build_generate_command_line( flat ) ) {
        dstring_free( flat );
        command_builder_free(&builder);
        return 1;
    }

    dstring_free( flat );

    if( !process_in_path( "clang" ) ) {
        cb_error( "build: could not find clang in path!" );
        command_builder_free( &builder );
        return 1;
    }

    PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
    int res = process_wait( pid );

    f64 end = timer_milliseconds();
    cb_info( "build: compilation took %.2fms", end - start );

    if( opt_out_builder ) {
        *opt_out_builder = builder;
    } else {
        command_builder_free( &builder );
    }

    return res;
}
int mode_test( struct TestArgs* args ) {
    CommandBuilder builder;
    memory_zero( &builder, sizeof(builder) );

    args->build.name = "libmedia-test";

    int res = mode_build( &args->build, &builder );
    if( res ) {
        command_builder_free( &builder );
        return res;
    }

    DString* flat = NULL;

    command_builder_clear( &builder );
    command_builder_push( &builder, "clang" );

    command_builder_append( &builder, "-std=c11", "./tests/test.c", "-I.", ARGS_WARN );

    if( args->build.is_static ) {
        command_builder_append(
            &builder, "./build/libmedia-test" STATIC_EXT,
            "-DMEDIA_ENABLE_STATIC_BUILD" );
    } else {
        command_builder_append( &builder, "-L./build", "-lmedia-test" );
    }

    if( args->build.release ) {
        command_builder_append( &builder, ARGS_OPT );
    } else {
        command_builder_append( &builder, ARGS_NO_OPT, "-DMEDIA_ENABLE_LOGGING" );
    }

    if( args->build.strip_symbols ) {
    } else {
        if( args->build.is_static ) {
            command_builder_append( &builder, ARGS_WITH_SYMBOLS_STATIC );
        } else {
            command_builder_append( &builder, ARGS_WITH_SYMBOLS );
        }
    }

    if( args->build.is_static ) {
        command_builder_append( &builder, ARGS_LINK );
    } else {
    }

#if !defined(PLATFORM_WINDOWS)
    command_builder_append( &builder, "-lm" );
#endif

    Command cmd = command_builder_cmd( &builder );
    if( args->build.dry ) {
        flat = command_flatten_dstring( &cmd );
        cb_info( "test: %s", flat );
        dstring_free( flat );
    } else {
        PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
        res = process_wait( pid );
        if( res ) {
            cb_error( "test: failed to compile test program!" );
            command_builder_free( &builder );
            return res;
        }
    }

    command_builder_clear( &builder );
    command_builder_push( &builder, TEST_PATH );

    if( args->argv ) {
        command_builder_append_list(
            &builder, (usize)(args->argc - args->start), args->argv + args->start );
    }

    cmd = command_builder_cmd( &builder );
    if( args->build.dry ) {
        flat = command_flatten_dstring( &cmd );
        cb_info( "test: %s", flat );
        dstring_free( flat );
        return 0;
    }

    PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
    res = process_wait( pid );

    cb_info( "test: exited with code %i", res );

    return 0;
}
int mode_docs( struct DocsArgs* args ) {
    if( !process_in_path( "doxygen" ) ) {
        cb_error(
            "docs: doxygen was not found in path! "
            "cannot generate docs without doxygen!" );
        return 1;
    }

    cb_info( "docs: generating doxygen settings for current platform . . ." );

    DString* local_settings = dstring_empty( kibibytes(4) );
    expect( local_settings, "docs: failed to allocate doxygen settings!" );

    #define write( args... ) do {\
        const char* string = local_fmt( args );\
        dstring_append_cstr( local_settings, string );\
    } while(0)

    write( "PREDEFINED += " );
    write( "MEDIA_LIB_VERSION_MAJOR=" macro_value_to_string(MEDIA_LIB_VERSION_MAJOR) " " );
    write( "MEDIA_LIB_VERSION_MINOR=" macro_value_to_string(MEDIA_LIB_VERSION_MINOR) " " );
    write( "MEDIA_LIB_VERSION_PATCH=" macro_value_to_string(MEDIA_LIB_VERSION_PATCH) " " );

    if( args->build.is_static ) {
        write( "MEDIA_ENABLE_STATIC_BUILD " );
    }
    write( "MEDIA_ENABLE_LOGGING __clang__ " );

    switch( args->build.target ) {
        case T_NATIVE: {
            switch( platform_current() ) {
                case P_WINDOWS: {
                    write( "_WIN32 " );
                } break;
                case P_LINUX: {
                    write( "__linux__ " );
                } break;
                case P_MACOS: {
                    write( "__APPLE__ TARGET_OS_MAC " );
                } break;
                case P_UNKNOWN: break;
            }
#if defined(ARCH_64BIT)

    #if defined(PLATFORM_WINDOWS) && defined(PLATFORM_MINGW)
            write( "__MINGW64__ " );
    #endif /* Mingw64 */

    #if defined(ARCH_X86)
            write( "__x86_64__ " );
    #elif defined(ARCH_ARM)
            write( "__aarch64__ " );
    #endif

#else /* Arch 64-bit */

    #if defined(PLATFORM_WINDOWS) && defined(PLATFORM_MINGW)
            write( "__MINGW32__ " );
    #endif /* Mingw32 */

    #if defined(ARCH_X86)
            write( "__i386__ " );
    #elif defined(ARCH_ARM)
            write( "__arm__ " );
    #endif

#endif /* Arch 32-bit */
        } break;

        case T_COUNT: break;
    }

    write( 
        "\nPROJECT_NUMBER = %i.%i.%i",
        MEDIA_LIB_VERSION_MAJOR, MEDIA_LIB_VERSION_MINOR, MEDIA_LIB_VERSION_PATCH );

    const char* settings_path = "./docs/Doxyfile_generated";
    if( path_exists( settings_path ) ) {
        expect(
            file_remove( settings_path ),
            "docs: failed to remove previous generated doxygen settings!" );
    }

    expect(
        file_copy( settings_path, "./docs/Doxyfile_default" ),
        "docs: failed to copy default doxygen settings!" );

    FD fd;
    expect(
        fd_open( settings_path, FOPEN_WRITE, &fd ),
        "docs: failed to open doxygen settings!");

    fd_seek( &fd, FSEEK_END, 0 );
    bool success = fd_write(
        &fd, dstring_len( local_settings ), local_settings, NULL );
    fd_close( &fd );
    dstring_free( local_settings );

    expect( success, "docs: failed to write to generated doxygen settings!" );

    cb_info( "docs: generated doxygen settings at path '%s'!", settings_path );

    Command cmd = command_new( "doxygen", "Doxyfile_generated", "-q" );

    cb_info( "docs: generating documentation . . ." );

    chdir( "docs" );
    PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
    chdir( ".." );

    int res = process_wait( pid );
    if( res ) {
        cb_error( "docs: doxygen exited with code %i", res );
        return res;
    }

    const char* docpath = "./docs/html/index.html";
    cb_info( "docs: documentation generated at '%s'", docpath );

    if( args->launch_browser ) {
        cb_info( "docs: searching for browser to open docs . . ." );
        bool process_found = false;
        Command browser_cmd;
#if defined(PLATFORM_WINDOWS)
        if( process_in_path("pwsh")) {
            browser_cmd = command_new( "pwsh", "-Command", "Invoke-Expression", docpath );
            process_found = true;
            cb_info( "docs: using shell to launch browser" );
        }
#endif
        if( !process_found ) {
            if( process_in_path( "firefox" ) ) {
                browser_cmd   = command_new( "firefox", "./docs/html/index.html" );
                process_found = true;
                cb_info( "docs: using firefox to open docs" );
            } else if( process_in_path( "chromium" ) ) {
                browser_cmd   = command_new( "chromium", "./docs/html/index.html" );
                process_found = true;
                cb_info( "docs: using chromium to open docs" );
            } else if( process_in_path( "google-chrome" ) ) {
                browser_cmd   = command_new( "google-chrome", "./docs/html/index.html" );
                process_found = true;
                cb_info( "docs: using google-chrome to open docs" );
            }
        }

        if( !process_found ) {
            cb_warn( "docs: no browser found!" );
            return 0;
        }

        pid = process_exec( browser_cmd, true, NULL, NULL, NULL, NULL );
        process_discard( pid );
    }

    #undef write
    return 0;
}
int mode_lsp( struct LspArgs* args ) {
    DString* temp = dstring_empty( kibibytes(1) );
    expect( temp, "lsp: failed to allocate buffer!" );

    #define push( str ) dstring_append_cstr( temp, str "\n" )

    push( "clang" );
    push( "-std=c11" );
    push( "-I.." );
    push( "-Wall\n-Wextra" );
    push( "-D_CLANGD" );
    push( "-DMEDIA_LIB_VERSION_MAJOR=" macro_value_to_string(MEDIA_LIB_VERSION_MAJOR) );
    push( "-DMEDIA_LIB_VERSION_MINOR=" macro_value_to_string(MEDIA_LIB_VERSION_MINOR) );
    push( "-DMEDIA_LIB_VERSION_PATCH=" macro_value_to_string(MEDIA_LIB_VERSION_PATCH) );
    push( "-DMEDIA_ENABLE_LOGGING" );

    usize len = dstring_len( temp );

    cb_info( "lsp: generating ./media/ compile flags . . ." );

    if( args->build.is_static ) {
        push( "-DMEDIA_ENABLE_STATIC_BUILD" );
    } else {
        push( "-DMEDIA_ENABLE_EXPORT" );
    }

    FD fd;
    FileOpenFlags flags = FOPEN_WRITE;

    const char* path = "./media/compile_flags.txt";
    if( path_exists( path ) ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    expect(
        fd_open( path, flags, &fd ),
        "lsp: failed to open %s!", path );

    fd_write( &fd, dstring_len(temp), temp, 0 );
    fd_close( &fd );

    cb_info( "lsp: \tgenerated %s compile flags", path );
    dstring_truncate( temp, len );

    cb_info( "lsp: generating ./tests/ compile flags . . ." );

    if( args->build.is_static ) {
        push( "-DMEDIA_ENABLE_STATIC_BUILD" );
    }

    flags = FOPEN_WRITE;

    path = "./tests/compile_flags.txt";
    if( path_exists( path ) ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    expect(
        fd_open( path, flags, &fd ),
        "lsp: failed to open %s!", path );

    fd_write( &fd, dstring_len(temp), temp, 0 );
    fd_close( &fd );

    cb_info( "lsp: \tgenerated %s compile flags", path );
    dstring_truncate( temp, len );

    cb_info( "lsp: generating ./impl/ compile flags . . ." );

    if( args->build.is_static ) {
        push( "-DMEDIA_ENABLE_STATIC_BUILD" );
    } else {
        push( "-DMEDIA_ENABLE_EXPORT" );
    }

    flags = FOPEN_WRITE;

    path = "./impl/compile_flags.txt";
    if( path_exists( path ) ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    expect(
        fd_open( path, flags, &fd ),
        "lsp: failed to open %s!", path );

    fd_write( &fd, dstring_len(temp), temp, 0 );
    fd_close( &fd );

    cb_info( "lsp: \tgenerated %s compile flags", path );
    dstring_truncate( temp, len );

    #undef push
    return 0;
}
int mode_help( ParsedArgs* args ) {
    Mode mode = args ? (args->mode == M_HELP ? args->help.mode : args->mode) : M_HELP;
    printf( "OVERVIEW:    Build system for media lib.\n");
    printf( "USAGE:       ./cbuild %s [args]\n", mode == M_HELP ? "<mode>" : mode_to_str(mode).cc );
    printf( "DESCRIPTION:\n");
    printf( "  %s\n", mode_description( mode ).cc );
    printf( "ARGUMENTS:\n");
    switch( mode ) {
        case M_HELP: {
            printf( "  <mode>       Mode to run cbuild in.\n");
            printf( "                 valid: " );
            for( Mode mode_i = 0; mode_i < M_COUNT; ++mode_i ) {
                printf( "%s", mode_to_str(mode_i).cc );
                if( mode_i + 1 < M_COUNT ) {
                    printf( ", " );
                } else {
                    printf( "\n" );
                }
            }
            printf( "  help <mode>  Print help for mode and exit.\n");
            printf( "                 valid: " );
            for( Mode mode_i = 0; mode_i < M_COUNT; ++mode_i ) {
                printf( "%s", mode_to_str(mode_i).cc );
                if( mode_i + 1 < M_COUNT ) {
                    printf( ", " );
                } else {
                    printf( "\n" );
                }
            }
        } break;
        case M_BUILD: {
            printf( "  -n <string>  Override library name. (default = libmedia)\n");
            printf( "  -o <path>    Set output directory. (default = ./build)\n");
            printf( "                 NOTE: cbuild only creates output dir when this flag is unused.\n" );
            printf( "  -t <target>  Set target. (default = native)\n");
            printf( "                 valid: " );
            for( Target target_i = 0; target_i < T_COUNT; ++target_i ) {
                printf( "%s", target_to_str(target_i).cc );
                if( target_i + 1 < T_COUNT ) {
                    printf( ", " );
                } else {
                    printf( "\n" );
                }
            }
            printf( "  -release     Build in release mode. (default = false)\n");
            printf( "                 Enables optimizations and disables logging.\n");
            printf( "  -no-symbols  Strips debug symbols from build. (default = false)\n" );
            printf( "  -static      Build static library instead of dynamic. (default = false)\n");
            printf( "                 Prints required link flags for current target after compilation completes.\n");
            printf( "  -dry         Don't actually build, just print configuration.\n" );
        } break;
        case M_TEST: {
            printf( "  -o <path>    Set output directory. (default = ./build)\n");
            printf( "                 NOTE: cbuild only creates output dir when this flag is unused.\n" );
            printf( "  -t <target>  Set target. (default = native)\n");
            printf( "                 valid: " );
            for( Target target_i = 0; target_i < T_COUNT; ++target_i ) {
                printf( "%s", target_to_str(target_i).cc );
                if( target_i + 1 < T_COUNT ) {
                    printf( ", " );
                } else {
                    printf( "\n" );
                }
            }
            printf( "  -release     Build in release mode. (default = false)\n");
            printf( "                 Enables optimizations and disables logging.\n");
            printf( "  -no-symbols  Strips debug symbols from build. (default = false)\n" );
            printf( "  -static      Build static library instead of dynamic. (default = false)\n");
            printf( "                 Prints required link flags for current target after compilation completes.\n");
            printf( "  -dry         Don't actually build, just print configuration.\n" );
            printf( "  --           Stop parsing cbuild arguments and pass remaining arguments to test program.\n" );
        } break;
        case M_DOCS: {
            printf( "  -t <target>  Set target. (default = native)\n");
            printf( "                 valid: " );
            for( Target target_i = 0; target_i < T_COUNT; ++target_i ) {
                printf( "%s", target_to_str(target_i).cc );
                if( target_i + 1 < T_COUNT ) {
                    printf( ", " );
                } else {
                    printf( "\n" );
                }
            }
            printf( "  -static      Set static flags. (default = false)\n");
            printf( "  -browser     Open docs after generating.\n" );
            printf( "                 Checks for firefox, chromium and google-chrome, in that order.\n");
        } break;
        case M_LSP: {
            printf( "  -t <target>  Set target. (default = native)\n");
            printf( "                 valid: " );
            for( Target target_i = 0; target_i < T_COUNT; ++target_i ) {
                printf( "%s", target_to_str(target_i).cc );
                if( target_i + 1 < T_COUNT ) {
                    printf( ", " );
                } else {
                    printf( "\n" );
                }
            }
            printf( "  -static      Set static flags in compile_flags.txt (default = false)\n");
        } break;
        case M_COUNT: break;
    }
    return 0;
}
String mode_to_str( Mode mode ) {
    switch( mode ) {
        case M_HELP:  return string_text("help");
        case M_BUILD: return string_text("build");
        case M_TEST:  return string_text("test");
        case M_DOCS:  return string_text("docs");
        case M_LSP:   return string_text("lsp");
        case M_COUNT: break;
    }
    unreachable();
}
bool mode_from_str( String str, Mode* out_mode ) {
    for( Mode mode = 0; mode < M_COUNT; ++mode ) {
        if( string_cmp( str, mode_to_str(mode))) {
            *out_mode = mode;
            return true;
        }
    }
    return false;
}
String mode_description( Mode mode ) {
    switch( mode ) {
        case M_HELP:  return string_text("Print this message and quit.");
        case M_BUILD: return string_text("Build library.");
        case M_TEST:  return string_text("Build library, tests and then run tests.");
        case M_DOCS:  return string_text("Generate documentation.");
        case M_LSP:   return string_text("Generate LSP files (clangd).");
        case M_COUNT: break;
    }
    unreachable();
}
String target_to_str( Target target ) {
    switch( target ) {
        case T_NATIVE: return string_text("native");
        case T_COUNT: break;
    }
    unreachable();
}
bool target_from_str( String str, Target* out_target ) {
    for( Target target = 0; target < T_COUNT; ++target ) {
        if( string_cmp( str, target_to_str( target ) ) ) {
            *out_target = target;
            return true;
        }
    }
    return false;
}
Platform platform_current(void) {
#if defined(PLATFORM_WINDOWS)
    return P_WINDOWS;
#elif defined(PLATFORM_LINUX)
    return P_LINUX;
#elif defined(PLATFORM_MACOS)
    return P_MACOS;
#else
    return P_UNKNOWN;
#endif
}
String platform_friendly_name( Platform platform ) {
    switch( platform ) {
        case P_WINDOWS : string_text( "windows" );
        case P_LINUX   : string_text( "linux" );
        case P_MACOS   : string_text( "macos" );
        case P_UNKNOWN : string_text( "unknown" );
    }
    unreachable();
}
String platform_name( Platform platform ) {
    switch( platform ) {
        case P_WINDOWS : string_text( "win32" ); // lol
        case P_LINUX   : string_text( "linux" );
        case P_MACOS   : string_text( "macos" );
        case P_UNKNOWN : string_text( "unknown" );
    }
    unreachable();
}

#define CBUILD_IMPLEMENTATION
#include "cbuild.h"

