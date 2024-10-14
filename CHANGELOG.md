Changelog
=========

The following rules only apply to major versions >= 1.

## Major version
Number between 0-65535.

Indicates breaking changes to API or implementation.

## Minor version
Number between 0-255.

Indicates implementation changes or bug fixes that affect behavior.

## Patch version
Number between 0-255

Indicates additions to API, minor bug fixes and other small changes.

0.1.1
-----
- tests: updated to use stdint and stdbool types.
- cbuild: copied latest version.
- cbuild: completely rewritten and now has flags for posix platforms.
- surface: made surface_pump_events() global. (thanks, X11)
- everything: updated to use stdint and stdbool types.
- types: changed typedefs to just use stdint and stdbool.
- defines: finished added define guards for attributes that corelib also defines.
- ./test/: renamed to tests
- sources.h: added guard around platform_sharedmain.c
- gitignore: ignore cbuild executable, ignore ./generated/
- readme.md: removed references to GNU make.
- build.md: updated with cbuild instructions.
- doxyfile: removed reference to corelib.
- cbuild: replaced Makefile with cbuild.
- input/win32: decoupled input handling from media_initialize into its own functions.
- surface:win32: bug: mouse delta y was negative.
- surface:win32: message thread no longer calls WaitMessage().
- surface:win32: PostMessageA for WM_INPUT_MOUSE_POSITION_RELATIVE is always called.
- surface:win32: bug: removed memory_free() call, surface is no longer allocated by medialib.
- render:opengl: defined MEDIA_OPENGL_DEFAULT_*_VERSION so that default OpenGL version is uniform across platforms.
- render:opengl: renamed OpenGLAttributes to MediaOpenGLAttributes and now is no longer heap-allocated.
- surface: added media_surface_query_memory_requirement()
- surface: no longer allocates surface memory, instead expects pre-allocated buffer.
- lib: applied changes made to logging from corelib
- bug:surface:win32: surface_set_name() would not clear previous name properly.
- Makefile: fixed TARGET dependency on libcore
- Makefile: added LIBCORE_SRC_PATH variable

0.1.0
------
- First version.

