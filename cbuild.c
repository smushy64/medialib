/**
 * @file   cbuild.c
 * @brief  Media library build system.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   September 10, 2024
*/
#include "cbuild.h"
#include <unistd.h>
#define println( format, args... ) printf( format "\n", ##args )

#define MEDIA_LIB_VERSION_MAJOR 0
#define MEDIA_LIB_VERSION_MINOR 1
#define MEDIA_LIB_VERSION_PATCH 0

enum Mode {
    M_HELP,
    M_BUILD,
    M_DOCS,
    M_TEST,
    M_LSP,

    M_COUNT
};
const char* mode_to_string( enum Mode mode, usize* opt_out_len );
b32 mode_parse( string str, enum Mode* out_mode );

enum Target {
    T_NATIVE,
};
const char* target_to_string( enum Target target, usize* opt_out_len );
b32 target_parse( string str, enum Target* out_target );

struct Arguments {
    enum Mode mode;
    union {
        struct HelpArguments {
            enum Mode mode;
        } help;
        struct BuildArguments {
            string output;
            enum Target target;
            b32 release;
            b32 optimized;
            b32 strip_symbols;
            b32 is_static;
            b32 enable_logging;
            b32 dry;
        } build;
        struct DocsArguments {
            struct BuildArguments build;
            b32 launch_browser;
        } docs;
        struct TestArguments {
            struct BuildArguments build;
            int passthrough_start;
        } test;
        struct LspArguments {
            struct BuildArguments build;
        } lsp;
    };
};
void print_arguments( const struct Arguments* args );

b32 parse_arguments( int argc, char** argv, struct Arguments* out_args );

void mode_help( enum Mode mode );
int mode_build( struct BuildArguments* build );
int mode_test( int argc, char** argv, struct TestArguments* test );
int mode_docs( struct DocsArguments* docs );
int mode_lsp( struct LspArguments* lsp );

