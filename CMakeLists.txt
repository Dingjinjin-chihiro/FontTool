# SPDX-License-Identifier: MPL-2.0
cmake_minimum_required(VERSION 3.20)

project(FontTool
    VERSION 1.0.0
    DESCRIPTION "Embeddable C++17 font inspection and subsetting library"
    LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(FONTTOOL_IS_TOP_LEVEL ON)
else()
    set(FONTTOOL_IS_TOP_LEVEL OFF)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(FONTTOOL_BUILD_CLI "Build the fonttool command-line application" ${FONTTOOL_IS_TOP_LEVEL})
option(FONTTOOL_BUILD_EXAMPLES "Build FontTool examples" ${FONTTOOL_IS_TOP_LEVEL})
option(FONTTOOL_BUILD_TESTS "Build FontTool tests" ${FONTTOOL_IS_TOP_LEVEL})
option(FONTTOOL_INSTALL "Generate install and package targets" ${FONTTOOL_IS_TOP_LEVEL})
option(FONTTOOL_ENABLE_WARNINGS "Enable compiler warnings for FontTool sources" ON)
option(FONTTOOL_WARNINGS_AS_ERRORS "Treat FontTool warnings as errors" OFF)

set(FONTTOOL_FREETYPE_ROOT "" CACHE PATH
    "Optional FreeType installation prefix used before normal package discovery")
set(FONTTOOL_EXTRA_LINK_LIBRARIES "" CACHE STRING
    "Additional libraries needed by a custom static FreeType build")
set(FONTTOOL_TEST_FONT "" CACHE FILEPATH
    "Optional font used by the integration test")

if(FONTTOOL_FREETYPE_ROOT)
    list(PREPEND CMAKE_PREFIX_PATH "${FONTTOOL_FREETYPE_ROOT}")
endif()

find_package(Freetype REQUIRED)

file(GLOB_RECURSE FONTTOOL_AFDKO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/adobe/afdko/source/*.c")

set(FONTTOOL_INTERNAL_SOURCES
    src/internal/podofo/auxiliary/BinaryIO.cpp
    src/internal/podofo/auxiliary/Corners.cpp
    src/internal/podofo/auxiliary/FontUtils.cpp
    src/internal/podofo/auxiliary/InputStream.cpp
    src/internal/podofo/auxiliary/Matrix.cpp
    src/internal/podofo/auxiliary/OutputStream.cpp
    src/internal/podofo/auxiliary/Rect.cpp
    src/internal/podofo/auxiliary/StreamDevice.cpp
    src/internal/podofo/auxiliary/StreamDeviceBase.cpp
    src/internal/podofo/auxiliary/Utf8Utils.cpp
    src/internal/podofo/auxiliary/Vector2.cpp
    src/internal/podofo/intermediate/FontSubsetAdapter.cpp
    src/internal/podofo/main/FontMetricsFreetype.cpp
    src/internal/podofo/main/FontToolsFacade.cpp
    src/internal/podofo/private/FontTrueTypeSubset.cpp
    src/internal/podofo/private/FontUtilsAFDKO.cpp
    src/internal/podofo/private/FreetypePrivate.cpp
    third_party/utf8proc/utf8proc.c
    ${FONTTOOL_AFDKO_SOURCES})

add_library(FontTool
    src/fonttool.cpp
    ${FONTTOOL_INTERNAL_SOURCES})
add_library(FontTool::FontTool ALIAS FontTool)

set_target_properties(FontTool PROPERTIES
    OUTPUT_NAME fonttool
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    DEBUG_POSTFIX d)

target_compile_features(FontTool PUBLIC cxx_std_17)
set_property(TARGET FontTool PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET FontTool PROPERTY C_STANDARD 99)
set_property(TARGET FontTool PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET FontTool PROPERTY C_EXTENSIONS OFF)

target_include_directories(FontTool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/adobe/afdko/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/adobe/afdko/resource
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/utf8cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/utf8proc)

target_link_libraries(FontTool
    PRIVATE
        Freetype::Freetype
        ${FONTTOOL_EXTRA_LINK_LIBRARIES})

target_compile_definitions(FontTool
    PRIVATE
        FONTTOOL_BUILDING_LIBRARY=1
        TCB_SPAN_NO_EXCEPTIONS=1
        UTF8PROC_STATIC=1
        UTF8PROC_EXPLICIT_INIT=1
        _CRT_SECURE_NO_WARNINGS)

if(UNIX)
    target_compile_definitions(FontTool PRIVATE _POSIX_C_SOURCE=200809L)
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(FontTool PUBLIC FONTTOOL_SHARED=1)
endif()

if(FONTTOOL_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(FontTool PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:/W4>
            $<$<COMPILE_LANGUAGE:CXX>:/utf-8>
            $<$<COMPILE_LANGUAGE:CXX>:/permissive->
            $<$<COMPILE_LANGUAGE:C>:/W3>)
        if(FONTTOOL_WARNINGS_AS_ERRORS)
            target_compile_options(FontTool PRIVATE /WX)
        endif()
    else()
        target_compile_options(FontTool PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
        if(FONTTOOL_WARNINGS_AS_ERRORS)
            target_compile_options(FontTool PRIVATE -Werror)
        endif()
    endif()
endif()

# The public library does not propagate C++ exceptions. utf8cpp's checked
# decoder is isolated in one translation unit and all exceptions are caught
# before returning to FontTool callers.
if(MSVC)
    target_compile_options(FontTool PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:/EHs-c->)
    set_property(SOURCE src/internal/podofo/auxiliary/Utf8Utils.cpp
        APPEND PROPERTY COMPILE_OPTIONS /EHsc)
    set_source_files_properties(${FONTTOOL_AFDKO_SOURCES}
        PROPERTIES COMPILE_OPTIONS "/w")
else()
    target_compile_options(FontTool PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    set_property(SOURCE src/internal/podofo/auxiliary/Utf8Utils.cpp
        APPEND PROPERTY COMPILE_OPTIONS -fexceptions)
    set_source_files_properties(${FONTTOOL_AFDKO_SOURCES}
        PROPERTIES COMPILE_OPTIONS "-w")
endif()

set_source_files_properties(third_party/utf8proc/utf8proc.c PROPERTIES
    COMPILE_DEFINITIONS "UTF8PROC_STATIC;UTF8PROC_EXPLICIT_INIT;_CRT_SECURE_NO_WARNINGS")

if(FONTTOOL_BUILD_CLI)
    add_executable(fonttool_cli src/cli/main.cpp)
    set_target_properties(fonttool_cli PROPERTIES OUTPUT_NAME fonttool)
    target_link_libraries(fonttool_cli PRIVATE FontTool::FontTool)
    target_compile_features(fonttool_cli PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(fonttool_cli PRIVATE /W4 /utf-8)
    else()
        target_compile_options(fonttool_cli PRIVATE -Wall -Wextra -Wpedantic -fno-exceptions)
    endif()
endif()

if(FONTTOOL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(FONTTOOL_BUILD_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

if(FONTTOOL_INSTALL)
    install(TARGETS FontTool
        EXPORT FontToolTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    if(TARGET fonttool_cli)
        install(TARGETS fonttool_cli
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()

    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    configure_package_config_file(
        cmake/FontToolConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/FontToolConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FontTool)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/FontToolConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(EXPORT FontToolTargets
        FILE FontToolTargets.cmake
        NAMESPACE FontTool::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FontTool)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/FontToolConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/FontToolConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FontTool)

    configure_file(cmake/fonttool.pc.in fonttool.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fonttool.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

    install(FILES README.md README_ZH.md CHANGELOG.md NOTICE LICENSE THIRD_PARTY_NOTICES.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
    install(DIRECTORY docs/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs)
    install(DIRECTORY LICENSES/
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/LICENSES)
endif()
