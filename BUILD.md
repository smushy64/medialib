Build from Source
=================

## Tools Required

### General

- clang
- [Doxygen >= 1.10.0](https://www.doxygen.nl/) (for generating documentation)

### Windows

- [MinGW](https://www.mingw-w64.org/)

## Steps

1) cd into root directory

2) compile `cbuild`
```console
clang cbuild.c -o cbuild
```

3) to compile for current platform:
```console
./cbuild build
```

By default, library will be in `./build/`

to compile and run tests:
```console
./cbuild test
```

to generate documentation:
```console
./cbuild docs
```

additional build instructions can be obtained with:
```console
./cbuild help
```

