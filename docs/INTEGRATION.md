# Integration Guide

## `add_subdirectory`

```cmake
set(FONTTOOL_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(FONTTOOL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FONTTOOL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FONTTOOL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/FontTool)

target_link_libraries(product PRIVATE FontTool::FontTool)
```

The consumer receives only `include/fonttool` as a public include directory. Internal PoDoFo and AFDKO headers are not exposed.

## FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    FontTool
    GIT_REPOSITORY https://github.com/OWNER/FontTool.git
    GIT_TAG v1.0.0)
set(FONTTOOL_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(FONTTOOL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(FontTool)

target_link_libraries(product PRIVATE FontTool::FontTool)
```

Replace `OWNER` with the repository owner after publishing.

## Installed package

FontTool installs:

```text
include/fonttool/*.hpp
lib/libfonttool.*
lib/cmake/FontTool/FontToolConfig.cmake
lib/pkgconfig/fonttool.pc
bin/fonttool                 # when CLI is enabled
share/doc/FontTool/*
```

Consumer:

```cmake
find_package(FontTool CONFIG REQUIRED)
target_link_libraries(product PRIVATE FontTool::FontTool)
```

`FontToolConfig.cmake` discovers FreeType automatically through CMake's `FindFreetype` module.

## pkg-config

```bash
c++ main.cpp $(pkg-config --cflags --libs fonttool)
```

For static linking, use:

```bash
c++ main.cpp $(pkg-config --cflags --libs --static fonttool)
```

## Custom FreeType installation

Point CMake at a prefix:

```bash
cmake -S . -B build -DFONTTOOL_FREETYPE_ROOT=/opt/freetype
```

or use standard CMake variables such as `CMAKE_PREFIX_PATH` and the variables accepted by `FindFreetype`.

When a custom static FreeType build requires libraries not represented by `Freetype::Freetype`, pass them through:

```bash
-DFONTTOOL_EXTRA_LINK_LIBRARIES="png;z;bz2"
```

## ABI guidance

The API uses C++ standard-library types. Build FontTool and its consumer with compatible compiler, standard library, runtime library and iterator-debug settings. This is especially important for Windows DLL builds.

## Mobile and cross-compilation

Use a target-platform FreeType build and the normal CMake toolchain file. Do not link a desktop FreeType binary into Android or iOS builds. Disable host tools when cross-compiling:

```bash
-DFONTTOOL_BUILD_CLI=OFF
-DFONTTOOL_BUILD_EXAMPLES=OFF
-DFONTTOOL_BUILD_TESTS=OFF
```