int main( int argc, char** argv ) {
    init( LOGGER_LEVEL_INFO );

    struct Arguments args;
    memory_set( &args, 0, sizeof(args) );
    if( !parse_arguments( argc, argv, &args ) ) {
        return -1;
    }

    int result = 0;
    switch( args.mode ) {
        case M_HELP: {
            mode_help( args.help.mode );
        } break;
        case M_BUILD: {
            result = mode_build( &args.build );
        } break;
        case M_TEST: {
            result = mode_build( &args.build );
            if( !result ) {
                result = mode_test( argc, argv, &args.test );
            }
        } break;
        case M_DOCS: {
            result = mode_docs( &args.docs );
        } break;
        case M_LSP: {
            result = mode_lsp( &args.lsp );
        } break;
        case M_COUNT:
            return -1;
    }

    return result;
}
int mode_build( struct BuildArguments* build ) {
    // TODO(alicia): linux flags

    if( !build->dry ) {
        if( !process_in_path( "clang") ) {
            cb_error( "build: 'clang' not found in path!");
            return -1;
        }
    }

    const char* path_command_line = "generated/medialib_command_line.c";

    dstring* output = dstring_fmt(
        "%.*s/libmedia.%s", build->output.len, build->output.cc,
        build->is_static ? "o" : "dll" );
    if( !output ) {
        cb_error( "build: failed to create output path!" );
        return -1;
    }
    if( !build->dry ) {
        cb_info( "build: compiling '%s' . . .", output );
    }

    CommandBuilder builder;
    if( !command_builder_new( "clang", &builder ) ) {
        cb_error( "build: failed to create command builder!" );
        dstring_free( output );
        return -1;
    }
    #define push( arg ) do {\
        if( !command_builder_push( &builder, arg)) {\
            cb_error( "build: failed to push argument '%s'!", arg);\
            command_builder_free( &builder );\
            dstring_free( output );\
            return -1;\
        }\
    } while(0)

    push( "-std=c11" );
    push( "-include" );
    push( path_command_line );
    push( "-xc" );
    push( "impl/sources.h" );
    if( build->is_static ) {
        push( "-c" );
        push( "-DMEDIA_ENABLE_STATIC_BUILD" );
    } else {
        push( "-shared" );
        push( "-DMEDIA_ENABLE_EXPORT" );
    }
    push( "-o" );
    push( output );
    push( "-I.");
    push( "-nostdlib" );
    push( "-Wall" );
    push( "-Wextra" );
    push( "-Werror=vla" );
    push( "-Werror" );

    if( build->optimized ) {
        push( "-O2" );
    } else {
        push( "-O0" );
    }
    if( build->strip_symbols ) {
    } else {
        push( "-g" );
        push( "-gcodeview");
        push( "-fuse-ld=lld" );
    }
    if( build->release ) {
    } else {
        if( !build->is_static ) {
            push( "-Wl,/debug");
        }
    }

    if( !build->is_static ) {
        push( "-lkernel32" );
    }

    if( build->enable_logging ) {
        push( "-DMEDIA_ENABLE_LOGGING" );
    }
    push( "-DMEDIA_LIB_VERSION_MAJOR=" macro_value_to_string(MEDIA_LIB_VERSION_MAJOR) );
    push( "-DMEDIA_LIB_VERSION_MINOR=" macro_value_to_string(MEDIA_LIB_VERSION_MINOR) );
    push( "-DMEDIA_LIB_VERSION_PATCH=" macro_value_to_string(MEDIA_LIB_VERSION_PATCH) );

    Command cmd = command_builder_cmd( &builder );
    dstring* flat = command_flatten_dstring( &cmd );
    if( !flat ) {
        cb_error( "build: failed to flatten command line!");
        command_builder_free( &builder );
        dstring_free( output );
        return -1;
    }

    if( !build->dry ) {
        if( string_cmp( build->output, string_text("build"))) {
            if( !path_exists( "build" ) ) {
                if( !dir_create("build") ) {
                    cb_error( "build: failed to create build directory!");
                    command_builder_free( &builder );
                    dstring_free( output );
                    dstring_free( flat );
                    return -1;
                }
            }
        }

        if( !path_exists( "generated" ) ) {
            if( !dir_create( "generated" ) ) {
                cb_error( "build: failed to create generated directory!" );
                command_builder_free( &builder );
                dstring_free( output );
                dstring_free( flat );
                return -1;
            }
        }

        FD fd_command_line;
        FileOpenFlags fd_flags = FOPEN_WRITE;
        if( path_exists( path_command_line ) ) {
            fd_flags |= FOPEN_TRUNCATE;
        } else {
            fd_flags |= FOPEN_CREATE;
        }
        if( !fd_open(
            path_command_line, fd_flags,
            &fd_command_line
        ) ) {
            cb_error( "build: failed to create generated command line!" );
            command_builder_free( &builder );
            dstring_free( output );
            dstring_free( flat );
            return -1;
        }
        #define write( format, args... ) do {\
            if( !fd_write_fmt( &fd_command_line, format, ##args ) ) {\
                cb_error( "build: failed to generate command line!" );\
                fd_close( &fd_command_line );\
                command_builder_free( &builder );\
                dstring_free( output );\
                dstring_free( flat );\
                return -1;\
            }\
        } while(0)

        write( "// generated command line for media lib.\n" );
        write( "const char external_media_library_command_line[] = \"%s\";\n", flat );
        write( "unsigned int external_media_library_command_line_len = "
              "sizeof(external_media_library_command_line) - 1;\n\n");

        fd_close( &fd_command_line );
        #undef write
    }

    int result = 0;
    if( build->dry ) {
        cb_info( "%s", flat );
        dstring_free( flat );
    } else {
        dstring_free( flat );

        PID pid = process_exec( cmd, false, 0, 0, 0, 0 );
        result  = process_wait( pid );

        if( result ) {
            cb_error( "build: failed to compile project!" );
        } else {
            cb_info( "build: project compiled at path '%s'!", output );
        }
    }

    command_builder_free( &builder );
    dstring_free( output );
    #undef push
    return result;
}
int mode_test( int argc, char** argv, struct TestArguments* test ) {
    // TODO(alicia): linux flags!

    if( test->build.dry ) {
        return 0;
    }

    dstring* output = dstring_fmt(
        "%.*s/libmedia.%s", test->build.output.len, test->build.output.cc,
        test->build.is_static ? "o" : "dll" );
    if( !output ) {
        cb_error( "test: failed to create output path!" );
        return -1;
    }

    cb_info( "test: compiling testing project . . .");
    CommandBuilder builder;
    if( !command_builder_new( "clang", &builder ) ) {
        cb_error( "test: failed to create command builder!" );
        dstring_free( output );
        return -1;
    }
    #define push( arg ) do {\
        if( !command_builder_push( &builder, arg ) ) {\
            cb_error( "test: failed to push argument!");\
            command_builder_free( &builder );\
            if( output ) {\
                dstring_free( output );\
            }\
            return -1;\
        }\
    } while(0)

    push( "-std=c11" );
    push( "tests/test.c" );
    if( test->build.is_static ) {
        push( output );
        push( "-lkernel32" );
        push( "-DMEDIA_ENABLE_STATIC_BUILD" );
    } else {
        push("-L");
        push( test->build.output.cc );
        push( "-lmedia" );
    }
    push( "-o" );
    push( "build/libmedia-test.exe" );
    push( "-O0" );
    push( "-g" );
    push( "-gcodeview");
    push( "-fuse-ld=lld");
    push( "-Wl,/debug");
    push( "-I.");

    Command cmd = command_builder_cmd( &builder );
    PID pid = process_exec( cmd, 0, 0, 0, 0, 0 );
    int result = process_wait( pid );

    dstring_free(output);
    command_builder_free( &builder );

    if( result ) {
        cb_error( "test: failed to compile test project!");
        return result;
    }
    cb_info( "test: test project compiled at path 'build/libmedia-test.exe'!" );

    if( !command_builder_new( "build/libmedia-test.exe", &builder ) ) {
        cb_error( "test: failed to create test command builder!");
        return -1;
    }

    if( test->passthrough_start ) {
        for( int i = test->passthrough_start; i < argc; ++i ) {
            push( argv[i] );
        }
    }

    cb_info( "test: running test project . . ." );
    cmd = command_builder_cmd( &builder );
    pid = process_exec( cmd, false, 0, 0, 0, 0 );
    result = process_wait( pid );

    cb_info( "test: test project exited with code %i.", result );

    #undef push
    return 0;
}
int mode_docs( struct DocsArguments* docs ) {
    if( !process_in_path( "doxygen" ) ) {
        cb_error( "docs: doxygen is required to build documentation!" );
        return -1;
    }
    cb_info( "docs: generating doxygen settings for build configuration . . ." );

    dstring* settings = dstring_empty( kibibytes(4) );
    if( !settings ) {
        cb_error( "docs: failed to allocate doxygen settings buffer!");
        return -1;
    }
    #define write( format, args... ) do {\
        const char* formatted = local_fmt( format, ##args );\
        dstring* _new = dstring_append_cstr( settings, formatted );\
        if( !_new ) {\
            cb_error( "docs: failed to reallocate settings buffer!");\
            return -1;\
        }\
    } while(0)

    write( "PREDEFINED += ");
    write( "MEDIA_LIB_VERSION_MAJOR=%i ", MEDIA_LIB_VERSION_MAJOR );
    write( "MEDIA_LIB_VERSION_MINOR=%i ", MEDIA_LIB_VERSION_MINOR );
    write( "MEDIA_LIB_VERSION_PATCH=%i ", MEDIA_LIB_VERSION_PATCH );
    if( docs->build.release ) {
    }
    if( docs->build.is_static ) {
        write( "MEDIA_ENABLE_STATIC_BUILD " );
    }
    if( docs->build.enable_logging ) {
        write( "MEDIA_ENABLE_LOGGING " );
    }
    write( "__clang__ " );
    switch( docs->build.target ) {
        case T_NATIVE: {
#if defined(PLATFORM_WINDOWS)
            write( "_WIN32 ");
#elif defined(PLATFORM_LINUX)
            write( "__linux__ " );
#elif defined(PLATFORM_MACOS)
            write( "__APPLE__ TARGET_OS_MAC " );
#endif
#if defined( ARCH_64BIT )
    #if defined(PLATFORM_MINGW)
            write( "__MINGW64__ " );
    #endif
#else
    #if defined(PLATFORM_MINGW)
            write( "__MINGW32__ " );
    #endif
#endif
#if defined(ARCH_X86)
    #if defined(ARCH_64BIT)
            write( "__x86_64__ " );
    #else
            write( "__i386__ " );
    #endif
#elif defined(ARCH_ARM)
    #if defined(ARCH_64BIT)
            write( "__aarch64__ " );
    #else
            write( "__arm__ " );
    #endif
#endif
        } break;
    }

    write( "\nPROJECT_NUMBER = %i.%i.%i",
        MEDIA_LIB_VERSION_MAJOR, MEDIA_LIB_VERSION_MINOR, MEDIA_LIB_VERSION_PATCH );

    const char* settings_path = "docs/Doxyfile_generated";
    if( path_exists( settings_path ) ) {
        file_remove( settings_path );
    }

    if( !file_copy( settings_path, "docs/Doxyfile_default")) {
        cb_error( "docs: failed to copy default doxygen settings!");
        dstring_free( settings );
        return -1;
    }

    FD fd_settings;
    if( !fd_open( settings_path, FOPEN_WRITE, &fd_settings ) ) {
        cb_error( "docs: failed to open settings file!" );
        dstring_free( settings );
        return -1;
    }
    fd_seek( &fd_settings, FSEEK_END, 0 );
    b32 write_success = fd_write( &fd_settings, dstring_len(settings), settings, 0 );
    fd_close( &fd_settings );
    dstring_free( settings );

    if( !write_success ) {
        cb_error( "docs: failed to write to settings!");
        return -1;
    }
    cb_info( "docs: generated doxygen settings at '%s'!", settings_path );

    Command cmd = command_new( "doxygen", "Doxyfile_generated", "-q" );
    // TODO(alicia): add dir_change to cbuild to avoid this
    chdir( "docs" );
    PID pid = process_exec( cmd, false, 0, 0, 0, 0 );
    chdir( ".." );

    cb_info( "docs: generating documentation with doxygen . . ." );
    int result = process_wait( pid );
    cb_info( "docs: doxygen exited with code %i.", result );
    if( result ) {
        return result;
    }

    cb_info( "docs: documentation generated at 'docs/html/index.html'");

    if( docs->launch_browser ) {
        cb_info("docs: attempting to launch browser . . ." );
        b32 process_found = false;
        Command browser_cmd;
#if defined( PLATFORM_WINDOWS )
        if( process_in_path( "pwsh" ) ) {
            browser_cmd = command_new(
                "pwsh", "-Command", "Invoke-Expression", "docs/html/index.html" );
            process_found = true;
        }
#endif
        if( !process_found ) {
            if( process_in_path( "firefox" ) ) {
                browser_cmd = command_new( "firefox", "docs/html/index.html" );
                process_found = true;
            } else if( process_in_path("chromium")) {
                browser_cmd = command_new( "chromium", "docs/html/index.html" );
                process_found = true;
            } else if( process_in_path("google-chrome")) {
                browser_cmd = command_new( "google-chrome", "docs/html/index.html" );
                process_found = true;
            }
        }

        if( !process_found ) {
            cb_warn(
                "docs: attempted to launch browser "
                "but no recognized browser was found!");
            return 0;
        }

        pid = process_exec( browser_cmd, true, 0, 0, 0, 0 );
        process_discard( pid );

        cb_info( "docs: launched documentation in browser." );
    }

    #undef write
    return 0;
}
int mode_lsp( struct LspArguments* lsp ) {
    dstring* _template = dstring_empty( kibibytes(1) );
    if( !_template ) {
        cb_error( "lsp: failed to allocate buffer!");
        return -1;
    }
    #define push( format, args... ) do {\
        const char* formatted = local_fmt( format, ##args );\
        dstring* _new = dstring_append_cstr( _template, formatted );\
        if( !_new ) {\
            cb_error( "lsp: failed to reallocate buffer!");\
            return -1;\
        }\
    } while(0)

    cb_info( "lsp: generating flags . . .");

    push( "clang\n" );
    push( "-std=c11\n" );
    push( "-I..\n" );
    push( "-Wall\n" );
    push( "-Wextra\n" );
    push( "-D_CLANGD\n" );
    push( "-DMEDIA_LIB_VERSION_MAJOR=" macro_value_to_string(MEDIA_LIB_VERSION_MAJOR) "\n" );
    push( "-DMEDIA_LIB_VERSION_MINOR=" macro_value_to_string(MEDIA_LIB_VERSION_MINOR) "\n" );
    push( "-DMEDIA_LIB_VERSION_PATCH=" macro_value_to_string(MEDIA_LIB_VERSION_PATCH) "\n" );

    if( lsp->build.enable_logging ) {
        push( "-DMEDIA_ENABLE_LOGGING\n" );
    }

    usize generic_flags_len = dstring_len( _template );

    cb_info( "lsp: generic compile flags generated." );

    cb_info( "lsp: writing ./media compile flags . . ." );

    if( lsp->build.is_static ) {
        push( "-DMEDIA_ENABLE_STATIC_BUILD\n");
    } else {
        push( "-DMEDIA_ENABLE_EXPORT\n" );
    }
    cb_info( "lsp: media compile flags generated.");

    FD fd;
    FileOpenFlags flags = FOPEN_WRITE;
    if( path_exists( "media/compile_flags.txt") ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    if( fd_open( "media/compile_flags.txt", flags, &fd ) ) {
        fd_write( &fd, dstring_len(_template), _template, 0 );
        fd_close( &fd );
    } else {
        cb_warn( "lsp: failed to open ./media compile flags!" );
    }

    cb_info( "lsp: writing ./tests compile flags . . ." );

    dstring_truncate( _template, generic_flags_len );
    if( lsp->build.is_static ) {
        push( "-DMEDIA_ENABLE_STATIC_BUILD\n");
    }
    cb_info( "lsp: tests compile flags generated." );

    flags = FOPEN_WRITE;
    if( path_exists( "tests/compile_flags.txt") ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    if( fd_open( "tests/compile_flags.txt", flags, &fd ) ) {
        fd_write( &fd, dstring_len(_template), _template, 0 );
        fd_close( &fd );
    } else {
        cb_warn( "lsp: failed to open ./tests compile flags!" );
    }

    cb_info( "lsp: writing ./impl compile flags . . ." );

    dstring_truncate( _template, generic_flags_len );
    if( lsp->build.is_static ) {
        push( "-DMEDIA_ENABLE_STATIC_BUILD\n");
    } else {
        push( "-DMEDIA_ENABLE_EXPORT\n");
    }
    cb_info( "lsp: impl compile flags generated.");

    flags = FOPEN_WRITE;
    if( path_exists( "impl/compile_flags.txt") ) {
        flags |= FOPEN_TRUNCATE;
    } else {
        flags |= FOPEN_CREATE;
    }

    if( fd_open( "impl/compile_flags.txt", flags, &fd ) ) {
        fd_write( &fd, dstring_len(_template), _template, 0 );
        fd_close( &fd );
    } else {
        cb_warn( "lsp: failed to open ./impl compile flags!" );
    }

    dstring_free( _template );
    #undef push
    return 0;
}

b32 parse_arguments( int argc, char** argv, struct Arguments* out_args ) {
    if( argc <= 1 ) {
        return true;
    }

    if( !mode_parse( string_from_cstr( argv[1] ), &out_args->mode ) ) {
        cb_error( "failed to parse mode '%s'!", argv[1] );
        mode_help( M_HELP );
        return false;
    }

    switch( out_args->mode ) {
        case M_BUILD:
        case M_DOCS:
        case M_TEST:
        case M_LSP: {
            out_args->build.output = string_text( "build" );
        } break;
        case M_HELP:
        case M_COUNT:
          break;
    }

    for( int i = 2; i < argc; ++i ) {
        string arg = string_from_cstr( argv[i] );
        switch( out_args->mode ) {
            case M_HELP: {
                if( mode_parse( arg, &out_args->help.mode ) ) {
                    return true;
                }
            } break;
            case M_BUILD: case M_DOCS: case M_TEST:
            case M_LSP: {
                // NOTE(alicia): parse generic flags

                string part = string_adv_by( arg, 2 );
                if( part.len ) {
                    if( string_cmp( part, string_text("release"))) {
                        out_args->build.release = true;
                        continue;
                    } else if( string_cmp( part, string_text("optimized"))) {
                        out_args->build.optimized = true;
                        continue;
                    } else if( string_cmp( part, string_text( "static" ))) {
                        out_args->build.is_static = true;
                        continue;
                    } else if( string_cmp( part, string_text("enable-logging"))) {
                        out_args->build.enable_logging = true;
                        continue;
                    }
                }
            } break;
            case M_COUNT: break;
        }
        switch( out_args->mode ) {
            case M_TEST:
            case M_BUILD: {
                if( string_cmp( arg, string_text("--dry"))) {
                    out_args->build.dry = true;
                    continue;
                } else if( string_cmp( string_truncate( arg, sizeof("--output") ), string_text("--output="))) {
                    string dir = string_adv_by( arg, sizeof("--output"));
                    if( string_is_empty( dir ) ) {
                        cb_error( "--output requires a path!" );
                        mode_help( out_args->mode );
                        return false;
                    }

                    out_args->build.output = dir;
                    continue;
                } else if( string_cmp( string_truncate( arg, sizeof("--target")), string_text("--target="))) {
                    string target = string_adv_by( arg, sizeof("--target"));
                    if( !target_parse( target, &out_args->build.target )) {
                        cb_error(
                            "--target requires valid target! '%.*s'",
                            target.len, target.cc );
                        return false;
                    }

                    continue;
                } else if( string_cmp( arg, string_text("--strip-symbols"))) {
                    out_args->build.strip_symbols = true;
                    continue;
                }
            } break;
            case M_DOCS: {
                if( string_cmp( arg, string_text( "--browser" ))) {
                    out_args->docs.launch_browser = true;
                    continue;
                }
            } break;
            case M_LSP: case M_HELP: case M_COUNT:
                break;
        }
        switch( out_args->mode ) {
            case M_TEST: {
                if( string_cmp( arg, string_text("--"))) {
                    out_args->test.passthrough_start = i + 1;
                    goto parse_arguments_end;
                }
            } break;
            case M_BUILD: case M_HELP: case M_DOCS: case M_LSP: case M_COUNT:
                break;
        }

        cb_error( "unrecognized argument '%s'", arg.cc );
        mode_help( out_args->mode );
        return false;
    }

parse_arguments_end:
    return true;
}

void mode_help( enum Mode mode ) {
    println( "OVERVIEW:    Media library build system." );
    println( "USAGE:       cbuild %s [args]", mode == M_HELP ? "<mode>" : mode_to_string(mode, 0));
    printf(  "DESCRIPTION: ");

    switch( mode ) {
        case M_HELP: {
            println( "Print this help message or if a mode is provided, print help for that mode. ex: cbuild help build" );
        } break;
        case M_BUILD: {
            println( "Build project. Compiled with clang (mingw on windows)." );
        } break;
        case M_DOCS: {
            println( "Create documentation. Doxygen is required.");
        } break;
        case M_TEST: {
            println( "Build project and run tests with it.");
        } break;
        case M_LSP: {
            println( "Generate clangd compile_flags.txt for build configuration.");
        } break;
        case M_COUNT: break;
    }

    if( mode == M_HELP ) {
        println( "MODES:" );
    } else {
        println( "ARGUMENTS:");
    }

    switch( mode ) {
        case M_TEST:
        case M_BUILD: {
            println( "  --output=<dir-path> set output directory. directory must already exist. (default=build)");
            println( "  --target=<platform> build for given platform. (default=native)" );
            println( "                        valid: native");
            println( "  --strip-symbols     don't generate debug symbols. (default=false)");
        } break;
        case M_HELP: case M_DOCS: case M_LSP: case M_COUNT:
          break;
    }
    switch( mode ) {
        case M_HELP: {
            println( "  <mode,optional>  name of mode to print help for.");
            printf ( "                      valid: " );
            for( enum Mode m = M_HELP; m < M_COUNT; ++m ) {
                printf( "%s", mode_to_string(m, 0) );
                if( m + 1 != M_COUNT ) {
                    printf( ", " );
                }
            }
            printf( "\n" );
        } break;
        case M_TEST: 
        case M_DOCS: 
        case M_LSP: 
        case M_BUILD: {
            println( "  --release           build in release mode. (default=false)");
            println( "  --optimized         build with optimizations on. (default=false)");
            println( "  --static            build object file instead of dll/so. (default=false)");
            println( "  --enable-logging    enable library logging. (default=false)");
        } break;
        case M_COUNT:
          break;
    }
    switch( mode ) {
        case M_DOCS: {
            println( "  --browser           attempt to launch browser with documentation. (chrome, chromium and firefox only)");
        } break;
        case M_LSP: {
        } break;

        case M_TEST: {
            println( "  --                  stop parsing arguments and pass remaining arguments to test executable.");
        }
        case M_BUILD: {
            println( "  --dry               don't actually build, just output command line arguments.");
        } break;
        case M_HELP:
        case M_COUNT: break;
    }
}
const char* target_to_string( enum Target target, usize* opt_out_len ) {
    #define result( str ) do {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(str) - 1;\
        }\
        return str;\
    } while(0)

    switch( target ) {
        case T_NATIVE: result( "native" );
    }
    panic( "invalid mode provided!" );

    #undef result
}
b32 target_parse( string str, enum Target* out_target ) {
    if( string_cmp( str, string_text("native"))) {
        *out_target = T_NATIVE;
        return true;
    }

    enum Target warning_if_not_implemented = 0;
    switch( warning_if_not_implemented ) {
        case T_NATIVE:
          break;
    }
    return false;
}
const char* mode_to_string( enum Mode mode, usize* opt_out_len ) {
    #define result( str ) do {\
        if( opt_out_len ) {\
            *opt_out_len = sizeof(str) - 1;\
        }\
        return str;\
    } while(0)

    switch( mode ) {
        case M_HELP:  result("help");
        case M_BUILD: result("build");
        case M_DOCS:  result("docs");
        case M_TEST:  result("test");
        case M_LSP:   result("lsp");

        case M_COUNT: break;
    }
    panic( "invalid mode provided!" );

