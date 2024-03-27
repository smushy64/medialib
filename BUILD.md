Build from Source
=================

## Tools Required

### General

- GNU Make
- clang
- [Doxygen >= 1.10.0](https://www.doxygen.nl/) (for generating documentation)

### Windows

- [MinGW](https://www.mingw-w64.org/)

## Dependencies

- [corelib](https://github.com/smushy64/corelib) (can be obtained with git submodule).

## Steps

1) cd into root directory

2) to compile for current platform and architecture:
```console
make
```

By default, library will be in ./build/

to compile and run tests:
```console
make test
```

to generate documentation:
```console
make docs
```

additional build instructions can be obtained with:
```console
make help
```

