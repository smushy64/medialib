Changelog
=========

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