#undef result
}
b32 mode_parse( string str, enum Mode* out_mode ) {
    if( string_cmp( str, string_text("help"))) {
        *out_mode = M_HELP;
        return true;
    }
    if( string_cmp( str, string_text("build"))) {
        *out_mode = M_BUILD;
        return true;
    }
    if( string_cmp( str, string_text("docs"))) {
        *out_mode = M_DOCS;
        return true;
    }
    if( string_cmp( str, string_text("test"))) {
        *out_mode = M_TEST;
        return true;
    }
    if( string_cmp( str, string_text("lsp"))) {
        *out_mode = M_LSP;
        return true;
    }

    enum Mode warning_if_not_implemented = 0;
    switch( warning_if_not_implemented ) {
        case M_HELP:
        case M_BUILD:
        case M_DOCS:
        case M_TEST:
        case M_LSP:
        case M_COUNT:
          break;
    }

    return false;
}

void print_arguments( const struct Arguments* args ) {
    cb_info( "mode: %s", mode_to_string( args->mode, 0 ));
    switch( args->mode ) {
        case M_HELP: {
            cb_info( "  help mode: %s", mode_to_string( args->help.mode, 0 ) );
        } break;
        case M_DOCS:
        case M_TEST:
        case M_LSP:
        case M_BUILD: {
            switch( args->mode ) {
                case M_TEST:
                case M_BUILD: {
                    cb_info( "  output:         '%.*s'", args->build.output.len, args->build.output.cc );
                    cb_info( "  target:         %s", target_to_string( args->build.target, 0 ) );
                    cb_info( "  dry:            %s", args->build.dry ? "true" : "false" );
                } break;
                case M_DOCS: {
                    cb_info( "  launch browser: %s", args->docs.launch_browser ? "true" : "false" );
                } break;
                case M_HELP: case M_LSP: case M_COUNT:
                    break;
            }
            cb_info( "  release:        %s", args->build.release ? "true" : "false" );
            cb_info( "  optimized:      %s", args->build.optimized ? "true" : "false" );
            cb_info( "  static:         %s", args->build.is_static ? "true" : "false" );
            cb_info( "  enable logging: %s", args->build.enable_logging ? "true" : "false" );
            if( args->mode == M_TEST ) {
                cb_info( "  passthrough:    %s", args->test.passthrough_start ? "true" : "false");
            }
        } break;
        case M_COUNT: break;
    }
}

#define CBUILD_IMPLEMENTATION
#include "cbuild.h"

